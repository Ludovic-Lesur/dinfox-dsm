/*
 * at.c
 *
 *  Created on: 18 apr. 2020
 *      Author: Ludo
 */

#include <at.h>
#include "adc.h"
#include "bpsm.h"
#include "ddrm.h"
#include "digital.h"
#include "dinfox.h"
#include "error.h"
#include "flash_reg.h"
#include "i2c.h"
#include "load.h"
#include "lpuart.h"
#include "lvrm.h"
#include "mapping.h"
#include "mode.h"
#include "nvic.h"
#include "nvm.h"
#include "parser.h"
#include "pwr.h"
#include "rcc_reg.h"
#include "rrm.h"
#include "sht3x.h"
#include "sm.h"
#include "string.h"
#include "types.h"
#include "version.h"

/*** AT local macros ***/

// Commands.
#define AT_COMMAND_BUFFER_SIZE			128
// Parameters separator.
#define AT_CHAR_SEPARATOR				','
// Replies.
#define AT_REPLY_BUFFER_SIZE			128
#define AT_STRING_VALUE_BUFFER_SIZE		16
#define AT_FRAME_END					STRING_CHAR_CR
#define AT_REPLY_TAB					"     "

/*** AT callbacks declaration ***/

static void _AT_print_ok(void);
static void _AT_print_command_list(void);
static void _AT_print_sw_version(void);
static void _AT_print_error_stack(void);
static void _AT_adc_callback(void);
static void _AT_read_callback(void);
static void _AT_write_callback(void);

/*** AT local structures ***/

typedef struct {
	PARSER_mode_t mode;
	char_t* syntax;
	char_t* parameters;
	char_t* description;
	void (*callback)(void);
} AT_command_t;

typedef struct {
	// Command.
	volatile char_t command[AT_COMMAND_BUFFER_SIZE];
	volatile uint32_t command_size;
	volatile uint8_t line_end_flag;
	PARSER_context_t parser;
	// Replies.
	char_t reply[AT_REPLY_BUFFER_SIZE];
	uint32_t reply_size;
} AT_context_t;

/*** AT local global variables ***/

static const AT_command_t AT_COMMAND_LIST[] = {
	{PARSER_MODE_COMMAND, "AT", STRING_NULL, "Ping command", _AT_print_ok},
	{PARSER_MODE_COMMAND, "AT?", STRING_NULL, "List all available commands", _AT_print_command_list},
	{PARSER_MODE_COMMAND, "AT$V?", STRING_NULL, "Get SW version", _AT_print_sw_version},
	{PARSER_MODE_COMMAND, "AT$ERROR?", STRING_NULL, "Read error stack", _AT_print_error_stack},
	{PARSER_MODE_COMMAND, "AT$RST", STRING_NULL, "Reset MCU", PWR_software_reset},
	{PARSER_MODE_COMMAND, "AT$ADC?", STRING_NULL, "Get ADC measurements", _AT_adc_callback},
	{PARSER_MODE_HEADER, "AT$R=", "address[hex]", "Read register", _AT_read_callback},
	{PARSER_MODE_HEADER, "AT$W=", "address[hex],value[hex]", "Write register",_AT_write_callback}
};

static AT_context_t at_ctx;

/*** AT local functions ***/

/* GENERIC MACRO TO ADD A CHARACTER TO THE REPLY BUFFER.
 * @param character:	Character to add.
 * @return:				None.
 */
#define _AT_reply_add_char(character) { \
	at_ctx.reply[at_ctx.reply_size] = character; \
	at_ctx.reply_size = (at_ctx.reply_size + 1) % AT_REPLY_BUFFER_SIZE; \
}

/* APPEND A STRING TO THE REPONSE BUFFER.
 * @param tx_string:	String to add.
 * @return:				None.
 */
static void _AT_reply_add_string(char_t* tx_string) {
	// Fill reply buffer with new bytes.
	while (*tx_string) {
		_AT_reply_add_char(*(tx_string++));
	}
}

/* APPEND A VALUE TO THE REPONSE BUFFER.
 * @param tx_value:		Value to add.
 * @param format:       Printing format.
 * @param print_prefix: Print base prefix is non zero.
 * @return:				None.
 */
static void _AT_reply_add_value(int32_t tx_value, STRING_format_t format, uint8_t print_prefix) {
	// Local variables.
	STRING_status_t string_status = STRING_SUCCESS;
	char_t str_value[AT_STRING_VALUE_BUFFER_SIZE];
	uint8_t idx = 0;
	// Reset string.
	for (idx=0 ; idx<AT_STRING_VALUE_BUFFER_SIZE ; idx++) str_value[idx] = STRING_CHAR_NULL;
	// Convert value to string.
	string_status = STRING_value_to_string(tx_value, format, print_prefix, str_value);
	STRING_error_check();
	// Add string.
	_AT_reply_add_string(str_value);
}

/* SEND AT REPONSE OVER AT INTERFACE.
 * @param:	None.
 * @return:	None.
 */
static void _AT_reply_send(void) {
	// Local variables.
#ifdef AM
	LBUS_status_t lbus_status = LBUS_SUCCESS;
#else
	LPUART_status_t lpuart1_status = LPUART_SUCCESS;
#endif
	// Add ending character.
	_AT_reply_add_char(AT_FRAME_END);
	_AT_reply_add_char(STRING_CHAR_NULL);
	// Send reply.
#ifdef AM
	lbus_status = LBUS_send((uint8_t*) at_ctx.reply, at_ctx.reply_size);
	LBUS_error_check();
#else
	lpuart1_status = LPUART1_send((uint8_t*) at_ctx.reply, at_ctx.reply_size);
	LPUART1_error_check();
#endif
	// Flush response buffer.
	at_ctx.reply_size = 0;
}

/* PRINT OK THROUGH AT INTERFACE.
 * @param:	None.
 * @return:	None.
 */
static void _AT_print_ok(void) {
	_AT_reply_add_string("OK");
	_AT_reply_send();
}

/* PRINT ERROR THROUGH AT INTERFACE.
 * @param error:	Error to print.
 * @return:			None.
 */
static void _AT_print_error(ERROR_t error) {
	// Add error to stack.
	ERROR_stack_add(error);
	// Print error.
	_AT_reply_add_string("ERROR_");
	if (error < 0x0100) {
		_AT_reply_add_value(0, STRING_FORMAT_HEXADECIMAL, 1);
		_AT_reply_add_value((int32_t) error, STRING_FORMAT_HEXADECIMAL, 0);
	}
	else {
		_AT_reply_add_value((int32_t) error, STRING_FORMAT_HEXADECIMAL, 1);
	}
	_AT_reply_send();
}

/* PRINT ALL SUPPORTED AT COMMANDS.
 * @param:	None.
 * @return:	None.
 */
static void _AT_print_command_list(void) {
	// Local variables.
	uint32_t idx = 0;
	// Commands loop.
	for (idx=0 ; idx<(sizeof(AT_COMMAND_LIST) / sizeof(AT_command_t)) ; idx++) {
		// Print syntax.
		_AT_reply_add_string(AT_COMMAND_LIST[idx].syntax);
		// Print parameters.
		_AT_reply_add_string(AT_COMMAND_LIST[idx].parameters);
		_AT_reply_send();
		// Print description.
		_AT_reply_add_string(AT_REPLY_TAB);
		_AT_reply_add_string(AT_COMMAND_LIST[idx].description);
		_AT_reply_send();
	}
	_AT_print_ok();
}

/* PRINT SW VERSION.
 * @param:	None.
 * @return:	None.
 */
static void _AT_print_sw_version(void) {
	_AT_reply_add_string("SW");
	_AT_reply_add_value((int32_t) GIT_MAJOR_VERSION, STRING_FORMAT_DECIMAL, 0);
	_AT_reply_add_string(".");
	_AT_reply_add_value((int32_t) GIT_MINOR_VERSION, STRING_FORMAT_DECIMAL, 0);
	_AT_reply_add_string(".");
	_AT_reply_add_value((int32_t) GIT_COMMIT_INDEX, STRING_FORMAT_DECIMAL, 0);
	if (GIT_DIRTY_FLAG != 0) {
		_AT_reply_add_string(".d");
	}
	_AT_reply_add_string(" (");
	_AT_reply_add_value((int32_t) GIT_COMMIT_ID, STRING_FORMAT_HEXADECIMAL, 1);
	_AT_reply_add_string(")");
	_AT_reply_send();
	_AT_print_ok();
}

/* PRINT ERROR STACK.
 * @param:	None.
 * @return:	None.
 */
static void _AT_print_error_stack(void) {
	// Local variables.
	ERROR_t error = SUCCESS;
	// Read stack.
	if (ERROR_stack_is_empty() != 0) {
		_AT_reply_add_string("Error stack empty");
	}
	else {
		// Unstack all errors.
		_AT_reply_add_string("[ ");
		do {
			error = ERROR_stack_read();
			if (error != SUCCESS) {
				_AT_reply_add_value((int32_t) error, STRING_FORMAT_HEXADECIMAL, 1);
				_AT_reply_add_string(" ");
			}
		}
		while (error != SUCCESS);
		_AT_reply_add_string("]");
	}
	_AT_reply_send();
	_AT_print_ok();
}

/* RS$ADC? EXECUTION CALLBACK.
 * @param:	None.
 * @return:	None.
 */
static void _AT_adc_callback(void) {
	// Local variables.
	ADC_status_t adc1_status = ADC_SUCCESS;
	uint32_t generic_u32 = 0;
	int8_t tmcu_degrees = 0;
	// Trigger internal ADC conversions.
	_AT_reply_add_string("ADC running...");
	_AT_reply_send();
	adc1_status = ADC1_perform_measurements();
	ADC1_error_check_print();
	// Read and print data.
	// Input voltage.
#ifdef LVRM
	_AT_reply_add_string("Vcom=");
	adc1_status = ADC1_get_data(ADC_DATA_INDEX_VCOM_MV, &generic_u32);
#endif
#ifdef BPSM
	_AT_reply_add_string("Vsrc=");
	adc1_status = ADC1_get_data(ADC_DATA_INDEX_VSRC_MV, &generic_u32);
#endif
#if (defined DDRM) || (defined RRM)
	_AT_reply_add_string("Vin=");
	adc1_status = ADC1_get_data(ADC_DATA_INDEX_VIN_MV, &generic_u32);
#endif
	ADC1_error_check_print();
	_AT_reply_add_value((int32_t) generic_u32, STRING_FORMAT_DECIMAL, 0);
	_AT_reply_add_string("mV");
	_AT_reply_send();
#ifdef BPSM
	// Storage element voltage.
	_AT_reply_add_string("Vstr=");
	adc1_status = ADC1_get_data(ADC_DATA_INDEX_VSTR_MV, &generic_u32);
	ADC1_error_check_print();
	_AT_reply_add_value((int32_t) generic_u32, STRING_FORMAT_DECIMAL, 0);
	_AT_reply_add_string("mV");
	_AT_reply_send();
#endif
	// Output voltage.
#if (defined LVRM) || (defined DDRM) || (defined RRM)
	_AT_reply_add_string("Vout=");
	adc1_status = ADC1_get_data(ADC_DATA_INDEX_VOUT_MV, &generic_u32);
#endif
#ifdef BPSM
	_AT_reply_add_string("Vbkp=");
	adc1_status = ADC1_get_data(ADC_DATA_INDEX_VBKP_MV, &generic_u32);
#endif
	ADC1_error_check_print();
	_AT_reply_add_value((int32_t) generic_u32, STRING_FORMAT_DECIMAL, 0);
	_AT_reply_add_string("mV");
	_AT_reply_send();
#if (defined LVRM) || (defined DDRM) || (defined RRM)
	// Output current.
	_AT_reply_add_string("Iout=");
	adc1_status = ADC1_get_data(ADC_DATA_INDEX_IOUT_UA, &generic_u32);
	ADC1_error_check_print();
	_AT_reply_add_value((int32_t) generic_u32, STRING_FORMAT_DECIMAL, 0);
	_AT_reply_add_string("uA");
	_AT_reply_send();
#endif
	// MCU voltage.
	_AT_reply_add_string("Vmcu=");
	adc1_status = ADC1_get_data(ADC_DATA_INDEX_VMCU_MV, &generic_u32);
	ADC1_error_check_print();
	_AT_reply_add_value((int32_t) generic_u32, STRING_FORMAT_DECIMAL, 0);
	_AT_reply_add_string("mV");
	_AT_reply_send();
	// MCU temperature.
	_AT_reply_add_string("Tmcu=");
	adc1_status = ADC1_get_tmcu(&tmcu_degrees);
	ADC1_error_check_print();
	_AT_reply_add_value((int32_t) tmcu_degrees, STRING_FORMAT_DECIMAL, 0);
	_AT_reply_add_string("dC");
	_AT_reply_send();
	_AT_print_ok();
errors:
	return;
}

/* RS$R EXECUTION CALLBACK.
 * @param:	None.
 * @return:	None.
 */
static void _AT_read_callback(void) {
	// Local variables.
	PARSER_status_t parser_status = PARSER_SUCCESS;
	NVM_status_t nvm_status = NVM_SUCCESS;
	ADC_status_t adc1_status = ADC_SUCCESS;
#ifdef SM
	DIGITAL_status_t digital_status = DIGITAL_SUCCESS;
	SHT3X_status_t sht3x_status = SHT3X_SUCCESS;
#endif
	int32_t register_address = 0;
	uint8_t generic_u8 = 0;
	int8_t generic_s8 = 0;
	uint32_t generic_u32 = 0;
	// Read address parameter.
	parser_status = PARSER_get_parameter(&at_ctx.parser, STRING_FORMAT_HEXADECIMAL, STRING_CHAR_NULL, &register_address);
	PARSER_error_check_print();
	// Get data.
	switch (register_address) {
	case DINFOX_REGISTER_LBUS_ADDRESS:
		nvm_status = NVM_read_byte(NVM_ADDRESS_LBUS_ADDRESS, &generic_u8);
		NVM_error_check_print();
		_AT_reply_add_value(generic_u8, STRING_FORMAT_HEXADECIMAL, 0);
		break;
	case DINFOX_REGISTER_BOARD_ID:
#ifdef LVRM
		_AT_reply_add_value(DINFOX_BOARD_ID_LVRM, STRING_FORMAT_HEXADECIMAL, 0);
#endif
#ifdef BPSM
		_AT_reply_add_value(DINFOX_BOARD_ID_BPSM, STRING_FORMAT_HEXADECIMAL, 0);
#endif
#ifdef DDRM
		_AT_reply_add_value(DINFOX_BOARD_ID_DDRM, STRING_FORMAT_HEXADECIMAL, 0);
#endif
#ifdef RRM
		_AT_reply_add_value(DINFOX_BOARD_ID_RRM, STRING_FORMAT_HEXADECIMAL, 0);
#endif
#ifdef SM
		_AT_reply_add_value(DINFOX_BOARD_ID_SM, STRING_FORMAT_HEXADECIMAL, 0);
#endif
		break;
	case DINFOX_REGISTER_HW_VERSION_MAJOR:
#ifdef HW1_0
		_AT_reply_add_value(1, STRING_FORMAT_DECIMAL, 0);
#endif
		break;
	case DINFOX_REGISTER_HW_VERSION_MINOR:
#ifdef HW1_0
		_AT_reply_add_value(0, STRING_FORMAT_DECIMAL, 0);
#endif
		break;
	case DINFOX_REGISTER_SW_VERSION_MAJOR:
		_AT_reply_add_value(GIT_MAJOR_VERSION, STRING_FORMAT_DECIMAL, 0);
		break;
	case DINFOX_REGISTER_SW_VERSION_MINOR:
		_AT_reply_add_value(GIT_MINOR_VERSION, STRING_FORMAT_DECIMAL, 0);
		break;
	case DINFOX_REGISTER_SW_VERSION_COMMIT_INDEX:
		_AT_reply_add_value(GIT_COMMIT_INDEX, STRING_FORMAT_DECIMAL, 0);
		break;
	case DINFOX_REGISTER_SW_VERSION_COMMIT_ID:
		_AT_reply_add_value(GIT_COMMIT_ID, STRING_FORMAT_HEXADECIMAL, 0);
		break;
	case DINFOX_REGISTER_SW_VERSION_DIRTY_FLAG:
		_AT_reply_add_value(GIT_DIRTY_FLAG, STRING_FORMAT_BOOLEAN, 0);
		break;
	case DINFOX_REGISTER_ERROR_STACK:
		_AT_reply_add_value(ERROR_stack_read(), STRING_FORMAT_HEXADECIMAL, 0);
		break;
	case DINFOX_REGISTER_RESET_REASON:
		_AT_reply_add_value((((RCC -> CSR) >> 24) & 0xFF), STRING_FORMAT_HEXADECIMAL, 0);
		break;
	case DINFOX_REGISTER_VMCU_MV:
#ifdef LVRM
	case LVRM_REGISTER_VCOM_MV:
	case LVRM_REGISTER_VOUT_MV:
	case LVRM_REGISTER_IOUT_UA:
#endif
#ifdef BPSM
	case BPSM_REGISTER_VSRC_MV:
	case BPSM_REGISTER_VSTR_MV:
	case BPSM_REGISTER_VBKP_MV:
#endif
#ifdef DDRM
	case DDRM_REGISTER_VIN_MV:
	case DDRM_REGISTER_VOUT_MV:
	case DDRM_REGISTER_IOUT_UA:
#endif
#ifdef RRM
	case RRM_REGISTER_VIN_MV:
	case RRM_REGISTER_VOUT_MV:
	case RRM_REGISTER_IOUT_UA:
#endif
#ifdef SM
	case SM_REGISTER_AIN0:
	case SM_REGISTER_AIN1:
	case SM_REGISTER_AIN2:
	case SM_REGISTER_AIN3:
#endif
		// Note: indexing only works if registers addresses are ordered in the same way as ADC data indexes.
		adc1_status = ADC1_get_data((register_address - DINFOX_REGISTER_VMCU_MV), &generic_u32);
		ADC1_error_check_print();
		_AT_reply_add_value((int32_t) generic_u32, STRING_FORMAT_DECIMAL, 0);
		break;
	case DINFOX_REGISTER_TMCU_DEGREES:
		// Read temperature.
		adc1_status = ADC1_get_tmcu(&generic_s8);
		ADC1_error_check_print();
		_AT_reply_add_value((int32_t) generic_s8, STRING_FORMAT_DECIMAL, 0);
		break;
#ifdef SM
	case SM_REGISTER_DIO0:
	case SM_REGISTER_DIO1:
	case SM_REGISTER_DIO2:
	case SM_REGISTER_DIO3:
		// Note: indexing only works if registers addresses are ordered in the same way as digital data indexes.
		digital_status = DIGITAL_read((register_address - SM_REGISTER_DIO0), &generic_u8);
		DIGITAL_error_check_print();
		_AT_reply_add_value((int32_t) generic_u8, STRING_FORMAT_BOOLEAN, 0);
		break;
	case SM_REGISTER_TAMB_DEGREES:
		sht3x_status = SHT3X_get_temperature(&generic_s8);
		SHT3X_error_check_print();
		_AT_reply_add_value((int32_t) generic_s8, STRING_FORMAT_DECIMAL, 0);
		break;
	case SM_REGISTER_HAMB_PERCENT:
		sht3x_status = SHT3X_get_humidity(&generic_u8);
		SHT3X_error_check_print();
		_AT_reply_add_value((int32_t) generic_u8, STRING_FORMAT_DECIMAL, 0);
		break;
#endif
#if (defined LVRM) || (defined BPSM) || (defined DDRM) || (defined RRM)
#ifdef LVRM
	case LVRM_REGISTER_RELAY_ENABLE:
#endif
#ifdef BPSM
	case BPSM_REGISTER_BACKUP_ENABLE:
#endif
#ifdef DDRM
	case DDRM_REGISTER_DC_DC_ENABLE:
#endif
#ifdef RRM
	case RRM_REGISTER_REGULATOR_ENABLE:
#endif
		_AT_reply_add_value(LOAD_get_output_state(), STRING_FORMAT_BOOLEAN, 0);
		break;
#endif
#ifdef BPSM
	case BPSM_REGISTER_CHARGE_ENABLE:
		_AT_reply_add_value(LOAD_get_charge_state(), STRING_FORMAT_BOOLEAN, 0);
		break;
	case BPSM_REGISTER_CHARGE_STATUS:
		_AT_reply_add_value(LOAD_get_charge_status(), STRING_FORMAT_BOOLEAN, 0);
		break;
#endif
	default:
		_AT_print_error(ERROR_REGISTER_ADDRESS);
		goto errors;
	}
	// Send response.
	_AT_reply_send();
errors:
	return;
}

/* RS$W EXECUTION CALLBACK.
 * @param:	None.
 * @return:	None.
 */
static void _AT_write_callback(void) {
	// Local variables.
	PARSER_status_t parser_status = PARSER_SUCCESS;
#ifdef DM
	NVM_status_t nvm_status = NVM_SUCCESS;
#endif
	int32_t register_address = 0;
#if (defined DM) || (defined LVRM) || (defined BPSM) || (defined DDRM) || (defined RRM)
	int32_t register_value = 0;
#endif
	// Read address parameter.
	parser_status = PARSER_get_parameter(&at_ctx.parser, STRING_FORMAT_HEXADECIMAL, AT_CHAR_SEPARATOR, &register_address);
	PARSER_error_check_print();
	// Check address.
#ifdef LVRM
	if (register_address >= LVRM_REGISTER_LAST) {
#endif
#ifdef BPSM
	if (register_address >= BPSM_REGISTER_LAST) {
#endif
#ifdef DDRM
	if (register_address >= DDRM_REGISTER_LAST) {
#endif
#ifdef RRM
	if (register_address >= RRM_REGISTER_LAST) {
#endif
#ifdef SM
	if (register_address >= SM_REGISTER_LAST) {
#endif
		_AT_print_error(ERROR_REGISTER_ADDRESS);
		goto errors;
	}
	// Write data.
	switch (register_address) {
#ifdef DM
	case DINFOX_REGISTER_AT_ADDRESS:
		// Read new address.
		parser_status = PARSER_get_parameter(&at_ctx.parser, STRING_FORMAT_HEXADECIMAL, STRING_CHAR_NULL, &register_value);
		PARSER_error_check_print();
		// Check value.
		if (register_value > AT_ADDRESS_LAST) {
			_AT_print_error(ERROR_AT_ADDRESS);
			goto errors;
		}
		nvm_status = NVM_write_byte(NVM_ADDRESS_AT_ADDRESS, (uint8_t) register_value);
		NVM_error_check_print();
		break;
#endif
#if (defined LVRM) || (defined BPSM) || (defined DDRM) || (defined RRM)
#ifdef LVRM
	case LVRM_REGISTER_RELAY_ENABLE:
#endif
#ifdef BPSM
	case BPSM_REGISTER_BACKUP_ENABLE:
#endif
#ifdef DDRM
	case DDRM_REGISTER_DC_DC_ENABLE:
#endif
#ifdef RRM
	case RRM_REGISTER_REGULATOR_ENABLE:
#endif
		// Read new output state.
		parser_status = PARSER_get_parameter(&at_ctx.parser, STRING_FORMAT_BOOLEAN, STRING_CHAR_NULL, &register_value);
		PARSER_error_check_print();
		// Set output state.
		LOAD_set_output_state(register_value);
		break;
#endif
#ifdef BPSM
	case BPSM_REGISTER_CHARGE_ENABLE:
		// Read new output state.
		parser_status = PARSER_get_parameter(&at_ctx.parser, STRING_FORMAT_BOOLEAN, STRING_CHAR_NULL, &register_value);
		PARSER_error_check_print();
		// Set charge state.
		LOAD_set_charge_state(register_value);
		break;
#endif
	default:
		_AT_print_error(ERROR_REGISTER_READ_ONLY);
		goto errors;
	}
	// Operation completed.
	_AT_print_ok();
errors:
	return;
}

/* RESET AT PARSER.
 * @param:	None.
 * @return:	None.
 */
static void _AT_reset_parser(void) {
	// Flush buffers.
	at_ctx.command_size = 0;
	at_ctx.reply_size = 0;
	// Reset flag.
	at_ctx.line_end_flag = 0;
	// Reset parser.
	at_ctx.parser.buffer = (char_t*) at_ctx.command;
	at_ctx.parser.buffer_size = 0;
	at_ctx.parser.separator_idx = 0;
	at_ctx.parser.start_idx = 0;
}

/* PARSE THE CURRENT AT COMMAND BUFFER.
 * @param:	None.
 * @return:	None.
 */
static void _AT_decode(void) {
	// Local variables.
	uint8_t idx = 0;
	uint8_t decode_success = 0;
	// Update parser length.
	at_ctx.parser.buffer_size = at_ctx.command_size;
	// Loop on available commands.
	for (idx=0 ; idx<(sizeof(AT_COMMAND_LIST) / sizeof(AT_command_t)) ; idx++) {
		// Check type.
		if (PARSER_compare(&at_ctx.parser, AT_COMMAND_LIST[idx].mode, AT_COMMAND_LIST[idx].syntax) == PARSER_SUCCESS) {
			// Execute callback and exit.
			AT_COMMAND_LIST[idx].callback();
			decode_success = 1;
			break;
		}
	}
	if (decode_success == 0) {
		_AT_print_error(ERROR_BASE_PARSER + PARSER_ERROR_UNKNOWN_COMMAND); // Unknown command.
		goto errors;
	}
errors:
	_AT_reset_parser();
	return;
}

/*** AT functions ***/

/* INIT AT MANAGER.
 * @param:	None.
 * @return:	None.
 */
void AT_init(void) {
	// Init context.
	_AT_reset_parser();
	// Enable LPUART.
	LPUART1_enable_rx();
}

/* MAIN TASK OF AT COMMAND MANAGER.
 * @param:	None.
 * @return:	None.
 */
void AT_task(void) {
	// Trigger decoding function if line end found.
	if (at_ctx.line_end_flag != 0) {
		// Decode and execute command.
		LPUART1_disable_rx();
		_AT_decode();
		LPUART1_enable_rx();
	}
}

/* FILL AT COMMAND BUFFER WITH A NEW BYTE (CALLED BY LPUART INTERRUPT).
 * @param rx_byte:	Incoming byte.
 * @return:			None.
 */
void AT_fill_rx_buffer(uint8_t rx_byte) {
	// Append byte if line end flag is not allready set.
	if (at_ctx.line_end_flag == 0) {
		// Check ending characters.
		if (rx_byte == AT_FRAME_END) {
			at_ctx.command[at_ctx.command_size] = STRING_CHAR_NULL;
			at_ctx.line_end_flag = 1;
		}
		else {
			// Store new byte.
			at_ctx.command[at_ctx.command_size] = rx_byte;
			// Manage index.
			at_ctx.command_size = (at_ctx.command_size + 1) % AT_COMMAND_BUFFER_SIZE;
		}
	}
}

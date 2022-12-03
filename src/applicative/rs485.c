/*
 * rs485.c
 *
 *  Created on: 18 apr. 2020
 *      Author: Ludo
 */

#include "rs485.h"

#include "adc.h"
#include "bpsm.h"
#include "ddrm.h"
#include "dinfox.h"
#include "error.h"
#include "flash_reg.h"
#include "led.h"
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
#include "rs485_common.h"
#include "string.h"
#include "types.h"
#include "version.h"

/*** RS485 local macros ***/

// Commands.
#define RS485_COMMAND_BUFFER_SIZE		32
// Parameters separator.
#define RS485_CHAR_SEPARATOR			','
// Replies.
#define RS485_REPLY_BUFFER_SIZE			32
#define RS485_STRING_VALUE_BUFFER_SIZE	16
#define RS485_REPLY_TAB					"     "

/*** RS485 callbacks declaration ***/

static void _RS485_print_ok(void);
static void _RS485_print_command_list(void);
static void _RS485_print_sw_version(void);
static void _RS485_print_error_stack(void);
static void _RS485_read_callback(void);
static void _RS485_write_callback(void);

/*** RS485 local structures ***/

typedef struct {
	PARSER_mode_t mode;
	char_t* syntax;
	char_t* parameters;
	char_t* description;
	void (*callback)(void);
} RS485_command_t;

typedef struct {
	// Commands.
	volatile char_t command[RS485_COMMAND_BUFFER_SIZE];
	volatile uint32_t command_size;
	volatile uint8_t line_end_flag;
	PARSER_context_t parser;
	// Replies.
	char_t reply[RS485_REPLY_BUFFER_SIZE];
	uint32_t reply_size;
} RS485_context_t;

/*** RS485 local global variables ***/

static const RS485_command_t RS485_COMMAND_LIST[] = {
	{PARSER_MODE_COMMAND, "RS", STRING_NULL, "Ping command", _RS485_print_ok},
	{PARSER_MODE_COMMAND, "RS?", STRING_NULL, "List all available commands", _RS485_print_command_list},
	{PARSER_MODE_COMMAND, "RS$V?", STRING_NULL, "Get SW version", _RS485_print_sw_version},
	{PARSER_MODE_COMMAND, "RS$ERROR?", STRING_NULL, "Read error stack", _RS485_print_error_stack},
	{PARSER_MODE_HEADER, "RS$R=", "address[hex]", "Read register", _RS485_read_callback},
	{PARSER_MODE_HEADER, "RS$W=", "address[hex],value[hex]", "Write register",_RS485_write_callback}
};

static RS485_context_t rs485_ctx;

/*** RS485 local functions ***/

/* GENERIC MACRO TO ADD A CHARACTER TO THE REPLY BUFFER.
 * @param character:	Character to add.
 * @return:				None.
 */
#define _RS485_reply_add_char(character) { \
	rs485_ctx.reply[rs485_ctx.reply_size] = character; \
	rs485_ctx.reply_size = (rs485_ctx.reply_size + 1) % RS485_REPLY_BUFFER_SIZE; \
}

/* APPEND A STRING TO THE REPONSE BUFFER.
 * @param tx_string:	String to add.
 * @return:				None.
 */
static void _RS485_reply_add_string(char_t* tx_string) {
	// Fill reply buffer with new bytes.
	while (*tx_string) {
		_RS485_reply_add_char(*(tx_string++));
	}
}

/* APPEND A VALUE TO THE REPONSE BUFFER.
 * @param tx_value:		Value to add.
 * @param format:       Printing format.
 * @param print_prefix: Print base prefix is non zero.
 * @return:				None.
 */
static void _RS485_reply_add_value(int32_t tx_value, STRING_format_t format, uint8_t print_prefix) {
	// Local variables.
	STRING_status_t string_status = STRING_SUCCESS;
	char_t str_value[RS485_STRING_VALUE_BUFFER_SIZE];
	uint8_t idx = 0;
	// Reset string.
	for (idx=0 ; idx<RS485_STRING_VALUE_BUFFER_SIZE ; idx++) str_value[idx] = STRING_CHAR_NULL;
	// Convert value to string.
	string_status = STRING_value_to_string(tx_value, format, print_prefix, str_value);
	STRING_error_check();
	// Add string.
	_RS485_reply_add_string(str_value);
}

/* SEND RS485 REPONSE OVER RS485 INTERFACE.
 * @param:	None.
 * @return:	None.
 */
static void _RS485_reply_send(void) {
	// Local variables.
	LPUART_status_t lpuart1_status = LPUART_SUCCESS;
	// Add ending character.
	_RS485_reply_add_char(RS485_FRAME_END);
	_RS485_reply_add_char(STRING_CHAR_NULL);
	// Send response over UART.
	lpuart1_status = LPUART1_send_string(rs485_ctx.reply);
	LPUART1_error_check();
	// Flush response buffer.
	rs485_ctx.reply_size = 0;
}

/* PRINT OK THROUGH RS485 INTERFACE.
 * @param:	None.
 * @return:	None.
 */
static void _RS485_print_ok(void) {
	_RS485_reply_add_string("OK");
	_RS485_reply_send();
}

/* PRINT ALL SUPPORTED AT COMMANDS.
 * @param:	None.
 * @return:	None.
 */
static void _RS485_print_command_list(void) {
	// Local variables.
	uint32_t idx = 0;
	// Commands loop.
	for (idx=0 ; idx<(sizeof(RS485_COMMAND_LIST) / sizeof(RS485_command_t)) ; idx++) {
		// Print syntax.
		_RS485_reply_add_string(RS485_COMMAND_LIST[idx].syntax);
		// Print parameters.
		_RS485_reply_add_string(RS485_COMMAND_LIST[idx].parameters);
		_RS485_reply_send();
		// Print description.
		_RS485_reply_add_string(RS485_REPLY_TAB);
		_RS485_reply_add_string(RS485_COMMAND_LIST[idx].description);
		_RS485_reply_send();
	}
	_RS485_print_ok();
}

/* PRINT SW VERSION.
 * @param:	None.
 * @return:	None.
 */
static void _RS485_print_sw_version(void) {
	_RS485_reply_add_string("SW");
	_RS485_reply_add_value((int32_t) GIT_MAJOR_VERSION, STRING_FORMAT_DECIMAL, 0);
	_RS485_reply_add_string(".");
	_RS485_reply_add_value((int32_t) GIT_MINOR_VERSION, STRING_FORMAT_DECIMAL, 0);
	_RS485_reply_add_string(".");
	_RS485_reply_add_value((int32_t) GIT_COMMIT_INDEX, STRING_FORMAT_DECIMAL, 0);
	if (GIT_DIRTY_FLAG != 0) {
		_RS485_reply_add_string(".d");
	}
	_RS485_reply_add_string(" (");
	_RS485_reply_add_value((int32_t) GIT_COMMIT_ID, STRING_FORMAT_HEXADECIMAL, 1);
	_RS485_reply_add_string(")");
	_RS485_reply_send();
	_RS485_print_ok();
}

/* PRINT ERROR STACK.
 * @param:	None.
 * @return:	None.
 */
static void _RS485_print_error_stack(void) {
	// Local variables.
	ERROR_t error = SUCCESS;
	// Read stack.
	if (ERROR_stack_is_empty() != 0) {
		_RS485_reply_add_string("Error stack empty");
	}
	else {
		// Unstack all errors.
		_RS485_reply_add_string("[ ");
		do {
			error = ERROR_stack_read();
			if (error != SUCCESS) {
				_RS485_reply_add_value((int32_t) error, STRING_FORMAT_HEXADECIMAL, 1);
				_RS485_reply_add_string(" ");
			}
		}
		while (error != SUCCESS);
		_RS485_reply_add_string("]");
	}
	_RS485_reply_send();
	_RS485_print_ok();
}

/* PRINT ERROR THROUGH RS485 INTERFACE.
 * @param error:	Error to print.
 * @return:			None.
 */
static void _RS485_print_error(ERROR_t error) {
	// Add error to stack.
	ERROR_stack_add(error);
	// Print error.
	_RS485_reply_add_string("ERROR");
	_RS485_reply_send();
}

/* RS$R EXECUTION CALLBACK.
 * @param:	None.
 * @return:	None.
 */
static void _RS485_read_callback(void) {
	// Local variables.
	PARSER_status_t parser_status = PARSER_SUCCESS;
	NVM_status_t nvm_status = NVM_SUCCESS;
	ADC_status_t adc1_status = ADC_SUCCESS;
	int32_t register_address = 0;
	uint8_t generic_u8 = 0;
	int8_t generic_s8 = 0;
	uint32_t generic_u32 = 0;
	// Read address parameter.
	parser_status = PARSER_get_parameter(&rs485_ctx.parser, STRING_FORMAT_HEXADECIMAL, STRING_CHAR_NULL, &register_address);
	PARSER_error_check_print();
	// Get data.
	switch (register_address) {
	case DINFOX_REGISTER_RS485_ADDRESS:
		nvm_status = NVM_read_byte(NVM_ADDRESS_RS485_ADDRESS, &generic_u8);
		NVM_error_check_print();
		_RS485_reply_add_value(generic_u8, STRING_FORMAT_HEXADECIMAL, 0);
		break;
	case DINFOX_REGISTER_BOARD_ID:
		_RS485_reply_add_value(DINFOX_BOARD_ID_LVRM, STRING_FORMAT_HEXADECIMAL, 0);
		break;
	case DINFOX_REGISTER_RESET:
		_RS485_reply_add_value((((RCC -> CSR) >> 24) & 0xFF), STRING_FORMAT_HEXADECIMAL, 0);
		break;
	case DINFOX_REGISTER_SW_VERSION_MAJOR:
		_RS485_reply_add_value(GIT_MAJOR_VERSION, STRING_FORMAT_DECIMAL, 0);
		break;
	case DINFOX_REGISTER_SW_VERSION_MINOR:
		_RS485_reply_add_value(GIT_MINOR_VERSION, STRING_FORMAT_DECIMAL, 0);
		break;
	case DINFOX_REGISTER_SW_VERSION_COMMIT_INDEX:
		_RS485_reply_add_value(GIT_COMMIT_INDEX, STRING_FORMAT_DECIMAL, 0);
		break;
	case DINFOX_REGISTER_SW_VERSION_COMMIT_ID:
		_RS485_reply_add_value(GIT_COMMIT_ID, STRING_FORMAT_HEXADECIMAL, 0);
		break;
	case DINFOX_REGISTER_SW_VERSION_DIRTY_FLAG:
		_RS485_reply_add_value(GIT_DIRTY_FLAG, STRING_FORMAT_BOOLEAN, 0);
		break;
	case DINFOX_REGISTER_ERROR_STACK:
		_RS485_reply_add_value(ERROR_stack_read(), STRING_FORMAT_HEXADECIMAL, 0);
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
		// Perform analog measurements.
		adc1_status = ADC1_perform_measurements();
		ADC1_error_check_print();
		// Note: indexing only works if registers addresses are ordered in the same way as ADC data indexes.
#ifdef LVRM
		adc1_status = ADC1_get_data((register_address - LVRM_REGISTER_VCOM_MV), &generic_u32);
#endif
#ifdef BPSM
		adc1_status = ADC1_get_data((register_address - BPSM_REGISTER_VSRC_MV), &generic_u32);
#endif
#ifdef DDRM
		adc1_status = ADC1_get_data((register_address - DDRM_REGISTER_VIN_MV), &generic_u32);
#endif
#ifdef RRM
		adc1_status = ADC1_get_data((register_address - RRM_REGISTER_VIN_MV), &generic_u32);
#endif
		ADC1_error_check_print();
		_RS485_reply_add_value((int32_t) generic_u32, STRING_FORMAT_DECIMAL, 0);
		break;
	case DINFOX_REGISTER_TMCU_DEGREES:
		// Perform analog measurements.
		adc1_status = ADC1_perform_measurements();
		ADC1_error_check_print();
		// Read temperature.
		adc1_status = ADC1_get_tmcu(&generic_s8);
		ADC1_error_check_print();
		_RS485_reply_add_value((int32_t) generic_s8, STRING_FORMAT_DECIMAL, 0);
		break;
#ifdef LVRM
	case LVRM_REGISTER_OUT_EN:
#endif
#ifdef BPSM
	case BPSM_REGISTER_OUT_EN:
#endif
#ifdef DDRM
	case DDRM_REGISTER_OUT_EN:
#endif
#ifdef RRM
	case RRM_REGISTER_OUT_EN:
#endif
		_RS485_reply_add_value(GPIO_read(&GPIO_OUT_EN), STRING_FORMAT_BOOLEAN, 0);
		break;
#ifdef BPSM
	case BPSM_REGISTER_CHRG_EN:
		_RS485_reply_add_value(GPIO_read(&GPIO_CHRG_EN), STRING_FORMAT_BOOLEAN, 0);
		break;
#endif
	default:
		_RS485_print_error(ERROR_REGISTER_ADDRESS);
		goto errors;
	}
	// Send response.
	_RS485_reply_send();
errors:
	return;
}

/* RS$W EXECUTION CALLBACK.
 * @param:	None.
 * @return:	None.
 */
static void _RS485_write_callback(void) {
	// Local variables.
	PARSER_status_t parser_status = PARSER_SUCCESS;
#ifdef DM
	NVM_status_t nvm_status = NVM_SUCCESS;
#endif
	int32_t register_address = 0;
	int32_t register_value = 0;
	// Read address parameter.
	parser_status = PARSER_get_parameter(&rs485_ctx.parser, STRING_FORMAT_HEXADECIMAL, RS485_CHAR_SEPARATOR, &register_address);
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
		_RS485_print_error(ERROR_REGISTER_ADDRESS);
		goto errors;
	}
	// Get data.
	switch (register_address) {
#ifdef DM
	case DINFOX_REGISTER_RS485_ADDRESS:
		// Read new address.
		parser_status = PARSER_get_parameter(&rs485_ctx.parser, STRING_FORMAT_HEXADECIMAL, STRING_CHAR_NULL, &register_value);
		PARSER_error_check_print();
		// Check value.
		if (register_value > RS485_ADDRESS_LAST) {
			_RS485_print_error(ERROR_RS485_ADDRESS);
			goto errors;
		}
		nvm_status = NVM_write_byte(NVM_ADDRESS_RS485_ADDRESS, (uint8_t) register_value);
		NVM_error_check_print();
		break;
#endif
	case DINFOX_REGISTER_RESET:
		// Read parameter.
		parser_status = PARSER_get_parameter(&rs485_ctx.parser, STRING_FORMAT_BOOLEAN, STRING_CHAR_NULL, &register_value);
		PARSER_error_check_print();
		// Reset if required.
		if (register_value != 0) {
			PWR_software_reset();
		}
		break;
#ifdef LVRM
	case LVRM_REGISTER_OUT_EN:
#endif
#ifdef BPSM
	case BPSM_REGISTER_OUT_EN:
#endif
#ifdef DDRM
	case DDRM_REGISTER_OUT_EN:
#endif
#ifdef RRM
	case RRM_REGISTER_OUT_EN:
#endif
		// Read new output state.
		parser_status = PARSER_get_parameter(&rs485_ctx.parser, STRING_FORMAT_BOOLEAN, STRING_CHAR_NULL, &register_value);
		PARSER_error_check_print();
		// Set relay state.
		GPIO_write(&GPIO_OUT_EN, register_value);
		break;
#ifdef BPSM
	case BPSM_REGISTER_CHRG_EN:
		// Read new output state.
		parser_status = PARSER_get_parameter(&rs485_ctx.parser, STRING_FORMAT_BOOLEAN, STRING_CHAR_NULL, &register_value);
		PARSER_error_check_print();
		// Set relay state.
		GPIO_write(&GPIO_CHRG_EN, register_value);
		break;
#endif
	default:
		_RS485_print_error(ERROR_REGISTER_READ_ONLY);
		goto errors;
	}
	// Operation completed.
	_RS485_print_ok();
errors:
	return;
}

/* RESET RS485 PARSER.
 * @param:	None.
 * @return:	None.
 */
static void _RS485_reset_parser(void) {
	// Flush buffers.
	rs485_ctx.command_size = 0;
	rs485_ctx.reply_size = 0;
	// Reset flag.
	rs485_ctx.line_end_flag = 0;
	// Reset parser.
	rs485_ctx.parser.buffer = (char_t*) rs485_ctx.command;
	rs485_ctx.parser.buffer_size = 0;
	rs485_ctx.parser.separator_idx = 0;
	rs485_ctx.parser.start_idx = 0;
}

/* PARSE THE CURRENT RS485 COMMAND BUFFER.
 * @param:	None.
 * @return:	None.
 */
static void _RS485_decode(void) {
	// Local variables.
	uint32_t idx = 0;
	uint8_t decode_success = 0;
	// Update parser length.
	rs485_ctx.parser.buffer_size = rs485_ctx.command_size;
	// Loop on available commands.
	for (idx=0 ; idx<(sizeof(RS485_COMMAND_LIST) / sizeof(RS485_command_t)) ; idx++) {
		// Check type.
		if (PARSER_compare(&rs485_ctx.parser, RS485_COMMAND_LIST[idx].mode, RS485_COMMAND_LIST[idx].syntax) == PARSER_SUCCESS) {
			// Execute callback and exit.
			RS485_COMMAND_LIST[idx].callback();
			decode_success = 1;
			break;
		}
	}
	if (decode_success == 0) {
		_RS485_print_error(ERROR_BASE_PARSER + PARSER_ERROR_UNKNOWN_COMMAND); // Unknown command.
		goto errors;
	}
errors:
	_RS485_reset_parser();
	return;
}

/*** RS485 functions ***/

/* INIT RS485 MANAGER.
 * @param:	None.
 * @return:	None.
 */
void RS485_init(void) {
	// Init context.
	_RS485_reset_parser();
	// Enable LPUART.
	LPUART1_enable_rx();
}

/* MAIN TASK OF RS485 COMMAND MANAGER.
 * @param:	None.
 * @return:	None.
 */
void RS485_task(void) {
	// Trigger decoding function if line end found.
	if (rs485_ctx.line_end_flag != 0) {
		LPUART1_disable_rx();
		_RS485_decode();
		LPUART1_enable_rx();
	}
}

/* FILL RS485 COMMAND BUFFER WITH A NEW BYTE (CALLED BY LPUART INTERRUPT).
 * @param rx_byte:	Incoming byte.
 * @return:			None.
 */
void RS485_fill_rx_buffer(uint8_t rx_byte) {
	// Append byte if line end flag is not allready set.
	if (rs485_ctx.line_end_flag == 0) {
		// Check ending characters.
		if (rx_byte == RS485_FRAME_END) {
			rs485_ctx.command[rs485_ctx.command_size] = STRING_CHAR_NULL;
			rs485_ctx.line_end_flag = 1;
		}
		else {
			// Store new byte.
			rs485_ctx.command[rs485_ctx.command_size] = rx_byte;
			// Manage index.
			rs485_ctx.command_size = (rs485_ctx.command_size + 1) % RS485_COMMAND_BUFFER_SIZE;
		}
	}
}

/*
 * rs485.c
 *
 *  Created on: 18 apr. 2020
 *      Author: Ludo
 */

#include "rs485.h"

#include "adc.h"
#include "dinfox.h"
#include "error.h"
#include "flash_reg.h"
#include "led.h"
#include "lpuart.h"
#include "lvrm.h"
#include "mapping.h"
#include "nvic.h"
#include "nvm.h"
#include "parser.h"
#include "relay.h"
#include "string.h"
#include "types.h"

/*** RS485 local macros ***/

// Common macros.
#define RS485_COMMAND_LENGTH_MIN			2
#define RS485_COMMAND_BUFFER_LENGTH			128
#define RS485_RESPONSE_BUFFER_LENGTH		128
#define RS485_STRING_VALUE_BUFFER_LENGTH	16
#define RS485_ADDRESS_LAST					RS485_ADDRESS_MASK
// Parameters separator.
#define RS485_CHAR_SEPARATOR				','
// Responses.
#define RS485_RESPONSE_END					"\r"
#define RS485_RESPONSE_TAB					"     "

/*** RS485 callbacks declaration ***/

static void _RS485_print_ok(void);
#ifdef ATM
static void _RS485_print_command_list(void);
#endif
static void _RS485_read_callback(void);
static void _RS485_write_callback(void);

/*** RS485 local structures ***/

typedef struct {
	PARSER_mode_t mode;
	char_t* syntax;
#ifdef ATM
	char_t* parameters;
	char_t* description;
#endif
	void (*callback)(void);
} RS485_command_t;

typedef struct {
	// RS485 command buffer.
	volatile char_t command_buf[RS485_COMMAND_BUFFER_LENGTH];
	volatile uint32_t command_buf_idx;
	volatile uint8_t line_end_flag;
	PARSER_context_t parser;
	char_t response_buf[RS485_RESPONSE_BUFFER_LENGTH];
	uint32_t response_buf_idx;
} RS485_context_t;

/*** RS485 local global variables ***/

#ifdef ATM
static const RS485_command_t RS485_COMMAND_LIST[] = {
	{PARSER_MODE_COMMAND,"RS", STRING_NULL, "Ping command", _RS485_print_ok},
	{PARSER_MODE_COMMAND, "RS?", STRING_NULL, "List all available RS485 commands", _RS485_print_command_list},
	{PARSER_MODE_HEADER, "RS$R=", "address[dec]", "Read board register", _RS485_read_callback},
	{PARSER_MODE_HEADER, "RS$W=", "address[dec]", "Write board register", _RS485_write_callback},
};
#endif
#ifdef RSM
static const RS485_command_t RS485_COMMAND_LIST[] = {
	{PARSER_MODE_COMMAND,"RS", _RS485_print_ok},
	{PARSER_MODE_HEADER, "RS$R=", _RS485_read_callback},
	{PARSER_MODE_HEADER, "RS$W=", _RS485_write_callback},
};
#endif

static RS485_context_t at_ctx;

/*** RS485 local functions ***/

/* APPEND A STRING TO THE REPONSE BUFFER.
 * @param tx_string:	String to add.
 * @return:				None.
 */
static void _RS485_response_add_string(char_t* tx_string) {
	// Fill TX buffer with new bytes.
	while (*tx_string) {
		at_ctx.response_buf[at_ctx.response_buf_idx++] = *(tx_string++);
		// Manage rollover.
		if (at_ctx.response_buf_idx >= RS485_RESPONSE_BUFFER_LENGTH) {
			at_ctx.response_buf_idx = 0;
		}
	}
}

/* APPEND A VALUE TO THE REPONSE BUFFER.
 * @param tx_value:		Value to add.
 * @param format:       Printing format.
 * @param print_prefix: Print base prefix is non zero.
 * @return:				None.
 */
static void _RS485_response_add_value(int32_t tx_value, STRING_format_t format, uint8_t print_prefix) {
	// Local variables.
	char_t str_value[RS485_STRING_VALUE_BUFFER_LENGTH];
	uint8_t idx = 0;
	// Reset string.
	for (idx=0 ; idx<RS485_STRING_VALUE_BUFFER_LENGTH ; idx++) str_value[idx] = STRING_CHAR_NULL;
	// Convert value to string.
	STRING_value_to_string(tx_value, format, print_prefix, str_value);
	// Add string.
	_RS485_response_add_string(str_value);
}

/* SEND RS485 REPONSE OVER RS485 INTERFACE.
 * @param:	None.
 * @return:	None.
 */
static void _RS485_response_send(void) {
	// Local variables.
	uint32_t idx = 0;
	// Send response over UART.
	LPUART1_send_string(at_ctx.response_buf);
	// Flush response buffer.
	for (idx=0 ; idx<RS485_RESPONSE_BUFFER_LENGTH ; idx++) at_ctx.response_buf[idx] = STRING_CHAR_NULL;
	at_ctx.response_buf_idx = 0;
}

/* PRINT OK THROUGH RS485 INTERFACE.
 * @param:	None.
 * @return:	None.
 */
static void _RS485_print_ok(void) {
	_RS485_response_add_string("OK");
	_RS485_response_add_string(RS485_RESPONSE_END);
	_RS485_response_send();
}

/* PRINT A STATUS THROUGH RS485 INTERFACE.
 * @param status:	Status to print.
 * @return:			None.
 */
static void _RS485_print_status(ERROR_t status) {
	_RS485_response_add_string("ERROR_");
	if (status < 0x0100) {
		_RS485_response_add_value(0, STRING_FORMAT_HEXADECIMAL, 1);
		_RS485_response_add_value(status, STRING_FORMAT_HEXADECIMAL, 0);
	}
	else {
		_RS485_response_add_value(status, STRING_FORMAT_HEXADECIMAL, 1);
	}
	_RS485_response_add_string(RS485_RESPONSE_END);
	_RS485_response_send();
}

#ifdef ATM
/* PRINT ALL SUPPORTED RS485 COMMANDS.
 * @param:	None.
 * @return:	None.
 */
static void _RS485_print_command_list(void) {
	// Local variables.
	uint32_t idx = 0;
	// Commands loop.
	for (idx=0 ; idx<(sizeof(RS485_COMMAND_LIST) / sizeof(RS485_command_t)) ; idx++) {
		// Print syntax.
		_RS485_response_add_string(RS485_COMMAND_LIST[idx].syntax);
		// Print parameters.
		_RS485_response_add_string(RS485_COMMAND_LIST[idx].parameters);
		_RS485_response_add_string(RS485_RESPONSE_END);
		// Print description.
		_RS485_response_add_string(RS485_RESPONSE_TAB);
		_RS485_response_add_string(RS485_COMMAND_LIST[idx].description);
		_RS485_response_add_string(RS485_RESPONSE_END);
		_RS485_response_send();
	}
}
#endif

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
	uint32_t generic_u32 = 0;
	// Read address parameter.
	parser_status = PARSER_get_parameter(&at_ctx.parser, STRING_FORMAT_HEXADECIMAL, STRING_CHAR_NULL, &register_address);
	PARSER_error_check_print();
	// Get data.
	switch (register_address) {
	case LVRM_REGISTER_RS485_ADDRESS:
		nvm_status = NVM_read_byte(NVM_ADDRESS_RS485_ADDRESS, &generic_u8);
		NVM_error_check_print();
		_RS485_response_add_value(generic_u8, STRING_FORMAT_HEXADECIMAL, 0);
		break;
	case LVRM_REGISTER_BOARD_ID:
		_RS485_response_add_value(DINFOX_BOARD_ID_LVRM, STRING_FORMAT_HEXADECIMAL, 0);
		break;
	case LVRM_REGISTER_VCOM_MV:
	case LVRM_REGISTER_VOUT_MV:
	case LVRM_REGISTER_IOUT_UA:
	case LVRM_REGISTER_VMCU_MV:
		// Perform analog measurements.
		adc1_status = ADC1_perform_measurements();
		ADC1_error_check_print();
		// Note: indexing only works if registers addresses are ordered in the same way as ADC data indexes.
		adc1_status = ADC1_get_data((register_address - LVRM_REGISTER_VCOM_MV), &generic_u32);
		ADC1_error_check_print();
		_RS485_response_add_value(generic_u32, STRING_FORMAT_DECIMAL, 0);
		break;
	case LVRM_REGISTER_OUT_EN:
		_RS485_response_add_value(GPIO_read(&GPIO_OUT_EN), STRING_FORMAT_BOOLEAN, 0);
		break;
	default:
		_RS485_print_status(ERROR_REGISTER_ADDRESS);
		goto errors;
	}
	// Send response.
	_RS485_response_add_string(RS485_RESPONSE_END);
	_RS485_response_send();
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
	NVM_status_t nvm_status = NVM_SUCCESS;
	int32_t register_address = 0;
	int32_t register_value = 0;
	// Read address parameter.
	parser_status = PARSER_get_parameter(&at_ctx.parser, STRING_FORMAT_HEXADECIMAL, RS485_CHAR_SEPARATOR, &register_address);
	PARSER_error_check_print();
	// Get data.
	switch (register_address) {
	case LVRM_REGISTER_RS485_ADDRESS:
		// Read new address.
		parser_status = PARSER_get_parameter(&at_ctx.parser, STRING_FORMAT_HEXADECIMAL, STRING_CHAR_NULL, &register_value);
		PARSER_error_check_print();
		// Check value.
		if (register_value > RS485_ADDRESS_LAST) {
			_RS485_print_status(ERROR_RS485_ADDRESS);
			goto errors;
		}
		nvm_status = NVM_write_byte(NVM_ADDRESS_RS485_ADDRESS, (uint8_t) register_value);
		NVM_error_check_print();
		break;
	case LVRM_REGISTER_OUT_EN:
		// Read new output state.
		parser_status = PARSER_get_parameter(&at_ctx.parser, STRING_FORMAT_BOOLEAN, STRING_CHAR_NULL, &register_value);
		PARSER_error_check_print();
		// Set relay state.
		RELAY_set_state((uint8_t) register_value);
		break;
	case LVRM_REGISTER_BOARD_ID:
	case LVRM_REGISTER_VCOM_MV:
	case LVRM_REGISTER_VOUT_MV:
	case LVRM_REGISTER_IOUT_UA:
	case LVRM_REGISTER_VMCU_MV:
		_RS485_print_status(ERROR_REGISTER_READ_ONLY);
		goto errors;
	default:
		_RS485_print_status(ERROR_REGISTER_ADDRESS);
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
static void RS485_reset_parser(void) {
	// Reset parsing variables.
	at_ctx.command_buf_idx = 0;
	at_ctx.line_end_flag = 0;
	at_ctx.parser.rx_buf = (char_t*) at_ctx.command_buf;
	at_ctx.parser.rx_buf_length = 0;
	at_ctx.parser.separator_idx = 0;
	at_ctx.parser.start_idx = 0;
}

/* PARSE THE CURRENT RS485 COMMAND BUFFER.
 * @param:	None.
 * @return:	None.
 */
static void RS485_decode(void) {
	// Local variables.
	uint32_t idx = 0;
	uint8_t decode_success = 0;
	// Empty or too short command.
	if (at_ctx.command_buf_idx < RS485_COMMAND_LENGTH_MIN) {
		_RS485_print_status(ERROR_BASE_PARSER + PARSER_ERROR_UNKNOWN_COMMAND);
		goto errors;
	}
	// Update parser length.
	at_ctx.parser.rx_buf_length = at_ctx.command_buf_idx;
	// Loop on available commands.
	for (idx=0 ; idx<(sizeof(RS485_COMMAND_LIST) / sizeof(RS485_command_t)) ; idx++) {
		// Check type.
		if (PARSER_compare(&at_ctx.parser, RS485_COMMAND_LIST[idx].mode, RS485_COMMAND_LIST[idx].syntax) == PARSER_SUCCESS) {
			// Execute callback and exit.
			RS485_COMMAND_LIST[idx].callback();
			decode_success = 1;
			break;
		}
	}
	if (decode_success == 0) {
		_RS485_print_status(ERROR_BASE_PARSER + PARSER_ERROR_UNKNOWN_COMMAND); // Unknown command.
		goto errors;
	}
errors:
	RS485_reset_parser();
	return;
}

/*** RS485 functions ***/

/* INIT RS485 MANAGER.
 * @param:	None.
 * @return:	None.
 */
void RS485_init(void) {
	// Init context.
	uint32_t idx = 0;
	for (idx=0 ; idx<RS485_COMMAND_BUFFER_LENGTH ; idx++) at_ctx.command_buf[idx] = '\0';
	for (idx=0 ; idx<RS485_RESPONSE_BUFFER_LENGTH ; idx++) at_ctx.response_buf[idx] = '\0';
	at_ctx.response_buf_idx = 0;
	// Reset parser.
	RS485_reset_parser();
	// Enable LPUART.
	LPUART1_enable_rx();
}

/* MAIN TASK OF RS485 COMMAND MANAGER.
 * @param:	None.
 * @return:	None.
 */
void RS485_task(void) {
	// Trigger decoding function if line end found.
	if (at_ctx.line_end_flag != 0) {
#ifdef ATM
		GPIO_write(&GPIO_LED_BLUE, 0);
#endif
		LPUART1_disable_rx();
		RS485_decode();
		LPUART1_enable_rx();
#ifdef ATM
		GPIO_write(&GPIO_LED_BLUE, 1);
#endif
	}
}

/* FILL RS485 COMMAND BUFFER WITH A NEW BYTE (CALLED BY USART INTERRUPT).
 * @param rx_byte:	Incoming byte.
 * @return:			None.
 */
void RS485_fill_rx_buffer(uint8_t rx_byte) {
	// Append byte if line end flag is not allready set.
	if (at_ctx.line_end_flag == 0) {
		// Check ending characters.
		if ((rx_byte == STRING_CHAR_CR) || (rx_byte == STRING_CHAR_LF)) {
			at_ctx.command_buf[at_ctx.command_buf_idx] = STRING_CHAR_NULL;
			at_ctx.line_end_flag = 1;
		}
		else {
			// Store new byte.
			at_ctx.command_buf[at_ctx.command_buf_idx] = rx_byte;
			// Manage index.
			at_ctx.command_buf_idx++;
			if (at_ctx.command_buf_idx >= RS485_COMMAND_BUFFER_LENGTH) {
				at_ctx.command_buf_idx = 0;
			}
		}
	}
}

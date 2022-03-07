/*
 * at.c
 *
 *  Created on: 18 apr. 2020
 *      Author: Ludo
 */

#include "at.h"

#include "adc.h"
#include "flash_reg.h"
#include "led.h"
#include "lpuart.h"
#include "lptim.h"
#include "mapping.h"
#include "math.h"
#include "nvic.h"
#include "parser.h"
#include "relay.h"
#include "string.h"
#include "tim.h"
#include "usart.h"

/*** AT local macros ***/

// Common macros.
#define AT_COMMAND_LENGTH_MIN			2
#define AT_COMMAND_BUFFER_LENGTH		128
#define AT_RESPONSE_BUFFER_LENGTH		128
#define AT_STRING_VALUE_BUFFER_LENGTH	16
// Input commands without parameter.
#define AT_COMMAND_TEST					"AT"
#define AT_COMMAND_INFO					"ATI?"
// Input commands with parameters (headers).
#define AT_HEADER_ADC					"AT$ADC="
#define AT_HEADER_OUT					"AT$OUT="
// Parameters separator.
#define AT_CHAR_SEPARATOR				','
// Responses.
#define AT_RESPONSE_OK					"OK"
#define AT_RESPONSE_END					"\n"
#define AT_RESPONSE_ERROR_PSR			"PSR_ERROR_"
#define AT_RESPONSE_ERROR_APP			"APP_ERROR_"

/*** AT local structures ***/

typedef enum {
	AT_ERROR_SOURCE_PARSER,
	AT_ERROR_SOURCE_PERIPHERAL
} AT_error_source_t;

typedef struct {
	// AT command buffer.
	volatile unsigned char at_command_buf[AT_COMMAND_BUFFER_LENGTH];
	volatile unsigned int at_command_buf_idx;
	volatile unsigned char at_line_end_flag;
	PARSER_Context at_parser;
	char at_response_buf[AT_RESPONSE_BUFFER_LENGTH];
	unsigned int at_response_buf_idx;
} AT_context_t;

/*** AT local global variables ***/

static AT_context_t at_ctx;

/*** AT local functions ***/

/* APPEND A STRING TO THE REPONSE BUFFER.
 * @param tx_string:	String to add.
 * @return:				None.
 */
static void AT_response_add_string(char* tx_string) {
	// Fill TX buffer with new bytes.
	while (*tx_string) {
		at_ctx.at_response_buf[at_ctx.at_response_buf_idx++] = *(tx_string++);
		// Manage rollover.
		if (at_ctx.at_response_buf_idx >= AT_RESPONSE_BUFFER_LENGTH) {
			at_ctx.at_response_buf_idx = 0;
		}
	}
}

/* APPEND A VALUE TO THE REPONSE BUFFER.
 * @param tx_value:		Value to add.
 * @param format:       Printing format.
 * @param print_prefix: Print base prefix is non zero.
 * @return:				None.
 */
static void AT_response_add_value(int tx_value, STRING_format_t format, unsigned char print_prefix) {
	// Local variables.
	char str_value[AT_STRING_VALUE_BUFFER_LENGTH];
	unsigned char idx = 0;
	// Reset string.
	for (idx=0 ; idx<AT_STRING_VALUE_BUFFER_LENGTH ; idx++) str_value[idx] = '\0';
	// Convert value to string.
	STRING_convert_value(tx_value, format, print_prefix, str_value);
	// Add string.
	AT_response_add_string(str_value);
}

/* PRINT OK THROUGH AT INTERFACE.
 * @param:	None.
 * @return:	None.
 */
static void AT_print_ok(void) {
	AT_response_add_string(AT_RESPONSE_OK);
	AT_response_add_string(AT_RESPONSE_END);
}

/* PRINT AN ERROR THROUGH AT INTERFACE.
 * @param error_code:	Error code to display.
 * @return:				None.
 */
static void AT_print_error(AT_error_source_t error_source, unsigned int error_code) {
	switch (error_source) {
	case AT_ERROR_SOURCE_PARSER:
		AT_response_add_string(AT_RESPONSE_ERROR_PSR);
		break;
	case AT_ERROR_SOURCE_PERIPHERAL:
		AT_response_add_string(AT_RESPONSE_ERROR_APP);
		break;
	default:
		break;
	}
	AT_response_add_value(error_code, STRING_FORMAT_HEXADECIMAL, 1);
	AT_response_add_string(AT_RESPONSE_END);
}

/* PARSE THE CURRENT AT COMMAND BUFFER.
 * @param:	None.
 * @return:	None.
 */
static void AT_decode(void) {
	// Local variables.
	PARSER_Status parser_status = PARSER_ERROR_UNKNOWN_COMMAND;
	int generic_int_1 = 0;
	int generic_int_2 = 0;
	int data_idx = 0;
	int enable = 0;
	unsigned int adc_data = 0;
	unsigned char extracted_length = 0;
	// Empty or too short command.
	if (at_ctx.at_command_buf_idx < AT_COMMAND_LENGTH_MIN) {
		AT_print_error(AT_ERROR_SOURCE_PARSER, PARSER_ERROR_UNKNOWN_COMMAND);
	}
	else {
		// Update parser length.
		at_ctx.at_parser.rx_buf_length = (at_ctx.at_command_buf_idx - 1); // To ignore line end.
		// Test command AT<CR>.
		if (PARSER_compare(&at_ctx.at_parser, PARSER_MODE_COMMAND, AT_COMMAND_TEST) == PARSER_SUCCESS) {
			AT_print_ok();
		}
		// ADC command AT$ADC?<CR>.
		else if (PARSER_compare(&at_ctx.at_parser, PARSER_MODE_HEADER, AT_HEADER_ADC) == PARSER_SUCCESS) {
			// Read index parameter.
			parser_status = PARSER_get_parameter(&at_ctx.at_parser, PARSER_PARAMETER_TYPE_DECIMAL, AT_CHAR_SEPARATOR, 1, &data_idx);
			if (parser_status == PARSER_SUCCESS) {
				// Perform measurements.
				ADC1_enable();
				ADC1_perform_measurements();
				ADC1_disable();
				// Get result.
				ADC1_get_data(data_idx, &adc_data);
				// Print response.
				AT_response_add_value((int) adc_data, STRING_FORMAT_DECIMAL, 0);
				AT_response_add_string(AT_RESPONSE_END);
			}
			else {
				AT_print_error(AT_ERROR_SOURCE_PARSER, parser_status);
			}
		}
		else if (PARSER_compare(&at_ctx.at_parser, PARSER_MODE_HEADER, AT_HEADER_OUT) == PARSER_SUCCESS) {
			// Read index parameter.
			parser_status = PARSER_get_parameter(&at_ctx.at_parser, PARSER_PARAMETER_TYPE_BOOLEAN, AT_CHAR_SEPARATOR, 1, &enable);
			if (parser_status == PARSER_SUCCESS) {
				// Set relay state.
				RELAY_set_state(enable);
				AT_print_ok();
			}
			else {
				AT_print_error(AT_ERROR_SOURCE_PARSER, parser_status);
			}
		}
		// Unknown command.
		else {
			AT_print_error(AT_ERROR_SOURCE_PARSER, PARSER_ERROR_UNKNOWN_COMMAND);
		}

	}
	// Send response.
	LPUART1_send_string(at_ctx.at_response_buf);
	// Reset AT parser.
	AT_init();
}

/*** AT functions ***/

/* INIT AT MANAGER.
 * @param:	None.
 * @return:	None.
 */
void AT_init(void) {
	// Init context.
	unsigned int idx = 0;
	for (idx=0 ; idx<AT_COMMAND_BUFFER_LENGTH ; idx++) at_ctx.at_command_buf[idx] = '\0';
	at_ctx.at_command_buf_idx = 0;
	at_ctx.at_line_end_flag = 0;
	for (idx=0 ; idx<AT_RESPONSE_BUFFER_LENGTH ; idx++) at_ctx.at_response_buf[idx] = '\0';
	at_ctx.at_response_buf_idx = 0;
	// Parsing variables.
	at_ctx.at_parser.rx_buf = (unsigned char*) at_ctx.at_command_buf;
	at_ctx.at_parser.rx_buf_length = 0;
	at_ctx.at_parser.separator_idx = 0;
	at_ctx.at_parser.start_idx = 0;
	// Enable LPUART.
	LPUART1_enable_rx();
}

/* MAIN TASK OF AT COMMAND MANAGER.
 * @param:	None.
 * @return:	None.
 */
void AT_task(void) {
	// Trigger decoding function if line end found.
	if (at_ctx.at_line_end_flag != 0) {
		LPUART1_disable_rx();
		LED_single_blink(100, TIM2_CHANNEL_MASK_BLUE);
		AT_decode();
		LPUART1_enable_rx();
	}
}

/* FILL AT COMMAND BUFFER WITH A NEW BYTE (CALLED BY USART INTERRUPT).
 * @param rx_byte:	Incoming byte.
 * @return:			None.
 */
void AT_fill_rx_buffer(unsigned char rx_byte) {
	// Append byte if line end flag is not allready set.
	if (at_ctx.at_line_end_flag == 0) {
		// Store new byte.
		at_ctx.at_command_buf[at_ctx.at_command_buf_idx] = rx_byte;
		// Manage index.
		at_ctx.at_command_buf_idx++;
		if (at_ctx.at_command_buf_idx >= AT_COMMAND_BUFFER_LENGTH) {
			at_ctx.at_command_buf_idx = 0;
		}
	}
	// Set line end flag to trigger decoding.
	if (rx_byte == STRING_CHAR_LF) {
		at_ctx.at_line_end_flag = 1;
	}
}

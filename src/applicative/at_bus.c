/*
 * at.c
 *
 *  Created on: 18 apr. 2020
 *      Author: Ludo
 */

#include "at_bus.h"

#include "aes.h"
#include "bpsm_reg.h"
#include "ddrm_reg.h"
#include "common_reg.h"
#include "dinfox.h"
#include "error.h"
#include "gpsm_reg.h"
#include "lvrm_reg.h"
#include "mode.h"
#include "node.h"
#include "nvic.h"
#include "parser.h"
#include "pwr.h"
#include "rrm_reg.h"
#include "sigfox_api.h"
#include "sm_reg.h"
#include "string.h"
#include "types.h"
#include "uhfm.h"
#include "uhfm_reg.h"

/*** AT local macros ***/

// Commands.
#define AT_BUS_COMMAND_BUFFER_SIZE			128
// Parameters separator.
#define AT_BUS_CHAR_SEPARATOR				','
// Replies.
#define AT_BUS_REPLY_BUFFER_SIZE			128
#define AT_BUS_STRING_VALUE_BUFFER_SIZE		16
#define AT_BUS_FRAME_END					STRING_CHAR_CR
#define AT_BUS_REPLY_TAB					"     "
#ifdef UHFM
// Duration of RSSI command.
#define AT_BUS_RSSI_REPORT_PERIOD_MS		500
#endif

/*** AT callbacks declaration ***/

static void _AT_BUS_print_ok(void);
#ifdef ATM
static void _AT_BUS_print_command_list(void);
static void _AT_BUS_print_sw_version(void);
static void _AT_BUS_print_error_stack(void);
static void _AT_BUS_adc_callback(void);
#endif
static void _AT_BUS_read_callback(void);
static void _AT_BUS_write_callback(void);
#if (defined UHFM) && (defined ATM)
static void _AT_BUS_nvmr_callback(void);
static void _AT_BUS_nvm_callback(void);
static void _AT_BUS_get_id_callback(void);
static void _AT_BUS_set_id_callback(void);
static void _AT_BUS_get_key_callback(void);
static void _AT_BUS_set_key_callback(void);
static void _AT_BUS_so_callback(void);
static void _AT_BUS_sb_callback(void);
static void _AT_BUS_sf_callback(void);
static void _AT_BUS_print_dl_payload(void);
static void _AT_BUS_tm_callback(void);
static void _AT_BUS_cw_callback(void);
static void _AT_BUS_dl_callback(void);
static void _AT_BUS_rssi_callback(void);
#endif /* UHFM */
#if (defined GPSM) && (defined ATM)
static void _AT_BUS_time_callback(void);
static void _AT_BUS_gps_callback(void);
static void _AT_BUS_pulse_callback(void);
#endif /* GPSM */

/*** AT local structures ***/

typedef struct {
	PARSER_mode_t mode;
	char_t* syntax;
	char_t* parameters;
	char_t* description;
	void (*callback)(void);
} AT_BUS_command_t;

typedef struct {
	// Command.
	volatile char_t command[AT_BUS_COMMAND_BUFFER_SIZE];
	volatile uint32_t command_size;
	volatile uint8_t line_end_flag;
	PARSER_context_t parser;
	// Replies.
	char_t reply[AT_BUS_REPLY_BUFFER_SIZE];
	uint32_t reply_size;
} AT_BUS_context_t;

/*** AT local global variables ***/

static const AT_BUS_command_t AT_BUS_COMMAND_LIST[] = {
#ifdef ATM
	{PARSER_MODE_COMMAND, "AT", STRING_NULL, "Ping command", _AT_BUS_print_ok},
	{PARSER_MODE_COMMAND, "AT?", STRING_NULL, "List all available commands", _AT_BUS_print_command_list},
	{PARSER_MODE_COMMAND, "AT$V?", STRING_NULL, "Get SW version", _AT_BUS_print_sw_version},
	{PARSER_MODE_COMMAND, "AT$ERROR?", STRING_NULL, "Read error stack", _AT_BUS_print_error_stack},
	{PARSER_MODE_COMMAND, "AT$RST", STRING_NULL, "Reset MCU", PWR_software_reset},
	{PARSER_MODE_COMMAND, "AT$ADC?", STRING_NULL, "Get ADC measurements", _AT_BUS_adc_callback},
#endif
	{PARSER_MODE_HEADER,  "AT$R=", "reg_addr[hex]", "Read register", _AT_BUS_read_callback},
	{PARSER_MODE_HEADER,  "AT$W=", "reg_addr[hex],reg_value[hex],(reg_mask[hex])", "Write register",_AT_BUS_write_callback},
#if (defined UHFM) && (defined ATM)
	{PARSER_MODE_COMMAND, "AT$NVMR", STRING_NULL, "Reset NVM data", _AT_BUS_nvmr_callback},
	{PARSER_MODE_HEADER,  "AT$NVM=", "address[dec]", "Get NVM data", _AT_BUS_nvm_callback},
	{PARSER_MODE_COMMAND, "AT$ID?", STRING_NULL, "Get Sigfox device ID", _AT_BUS_get_id_callback},
	{PARSER_MODE_HEADER,  "AT$ID=", "id[hex]", "Set Sigfox device ID", _AT_BUS_set_id_callback},
	{PARSER_MODE_COMMAND, "AT$KEY?", STRING_NULL, "Get Sigfox device key", _AT_BUS_get_key_callback},
	{PARSER_MODE_HEADER,  "AT$KEY=", "key[hex]", "Set Sigfox device key", _AT_BUS_set_key_callback},
	{PARSER_MODE_COMMAND, "AT$SO", STRING_NULL, "Sigfox send control message", _AT_BUS_so_callback},
	{PARSER_MODE_HEADER,  "AT$SB=", "data[bit],(bidir_flag[bit])", "Sigfox send bit", _AT_BUS_sb_callback},
	{PARSER_MODE_HEADER,  "AT$SF=", "data[hex],(bidir_flag[bit])", "Sigfox send frame", _AT_BUS_sf_callback},
	{PARSER_MODE_COMMAND, "AT$DL?", STRING_NULL, "Read last DL payload", _AT_BUS_print_dl_payload},
	{PARSER_MODE_HEADER,  "AT$TM=", "rc_index[dec],test_mode[dec]", "Execute Sigfox test mode", _AT_BUS_tm_callback},
	{PARSER_MODE_HEADER,  "AT$CW=", "frequency[hz],enable[bit],(output_power[dbm])", "Start or stop continuous radio transmission", _AT_BUS_cw_callback},
	{PARSER_MODE_HEADER,  "AT$DL=", "frequency[hz]", "Continuous downlink frames decoding", _AT_BUS_dl_callback},
	{PARSER_MODE_HEADER,  "AT$RSSI=", "frequency[hz],duration[s]", "Start or stop continuous RSSI measurement", _AT_BUS_rssi_callback},
#endif /* UHFM */
#if (defined GPSM) && (defined ATM)
	{PARSER_MODE_HEADER,  "AT$TIME=", "timeout[s]", "Get GPS time", _AT_BUS_time_callback},
	{PARSER_MODE_HEADER,  "AT$GPS=", "timeout[s]", "Get GPS position", _AT_BUS_gps_callback},
	{PARSER_MODE_HEADER,  "AT$PULSE=", "enable[bit],frequency[hz],duty_cycle[percent]", "Start or stop GPS timepulse signal", _AT_BUS_pulse_callback},
#endif
};

static AT_BUS_context_t at_bus_ctx;

/*** AT local functions ***/

/* GENERIC MACRO TO ADD A CHARACTER TO THE REPLY BUFFER.
 * @param character:	Character to add.
 * @return:				None.
 */
#define _AT_BUS_reply_add_char(character) { \
	at_bus_ctx.reply[at_bus_ctx.reply_size] = character; \
	at_bus_ctx.reply_size = (at_bus_ctx.reply_size + 1) % AT_BUS_REPLY_BUFFER_SIZE; \
}

/* APPEND A STRING TO THE REPONSE BUFFER.
 * @param tx_string:	String to add.
 * @return:				None.
 */
static void _AT_BUS_reply_add_string(char_t* tx_string) {
	// Fill reply buffer with new bytes.
	while (*tx_string) {
		_AT_BUS_reply_add_char(*(tx_string++));
	}
}

/* APPEND A VALUE TO THE REPLY BUFFER.
 * @param tx_value:		Value to add.
 * @param format:       Printing format.
 * @param print_prefix: Print base prefix is non zero.
 * @return:				None.
 */
static void _AT_BUS_reply_add_value(int32_t tx_value, STRING_format_t format, uint8_t print_prefix) {
	// Local variables.
	STRING_status_t string_status = STRING_SUCCESS;
	char_t str_value[AT_BUS_STRING_VALUE_BUFFER_SIZE];
	uint8_t idx = 0;
	// Reset string.
	for (idx=0 ; idx<AT_BUS_STRING_VALUE_BUFFER_SIZE ; idx++) str_value[idx] = STRING_CHAR_NULL;
	// Convert value to string.
	string_status = STRING_value_to_string(tx_value, format, print_prefix, str_value);
	STRING_error_check();
	// Add string.
	_AT_BUS_reply_add_string(str_value);
}

/* APPEND A REGISTER VALUE TO THE REPLY BUFFER.
 * @param reg_value:	Register value to add.
 * @return:				None.
 */
static void _AT_BUS_reply_add_register(uint32_t reg_value) {
	// Local variables.
	STRING_status_t string_status = STRING_SUCCESS;
	char_t str_value[AT_BUS_STRING_VALUE_BUFFER_SIZE] = {STRING_CHAR_NULL};
	// Convert register to string.
	string_status = DINFOX_register_to_string(reg_value, str_value);
	STRING_error_check();
	// Add string.
	_AT_BUS_reply_add_string(str_value);
}

/* SEND AT REPONSE OVER AT INTERFACE.
 * @param:	None.
 * @return:	None.
 */
static void _AT_BUS_reply_send(void) {
	// Local variables.
	LBUS_status_t lbus_status = LBUS_SUCCESS;
	// Add ending character.
	_AT_BUS_reply_add_char(AT_BUS_FRAME_END);
	// Send reply.
	lbus_status = LBUS_send((uint8_t*) at_bus_ctx.reply, at_bus_ctx.reply_size);
	LBUS_error_check();
	// Flush response buffer.
	at_bus_ctx.reply_size = 0;
}

/* PRINT OK THROUGH AT INTERFACE.
 * @param:	None.
 * @return:	None.
 */
static void _AT_BUS_print_ok(void) {
	_AT_BUS_reply_add_string("OK");
	_AT_BUS_reply_send();
}

/* PRINT ERROR THROUGH AT INTERFACE.
 * @param error:	Error to print.
 * @return:			None.
 */
static void _AT_BUS_print_error(ERROR_t error) {
	// Add error to stack.
	ERROR_stack_add(error);
	// Print error.
	_AT_BUS_reply_add_string("ERROR_");
	if (error < 0x0100) {
		_AT_BUS_reply_add_value(0, STRING_FORMAT_HEXADECIMAL, 1);
		_AT_BUS_reply_add_value((int32_t) error, STRING_FORMAT_HEXADECIMAL, 0);
	}
	else {
		_AT_BUS_reply_add_value((int32_t) error, STRING_FORMAT_HEXADECIMAL, 1);
	}
	_AT_BUS_reply_send();
}

/* AT$R EXECUTION CALLBACK.
 * @param:	None.
 * @return:	None.
 */
static void _AT_BUS_read_callback(void) {
	// Local variables.
	PARSER_status_t parser_status = PARSER_SUCCESS;
	NODE_status_t node_status = NODE_SUCCESS;
	uint32_t reg_addr = 0;
	uint32_t reg_value = 0;
	// Read address parameter.
	parser_status = DINFOX_parse_register(&at_bus_ctx.parser, STRING_CHAR_NULL, &reg_addr);
	PARSER_error_check_print();
	// Read register.
	node_status = NODE_read_register(NODE_REQUEST_SOURCE_EXTERNAL, reg_addr, &reg_value);
	NODE_error_check_print();
	// Send reply.
	_AT_BUS_reply_add_register(reg_value);
	_AT_BUS_reply_send();
errors:
	return;
}

/* AT$W EXECUTION CALLBACK.
 * @param:	None.
 * @return:	None.
 */
static void _AT_BUS_write_callback(void) {
	// Local variables.
	PARSER_status_t parser_status = PARSER_SUCCESS;
	NODE_status_t node_status = NODE_SUCCESS;
	uint32_t reg_addr = 0;
	uint32_t reg_value = 0;
	uint32_t reg_mask = 0;
	// Read address parameter.
	parser_status = DINFOX_parse_register(&at_bus_ctx.parser, AT_BUS_CHAR_SEPARATOR, &reg_addr);
	PARSER_error_check_print();
	// First try with 3 parameters.
	parser_status = DINFOX_parse_register(&at_bus_ctx.parser, AT_BUS_CHAR_SEPARATOR, &reg_value);
	if (parser_status == PARSER_SUCCESS) {
		// Try parsing register mask parameter.
		parser_status = DINFOX_parse_register(&at_bus_ctx.parser, STRING_CHAR_NULL, &reg_mask);
		PARSER_error_check_print();
	}
	else {
		// Try with only 2 parameters.
		parser_status = DINFOX_parse_register(&at_bus_ctx.parser, STRING_CHAR_NULL, &reg_value);
		PARSER_error_check_print();
		// Perform full write operation since mask is not given.
		reg_mask = DINFOX_REG_MASK_ALL;
	}
	// Write register.
	node_status = NODE_write_register(NODE_REQUEST_SOURCE_EXTERNAL, reg_addr, reg_mask, reg_value);
	NODE_error_check_print();
	// Operation completed.
	_AT_BUS_print_ok();
errors:
	return;
}

#ifdef ATM
/* PRINT ALL SUPPORTED AT COMMANDS.
 * @param:	None.
 * @return:	None.
 */
static void _AT_BUS_print_command_list(void) {
	// Local variables.
	uint32_t idx = 0;
	// Commands loop.
	for (idx=0 ; idx<(sizeof(AT_BUS_COMMAND_LIST) / sizeof(AT_BUS_command_t)) ; idx++) {
		// Print syntax.
		_AT_BUS_reply_add_string(AT_BUS_COMMAND_LIST[idx].syntax);
		// Print parameters.
		_AT_BUS_reply_add_string(AT_BUS_COMMAND_LIST[idx].parameters);
		_AT_BUS_reply_send();
		// Print description.
		_AT_BUS_reply_add_string(AT_BUS_REPLY_TAB);
		_AT_BUS_reply_add_string(AT_BUS_COMMAND_LIST[idx].description);
		_AT_BUS_reply_send();
	}
	_AT_BUS_print_ok();
}
#endif

#ifdef ATM
/* PRINT SW VERSION.
 * @param:	None.
 * @return:	None.
 */
static void _AT_BUS_print_sw_version(void) {
	// Local variables.
	NODE_status_t node_status = NODE_SUCCESS;
	uint32_t generic_u32 = 0;
	_AT_BUS_reply_add_string("SW");
	// Major version.
	node_status = NODE_read_field(NODE_REQUEST_SOURCE_EXTERNAL, COMMON_REG_ADDR_SW_VERSION_0, COMMON_REG_SW_VERSION_0_MASK_MAJOR, &generic_u32);
	NODE_error_check_print();
	_AT_BUS_reply_add_value((int32_t) generic_u32, STRING_FORMAT_DECIMAL, 0);
	// Minor version.
	node_status = NODE_read_field(NODE_REQUEST_SOURCE_EXTERNAL, COMMON_REG_ADDR_SW_VERSION_0, COMMON_REG_SW_VERSION_0_MASK_MINOR, &generic_u32);
	NODE_error_check_print();
	_AT_BUS_reply_add_string(".");
	_AT_BUS_reply_add_value((int32_t) generic_u32, STRING_FORMAT_DECIMAL, 0);
	// Commit index.
	node_status = NODE_read_field(NODE_REQUEST_SOURCE_EXTERNAL, COMMON_REG_ADDR_SW_VERSION_0, COMMON_REG_SW_VERSION_0_MASK_COMMIT_INDEX, &generic_u32);
	NODE_error_check_print();
	_AT_BUS_reply_add_string(".");
	_AT_BUS_reply_add_value((int32_t) generic_u32, STRING_FORMAT_DECIMAL, 0);
	// Dirty flag.
	node_status = NODE_read_field(NODE_REQUEST_SOURCE_EXTERNAL, COMMON_REG_ADDR_SW_VERSION_0, COMMON_REG_SW_VERSION_0_MASK_DTYF, &generic_u32);
	NODE_error_check_print();
	if (generic_u32 != 0) {
		_AT_BUS_reply_add_string(".d");
	}
	// Commit ID.
	node_status = NODE_read_field(NODE_REQUEST_SOURCE_EXTERNAL, COMMON_REG_ADDR_SW_VERSION_1, COMMON_REG_SW_VERSION_1_MASK_COMMIT_ID, &generic_u32);
	NODE_error_check_print();
	_AT_BUS_reply_add_string(" (");
	_AT_BUS_reply_add_value((int32_t) generic_u32, STRING_FORMAT_HEXADECIMAL, 1);
	_AT_BUS_reply_add_string(")");
	_AT_BUS_reply_send();
	_AT_BUS_print_ok();
errors:
	return;
}
#endif

#ifdef ATM
/* PRINT ERROR STACK.
 * @param:	None.
 * @return:	None.
 */
static void _AT_BUS_print_error_stack(void) {
	// Local variables.
	NODE_status_t node_status = NODE_SUCCESS;
	uint32_t generic_u32 = 0;
	ERROR_t error = SUCCESS;
	// Read stack.
	if (ERROR_stack_is_empty() != 0) {
		_AT_BUS_reply_add_string("Error stack empty");
	}
	else {
		// Unstack all errors.
		_AT_BUS_reply_add_string("[ ");
		do {
			// Read error stack.
			node_status = NODE_read_field(NODE_REQUEST_SOURCE_EXTERNAL, COMMON_REG_ADDR_ERROR_STACK, COMMON_REG_ERROR_STACK_MASK_ERROR, &generic_u32);
			NODE_error_check_print();
			error = (ERROR_t) generic_u32;
			// Check value.
			if (error != SUCCESS) {
				_AT_BUS_reply_add_value((int32_t) error, STRING_FORMAT_HEXADECIMAL, 1);
				_AT_BUS_reply_add_string(" ");
			}
		}
		while (error != SUCCESS);
		_AT_BUS_reply_add_string("]");
	}
	_AT_BUS_reply_send();
	_AT_BUS_print_ok();
errors:
	return;
}
#endif

#ifdef ATM
/* AT$ADC? EXECUTION CALLBACK.
 * @param:	None.
 * @return:	None.
 */
static void _AT_BUS_adc_callback(void) {
	// Local variables.
	NODE_status_t node_status = NODE_SUCCESS;
	uint32_t generic_u32 = 0;
	// Trigger measurements.
	node_status = NODE_write_field(NODE_REQUEST_SOURCE_EXTERNAL, COMMON_REG_ADDR_STATUS_CONTROL_0, COMMON_REG_STATUS_CONTROL_0_MASK_MTRG, 0b1);
	NODE_error_check_print();
	// Read and print data.
	// MCU voltage.
	_AT_BUS_reply_add_string("Vmcu=");
	node_status = NODE_read_field(NODE_REQUEST_SOURCE_EXTERNAL, COMMON_REG_ADDR_ANALOG_DATA_0, COMMON_REG_ANALOG_DATA_0_MASK_VMCU, &generic_u32);
	NODE_error_check_print();
	_AT_BUS_reply_add_value((int32_t) DINFOX_get_mv((uint16_t) generic_u32), STRING_FORMAT_DECIMAL, 0);
	_AT_BUS_reply_add_string("mV");
	_AT_BUS_reply_send();
	// MCU temperature.
	_AT_BUS_reply_add_string("Tmcu=");
	node_status = NODE_read_field(NODE_REQUEST_SOURCE_EXTERNAL, COMMON_REG_ADDR_ANALOG_DATA_0, COMMON_REG_ANALOG_DATA_0_MASK_TMCU, &generic_u32);
	NODE_error_check_print();
	_AT_BUS_reply_add_value((int32_t) DINFOX_get_degrees((uint8_t) generic_u32), STRING_FORMAT_DECIMAL, 0);
	_AT_BUS_reply_add_string("dC");
	_AT_BUS_reply_send();
	// Input voltage.
#ifdef LVRM
	_AT_BUS_reply_add_string("Vcom=");
	node_status = NODE_read_field(NODE_REQUEST_SOURCE_EXTERNAL, LVRM_REG_ADDR_ANALOG_DATA_1, LVRM_REG_ANALOG_DATA_1_MASK_VCOM, &generic_u32);
#endif
#ifdef BPSM
	_AT_BUS_reply_add_string("Vsrc=");
	node_status = NODE_read_field(NODE_REQUEST_SOURCE_EXTERNAL, BPSM_REG_ADDR_ANALOG_DATA_1, BPSM_REG_ANALOG_DATA_1_MASK_VSRC, &generic_u32);
#endif
#ifdef DDRM
	_AT_BUS_reply_add_string("Vin=");
	node_status = NODE_read_field(NODE_REQUEST_SOURCE_EXTERNAL, DDRM_REG_ADDR_ANALOG_DATA_1, DDRM_REG_ANALOG_DATA_1_MASK_VIN, &generic_u32);
#endif
#ifdef RRM
	_AT_BUS_reply_add_string("Vin=");
	node_status = NODE_read_field(NODE_REQUEST_SOURCE_EXTERNAL, RRM_REG_ADDR_ANALOG_DATA_1, RRM_REG_ANALOG_DATA_1_MASK_VIN, &generic_u32);
#endif
#if (defined LVRM) || (defined BPSM) || (defined DDRM) || (defined RRM)
	NODE_error_check_print();
	_AT_BUS_reply_add_value((int32_t) DINFOX_get_mv((uint16_t) generic_u32), STRING_FORMAT_DECIMAL, 0);
	_AT_BUS_reply_add_string("mV");
	_AT_BUS_reply_send();
#endif
	// Storage element voltage.
#ifdef BPSM
	_AT_BUS_reply_add_string("Vstr=");
	node_status = NODE_read_field(NODE_REQUEST_SOURCE_EXTERNAL, BPSM_REG_ADDR_ANALOG_DATA_1, BPSM_REG_ANALOG_DATA_1_MASK_VSTR, &generic_u32);
	NODE_error_check_print();
	_AT_BUS_reply_add_value((int32_t) DINFOX_get_mv((uint16_t) generic_u32), STRING_FORMAT_DECIMAL, 0);
	_AT_BUS_reply_add_string("mV");
	_AT_BUS_reply_send();
#endif
	// Output voltage.
#ifdef LVRM
	_AT_BUS_reply_add_string("Vout=");
	node_status = NODE_read_field(NODE_REQUEST_SOURCE_EXTERNAL, LVRM_REG_ADDR_ANALOG_DATA_1, LVRM_REG_ANALOG_DATA_1_MASK_VOUT, &generic_u32);
#endif
#ifdef DDRM
	_AT_BUS_reply_add_string("Vout=");
	node_status = NODE_read_field(NODE_REQUEST_SOURCE_EXTERNAL, DDRM_REG_ADDR_ANALOG_DATA_1, DDRM_REG_ANALOG_DATA_1_MASK_VOUT, &generic_u32);
#endif
#ifdef RRM
	_AT_BUS_reply_add_string("Vout=");
	node_status = NODE_read_field(NODE_REQUEST_SOURCE_EXTERNAL, RRM_REG_ADDR_ANALOG_DATA_1, RRM_REG_ANALOG_DATA_1_MASK_VOUT, &generic_u32);
#endif
#ifdef BPSM
	_AT_BUS_reply_add_string("Vbkp=");
	node_status = NODE_read_field(NODE_REQUEST_SOURCE_EXTERNAL, BPSM_REG_ADDR_ANALOG_DATA_2, BPSM_REG_ANALOG_DATA_2_MASK_VBKP, &generic_u32);
#endif
#if (defined LVRM) || (defined BPSM) || (defined DDRM) || (defined RRM)
	NODE_error_check_print();
	_AT_BUS_reply_add_value((int32_t) DINFOX_get_mv((uint16_t) generic_u32), STRING_FORMAT_DECIMAL, 0);
	_AT_BUS_reply_add_string("mV");
	_AT_BUS_reply_send();
#endif
	// Output current.
#ifdef LVRM
	_AT_BUS_reply_add_string("Iout=");
	node_status = NODE_read_field(NODE_REQUEST_SOURCE_EXTERNAL, LVRM_REG_ADDR_ANALOG_DATA_2, LVRM_REG_ANALOG_DATA_2_MASK_IOUT, &generic_u32);
#endif
#ifdef DDRM
	_AT_BUS_reply_add_string("Iout=");
	node_status = NODE_read_field(NODE_REQUEST_SOURCE_EXTERNAL, DDRM_REG_ADDR_ANALOG_DATA_2, DDRM_REG_ANALOG_DATA_2_MASK_IOUT, &generic_u32);
#endif
#ifdef RRM
	_AT_BUS_reply_add_string("Iout=");
	node_status = NODE_read_field(NODE_REQUEST_SOURCE_EXTERNAL, RRM_REG_ADDR_ANALOG_DATA_2, RRM_REG_ANALOG_DATA_2_MASK_IOUT, &generic_u32);
#endif
#if (defined LVRM) || (defined DDRM) || (defined RRM)
	_AT_BUS_reply_add_value((int32_t) DINFOX_get_ua((uint16_t) generic_u32), STRING_FORMAT_DECIMAL, 0);
	_AT_BUS_reply_add_string("uA");
	_AT_BUS_reply_send();
#endif
	// AINx voltage.
#if (defined SM) && (defined SM_AIN_ENABLE)
	// AIN0 voltage.
	_AT_BUS_reply_add_string("AIN0=");
	node_status = NODE_read_field(NODE_REQUEST_SOURCE_EXTERNAL, SM_REG_ADDR_ANALOG_DATA_1, SM_REG_ANALOG_DATA_1_MASK_VAIN0, &generic_u32);
	NODE_error_check_print();
	_AT_BUS_reply_add_value((int32_t) DINFOX_get_mv((uint16_t) generic_u32), STRING_FORMAT_DECIMAL, 0);
	_AT_BUS_reply_add_string("mV");
	_AT_BUS_reply_send();
	// AIN1 voltage.
	_AT_BUS_reply_add_string("AIN1=");
	node_status = NODE_read_field(NODE_REQUEST_SOURCE_EXTERNAL, SM_REG_ADDR_ANALOG_DATA_1, SM_REG_ANALOG_DATA_1_MASK_VAIN1, &generic_u32);
	NODE_error_check_print();
	_AT_BUS_reply_add_value((int32_t) DINFOX_get_mv((uint16_t) generic_u32), STRING_FORMAT_DECIMAL, 0);
	_AT_BUS_reply_add_string("mV");
	_AT_BUS_reply_send();
	// AIN2 voltage.
	_AT_BUS_reply_add_string("AIN2=");
	node_status = NODE_read_field(NODE_REQUEST_SOURCE_EXTERNAL, SM_REG_ADDR_ANALOG_DATA_2, SM_REG_ANALOG_DATA_2_MASK_VAIN2, &generic_u32);
	NODE_error_check_print();
	_AT_BUS_reply_add_value((int32_t) DINFOX_get_mv((uint16_t) generic_u32), STRING_FORMAT_DECIMAL, 0);
	_AT_BUS_reply_add_string("mV");
	_AT_BUS_reply_send();
	// AIN3 voltage.
	_AT_BUS_reply_add_string("AIN3=");
	node_status = NODE_read_field(NODE_REQUEST_SOURCE_EXTERNAL, SM_REG_ADDR_ANALOG_DATA_2, SM_REG_ANALOG_DATA_2_MASK_VAIN3, &generic_u32);
	NODE_error_check_print();
	_AT_BUS_reply_add_value((int32_t) DINFOX_get_mv((uint16_t) generic_u32), STRING_FORMAT_DECIMAL, 0);
	_AT_BUS_reply_add_string("mV");
	_AT_BUS_reply_send();
#endif
	// Radio voltages.
#ifdef UHFM
	// TX voltage.
	_AT_BUS_reply_add_string("Vrf_tx=");
	node_status = NODE_read_field(NODE_REQUEST_SOURCE_EXTERNAL, UHFM_REG_ADDR_ANALOG_DATA_1, UHFM_REG_ANALOG_DATA_1_MASK_VRF_TX, &generic_u32);
	NODE_error_check_print();
	_AT_BUS_reply_add_value((int32_t) DINFOX_get_mv((uint16_t) generic_u32), STRING_FORMAT_DECIMAL, 0);
	_AT_BUS_reply_add_string("mV");
	_AT_BUS_reply_send();
	// RX voltage.
	_AT_BUS_reply_add_string("Vrf_rx=");
	node_status = NODE_read_field(NODE_REQUEST_SOURCE_EXTERNAL, UHFM_REG_ADDR_ANALOG_DATA_1, UHFM_REG_ANALOG_DATA_1_MASK_VRF_RX, &generic_u32);
	NODE_error_check_print();
	_AT_BUS_reply_add_value((int32_t) DINFOX_get_mv((uint16_t) generic_u32), STRING_FORMAT_DECIMAL, 0);
	_AT_BUS_reply_add_string("mV");
	_AT_BUS_reply_send();
#endif
#ifdef GPSM
	// GPS voltage.
	_AT_BUS_reply_add_string("Vgps=");
	node_status = NODE_read_field(NODE_REQUEST_SOURCE_EXTERNAL, GPSM_REG_ADDR_ANALOG_DATA_1, GPSM_REG_ANALOG_DATA_1_MASK_VGPS, &generic_u32);
	NODE_error_check_print();
	_AT_BUS_reply_add_value((int32_t) DINFOX_get_mv((uint16_t) generic_u32), STRING_FORMAT_DECIMAL, 0);
	_AT_BUS_reply_add_string("mV");
	_AT_BUS_reply_send();
#ifdef GPSM_ACTIVE_ANTENNA
	// Active antenna voltage.
	_AT_BUS_reply_add_string("Vant=");
	node_status = NODE_read_field(NODE_REQUEST_SOURCE_EXTERNAL, GPSM_REG_ADDR_ANALOG_DATA_1, GPSM_REG_ANALOG_DATA_1_MASK_VANT, &generic_u32);
	NODE_error_check_print();
	_AT_BUS_reply_add_value((int32_t) DINFOX_get_mv((uint16_t) generic_u32), STRING_FORMAT_DECIMAL, 0);
	_AT_BUS_reply_add_string("mV");
	_AT_BUS_reply_send();
#endif
#endif
	_AT_BUS_print_ok();
errors:
	return;
}
#endif

#if (defined UHFM) && (defined ATM)
/* AT$NVMR EXECUTION CALLBACK.
 * @param:	None.
 * @return:	None.
 */
static void _AT_BUS_nvmr_callback(void) {
	// Local variables.
	NVM_status_t nvm_status = NVM_SUCCESS;
	// Reset all NVM field to default value.
	nvm_status = NVM_reset_default();
	NVM_error_check_print();
	_AT_BUS_print_ok();
errors:
	return;
}
#endif

#if (defined UHFM) && (defined ATM)
/* AT$NVM EXECUTION CALLBACK.
 * @param:	None.
 * @return:	None.
 */
static void _AT_BUS_nvm_callback(void) {
	// Local variables.
	PARSER_status_t parser_status = PARSER_ERROR_UNKNOWN_COMMAND;
	NVM_status_t nvm_status = NVM_SUCCESS;
	int32_t address = 0;
	uint8_t nvm_data = 0;
	// Read address parameters.
	parser_status = PARSER_get_parameter(&at_bus_ctx.parser, STRING_FORMAT_DECIMAL, STRING_CHAR_NULL, &address);
	PARSER_error_check_print();
	// Read byte at requested address.
	nvm_status = NVM_read_byte((uint16_t) address, &nvm_data);
	NVM_error_check_print();
	// Print data.
	_AT_BUS_reply_add_value(nvm_data, STRING_FORMAT_HEXADECIMAL, 1);
	_AT_BUS_reply_send();
	_AT_BUS_print_ok();
errors:
	return;
}
#endif

#if (defined UHFM) && (defined ATM)
/* AT$ID? EXECUTION CALLBACK.
 * @param:	None.
 * @return:	None.
 */
static void _AT_BUS_get_id_callback(void) {
	// Local variables.
	NODE_status_t node_status = NODE_SUCCESS;
	uint8_t sigfox_ep_id[ID_LENGTH];
	uint8_t idx = 0;
	// Read ID.
	node_status = NODE_read_byte_array(NODE_REQUEST_SOURCE_EXTERNAL, UHFM_REG_ADDR_SIGFOX_EP_ID, sigfox_ep_id, ID_LENGTH);
	NODE_error_check_print();
	// Print ID.
	for (idx=0 ; idx<ID_LENGTH ; idx++) {
		_AT_BUS_reply_add_value(sigfox_ep_id[idx], STRING_FORMAT_HEXADECIMAL, (idx==0 ? 1 : 0));
	}
	_AT_BUS_reply_send();
	_AT_BUS_print_ok();
errors:
	return;
}
#endif

#if (defined UHFM) && (defined ATM)
/* AT$ID EXECUTION CALLBACK.
 * @param:	None.
 * @return:	None.
 */
static void _AT_BUS_set_id_callback(void) {
	// Local variables.
	PARSER_status_t parser_status = PARSER_ERROR_UNKNOWN_COMMAND;
	NODE_status_t node_status = NODE_SUCCESS;
	NVM_status_t nvm_status = NVM_SUCCESS;
	uint8_t sigfox_ep_id[ID_LENGTH];
	uint8_t extracted_length = 0;
	uint8_t idx = 0;
	// Read ID parameter.
	parser_status = PARSER_get_byte_array(&at_bus_ctx.parser, STRING_CHAR_NULL, ID_LENGTH, 1, sigfox_ep_id, &extracted_length);
	PARSER_error_check_print();
	// Write device ID in NVM.
	for (idx=0 ; idx<ID_LENGTH ; idx++) {
		nvm_status = NVM_write_byte((NVM_ADDRESS_SIGFOX_DEVICE_ID + idx), sigfox_ep_id[idx]);
		NVM_error_check_print();
	}
	// Update register.
	node_status = NODE_write_byte_array(NODE_REQUEST_SOURCE_INTERNAL, UHFM_REG_ADDR_SIGFOX_EP_ID, (uint8_t*) sigfox_ep_id, ID_LENGTH);
	NODE_error_check_print();
	_AT_BUS_print_ok();
errors:
	return;
}
#endif

#if (defined UHFM) && (defined ATM)
/* AT$KEY? EXECUTION CALLBACK.
 * @param:	None.
 * @return:	None.
 */
static void _AT_BUS_get_key_callback(void) {
	// Local variables.
	NODE_status_t node_status = NODE_SUCCESS;
	uint8_t sigfox_ep_key[AES_BLOCK_SIZE];
	uint8_t idx = 0;
	// Read key.
	node_status = NODE_read_byte_array(NODE_REQUEST_SOURCE_EXTERNAL, UHFM_REG_ADDR_SIGFOX_EP_KEY_0, sigfox_ep_key, AES_BLOCK_SIZE);
	NODE_error_check_print();
	// Print key.
	for (idx=0 ; idx<AES_BLOCK_SIZE ; idx++) {
		_AT_BUS_reply_add_value(sigfox_ep_key[idx], STRING_FORMAT_HEXADECIMAL, (idx==0 ? 1 : 0));
	}
	_AT_BUS_reply_send();
	_AT_BUS_print_ok();
errors:
	return;
}
#endif

#if (defined UHFM) && (defined ATM)
/* AT$KEY EXECUTION CALLBACK.
 * @param:	None.
 * @return:	None.
 */
static void _AT_BUS_set_key_callback(void) {
	// Local variables.
	PARSER_status_t parser_status = PARSER_ERROR_UNKNOWN_COMMAND;
	NODE_status_t node_status = NODE_SUCCESS;
	NVM_status_t nvm_status = NVM_SUCCESS;
	uint8_t sigfox_ep_key[AES_BLOCK_SIZE];
	uint8_t extracted_length = 0;
	uint8_t idx = 0;
	// Read key parameter.
	parser_status = PARSER_get_byte_array(&at_bus_ctx.parser, STRING_CHAR_NULL, AES_BLOCK_SIZE, 1, sigfox_ep_key, &extracted_length);
	PARSER_error_check_print();
	// Write device ID in NVM.
	for (idx=0 ; idx<AES_BLOCK_SIZE ; idx++) {
		nvm_status = NVM_write_byte((NVM_ADDRESS_SIGFOX_DEVICE_KEY + idx), sigfox_ep_key[idx]);
		NVM_error_check_print();
	}
	// Update registers.
	node_status = NODE_write_byte_array(NODE_REQUEST_SOURCE_INTERNAL, UHFM_REG_ADDR_SIGFOX_EP_KEY_0, (uint8_t*) sigfox_ep_key, AES_BLOCK_SIZE);
	NODE_error_check_print();
	_AT_BUS_print_ok();
errors:
	return;
}
#endif

#if (defined UHFM) && (defined ATM)
/* AT$SO EXECUTION CALLBACK.
 * @param:	None.
 * @return:	None.
 */
static void _AT_BUS_so_callback(void) {
	// Local variables.
	NODE_status_t node_status = NODE_SUCCESS;
	// Configure message.
	node_status = NODE_write_field(NODE_REQUEST_SOURCE_EXTERNAL, UHFM_REG_ADDR_SIGFOX_EP_CONFIGURATION_2, UHFM_REG_SIGFOX_EP_CONFIGURATION_2_MASK_PRT, (uint32_t) 0b01);
	NODE_error_check_print();
	node_status = NODE_write_field(NODE_REQUEST_SOURCE_EXTERNAL, UHFM_REG_ADDR_SIGFOX_EP_CONFIGURATION_2, UHFM_REG_SIGFOX_EP_CONFIGURATION_2_MASK_CMSG, (uint32_t) 0b1);
	NODE_error_check_print();
	node_status = NODE_write_field(NODE_REQUEST_SOURCE_EXTERNAL, UHFM_REG_ADDR_SIGFOX_EP_CONFIGURATION_2, UHFM_REG_SIGFOX_EP_CONFIGURATION_2_MASK_MSGT, (uint32_t) 0b000);
	NODE_error_check_print();
	node_status = NODE_write_field(NODE_REQUEST_SOURCE_EXTERNAL, UHFM_REG_ADDR_SIGFOX_EP_CONFIGURATION_2, UHFM_REG_SIGFOX_EP_CONFIGURATION_2_MASK_BF, (uint32_t) 0b0);
	NODE_error_check_print();
	node_status = NODE_write_field(NODE_REQUEST_SOURCE_EXTERNAL, UHFM_REG_ADDR_SIGFOX_EP_CONFIGURATION_2, UHFM_REG_SIGFOX_EP_CONFIGURATION_2_MASK_UL_PAYLOAD_SIZE, (uint32_t) 0b00000);
	NODE_error_check_print();
	// Send Sigfox message.
	node_status = NODE_write_field(NODE_REQUEST_SOURCE_EXTERNAL, UHFM_REG_ADDR_STATUS_CONTROL_1, UHFM_REG_STATUS_CONTROL_1_MASK_STRG, (uint32_t) 0b1);
	NODE_error_check_print();
	_AT_BUS_print_ok();
errors:
	return;
}
#endif

#if (defined UHFM) && (defined ATM)
/* AT$SB EXECUTION CALLBACK.
 * @param:	None.
 * @return:	None.
 */
static void _AT_BUS_sb_callback(void) {
	// Local variables.
	PARSER_status_t parser_status = PARSER_ERROR_UNKNOWN_COMMAND;
	NODE_status_t node_status = NODE_SUCCESS;
	int32_t ul_bit = 0;
	int32_t bidir_flag = 0;
	UHFM_message_status_t message_status;
	uint32_t generic_u32 = 0;
	// First try with 2 parameters.
	parser_status = PARSER_get_parameter(&at_bus_ctx.parser, STRING_FORMAT_BOOLEAN, AT_BUS_CHAR_SEPARATOR, &ul_bit);
	if (parser_status == PARSER_SUCCESS) {
		// Try parsing downlink request parameter.
		parser_status =  PARSER_get_parameter(&at_bus_ctx.parser, STRING_FORMAT_BOOLEAN, STRING_CHAR_NULL, &bidir_flag);
		PARSER_error_check_print();
	}
	else {
		// Try with 1 parameter.
		parser_status = PARSER_get_parameter(&at_bus_ctx.parser, STRING_FORMAT_BOOLEAN, STRING_CHAR_NULL, &ul_bit);
		PARSER_error_check_print();
	}
	// Configure message.
	node_status = NODE_write_field(NODE_REQUEST_SOURCE_EXTERNAL, UHFM_REG_ADDR_SIGFOX_EP_CONFIGURATION_2, UHFM_REG_SIGFOX_EP_CONFIGURATION_2_MASK_PRT, (uint32_t) 0b01);
	NODE_error_check_print();
	node_status = NODE_write_field(NODE_REQUEST_SOURCE_EXTERNAL, UHFM_REG_ADDR_SIGFOX_EP_CONFIGURATION_2, UHFM_REG_SIGFOX_EP_CONFIGURATION_2_MASK_CMSG, (uint32_t) 0b0);
	NODE_error_check_print();
	node_status = NODE_write_field(NODE_REQUEST_SOURCE_EXTERNAL, UHFM_REG_ADDR_SIGFOX_EP_CONFIGURATION_2, UHFM_REG_SIGFOX_EP_CONFIGURATION_2_MASK_MSGT, (uint32_t) (ul_bit + 1));
	NODE_error_check_print();
	node_status = NODE_write_field(NODE_REQUEST_SOURCE_EXTERNAL, UHFM_REG_ADDR_SIGFOX_EP_CONFIGURATION_2, UHFM_REG_SIGFOX_EP_CONFIGURATION_2_MASK_BF, (uint32_t) bidir_flag);
	NODE_error_check_print();
	node_status = NODE_write_field(NODE_REQUEST_SOURCE_EXTERNAL, UHFM_REG_ADDR_SIGFOX_EP_CONFIGURATION_2, UHFM_REG_SIGFOX_EP_CONFIGURATION_2_MASK_UL_PAYLOAD_SIZE, (uint32_t) 0b00000);
	NODE_error_check_print();
	// Send Sigfox message.
	node_status = NODE_write_field(NODE_REQUEST_SOURCE_EXTERNAL, UHFM_REG_ADDR_STATUS_CONTROL_1, UHFM_REG_STATUS_CONTROL_1_MASK_STRG, (uint32_t) 0b1);
	NODE_error_check_print();
	// Read message status.
	node_status = NODE_read_field(NODE_REQUEST_SOURCE_EXTERNAL, UHFM_REG_ADDR_STATUS_CONTROL_1, UHFM_REG_STATUS_CONTROL_1_MASK_MESSAGE_STATUS, &generic_u32);
	NODE_error_check_print();
	message_status.all = (uint8_t) generic_u32;
	// Print message status.
	_AT_BUS_reply_add_string("+MSG_STAT=");
	_AT_BUS_reply_add_value(message_status.all, STRING_FORMAT_HEXADECIMAL, 1);
	_AT_BUS_reply_send();
	// Print DL payload if required.
	if ((bidir_flag != 0) && (message_status.dl_frame != 0)) {
		_AT_BUS_print_dl_payload();
	}
	_AT_BUS_print_ok();
errors:
	return;
}
#endif

#if (defined UHFM) && (defined ATM)
/* AT$SF EXECUTION CALLBACK.
 * @param:	None.
 * @return:	None.
 */
static void _AT_BUS_sf_callback(void) {
	// Local variables.
	PARSER_status_t parser_status = PARSER_ERROR_UNKNOWN_COMMAND;
	NODE_status_t node_status = NODE_SUCCESS;
	uint8_t ul_payload[SIGFOX_UPLINK_DATA_MAX_SIZE_BYTES];
	uint8_t ul_payload_size = 0;
	int32_t bidir_flag = 0;
	UHFM_message_status_t message_status;
	uint32_t generic_u32 = 0;
	// First try with 2 parameters.
	parser_status = PARSER_get_byte_array(&at_bus_ctx.parser, AT_BUS_CHAR_SEPARATOR, SIGFOX_UPLINK_DATA_MAX_SIZE_BYTES, 0, ul_payload, &ul_payload_size);
	if (parser_status == PARSER_SUCCESS) {
		// Try parsing downlink request parameter.
		parser_status =  PARSER_get_parameter(&at_bus_ctx.parser, STRING_FORMAT_BOOLEAN, STRING_CHAR_NULL, &bidir_flag);
		PARSER_error_check_print();
	}
	else {
		// Try with 1 parameter.
		parser_status = PARSER_get_byte_array(&at_bus_ctx.parser, STRING_CHAR_NULL, SIGFOX_UPLINK_DATA_MAX_SIZE_BYTES, 0, ul_payload, &ul_payload_size);
		PARSER_error_check_print();
	}
	// Configure message.
	node_status = NODE_write_field(NODE_REQUEST_SOURCE_EXTERNAL, UHFM_REG_ADDR_SIGFOX_EP_CONFIGURATION_2, UHFM_REG_SIGFOX_EP_CONFIGURATION_2_MASK_PRT, (uint32_t) 0b01);
	NODE_error_check_print();
	node_status = NODE_write_field(NODE_REQUEST_SOURCE_EXTERNAL, UHFM_REG_ADDR_SIGFOX_EP_CONFIGURATION_2, UHFM_REG_SIGFOX_EP_CONFIGURATION_2_MASK_CMSG, (uint32_t) 0b0);
	NODE_error_check_print();
	node_status = NODE_write_field(NODE_REQUEST_SOURCE_EXTERNAL, UHFM_REG_ADDR_SIGFOX_EP_CONFIGURATION_2, UHFM_REG_SIGFOX_EP_CONFIGURATION_2_MASK_MSGT, (uint32_t) 3);
	NODE_error_check_print();
	node_status = NODE_write_field(NODE_REQUEST_SOURCE_EXTERNAL, UHFM_REG_ADDR_SIGFOX_EP_CONFIGURATION_2, UHFM_REG_SIGFOX_EP_CONFIGURATION_2_MASK_BF, (uint32_t) bidir_flag);
	NODE_error_check_print();
	node_status = NODE_write_field(NODE_REQUEST_SOURCE_EXTERNAL, UHFM_REG_ADDR_SIGFOX_EP_CONFIGURATION_2, UHFM_REG_SIGFOX_EP_CONFIGURATION_2_MASK_UL_PAYLOAD_SIZE, (uint32_t) ul_payload_size);
	NODE_error_check_print();
	node_status = NODE_write_byte_array(NODE_REQUEST_SOURCE_EXTERNAL, UHFM_REG_ADDR_SIGFOX_UL_PAYLOAD_0, (uint8_t*) ul_payload, ul_payload_size);
	NODE_error_check_print();
	// Send Sigfox message.
	node_status = NODE_write_field(NODE_REQUEST_SOURCE_EXTERNAL, UHFM_REG_ADDR_STATUS_CONTROL_1, UHFM_REG_STATUS_CONTROL_1_MASK_STRG, (uint32_t) 0b1);
	NODE_error_check_print();
	// Read message status.
	node_status = NODE_read_field(NODE_REQUEST_SOURCE_EXTERNAL, UHFM_REG_ADDR_STATUS_CONTROL_1, UHFM_REG_STATUS_CONTROL_1_MASK_MESSAGE_STATUS, &generic_u32);
	NODE_error_check_print();
	message_status.all = (uint8_t) generic_u32;
	// Print message status.
	_AT_BUS_reply_add_string("+MSG_STAT=");
	_AT_BUS_reply_add_value(message_status.all, STRING_FORMAT_HEXADECIMAL, 1);
	_AT_BUS_reply_send();
	// Print DL payload if required.
	if ((bidir_flag != 0) && (message_status.dl_frame != 0)) {
		_AT_BUS_print_dl_payload();
	}
	_AT_BUS_print_ok();
errors:
	return;
}
#endif

#if (defined UHFM) && (defined ATM)
/* AT$DL? EXECUTION CALLBACK.
 * @param:	None.
 * @return:	None.
 */
static void _AT_BUS_print_dl_payload(void) {
	// Local variables.
	NODE_status_t node_status = NODE_SUCCESS;
	uint8_t dl_payload[SIGFOX_DOWNLINK_DATA_SIZE_BYTES];
	uint8_t idx = 0;
	// Read registers.
	node_status = NODE_read_byte_array(NODE_REQUEST_SOURCE_EXTERNAL, UHFM_REG_ADDR_SIGFOX_DL_PAYLOAD_0, dl_payload, SIGFOX_DOWNLINK_DATA_SIZE_BYTES);
	NODE_error_check_print();
	// Print DL payload.
	_AT_BUS_reply_add_string("+RX=");
	for (idx=0 ; idx<SIGFOX_DOWNLINK_DATA_SIZE_BYTES ; idx++) {
		_AT_BUS_reply_add_value(dl_payload[idx], STRING_FORMAT_HEXADECIMAL, 0);
	}
	_AT_BUS_reply_send();
errors:
	return;
}
#endif

#if (defined UHFM) && (defined ATM)
/* PRINT SIGFOX DOWNLINK FRAME ON AT INTERFACE.
 * @param dl_payload:	Downlink data to print.
 * @return:				None.
 */
static void _AT_BUS_print_dl_phy_content(uint8_t* dl_phy_content, int32_t rssi_dbm) {
	// Local variables.
	uint8_t idx = 0;
	// Print DL-PHY content.
	_AT_BUS_reply_add_string("+DL_PHY=");
	for (idx=0 ; idx<SIGFOX_DOWNLINK_PHY_SIZE_BYTES ; idx++) {
		_AT_BUS_reply_add_value(dl_phy_content[idx], STRING_FORMAT_HEXADECIMAL, 0);
	}
	_AT_BUS_reply_add_string(" RSSI=");
	_AT_BUS_reply_add_value(rssi_dbm, STRING_FORMAT_DECIMAL, 0);
	_AT_BUS_reply_add_string("dBm");
	_AT_BUS_reply_send();
}
#endif

#if (defined UHFM) && (defined ATM)
/* AT$TM EXECUTION CALLBACK.
 * @param:	None.
 * @return:	None.
 */
static void _AT_BUS_tm_callback(void) {
	// Local variables.
	PARSER_status_t parser_status = PARSER_ERROR_UNKNOWN_COMMAND;
	NODE_status_t node_status = NODE_SUCCESS;
	int32_t rc_index = 0;
	int32_t test_mode = 0;
	// Read RC parameter.
	parser_status = PARSER_get_parameter(&at_bus_ctx.parser, STRING_FORMAT_DECIMAL, AT_BUS_CHAR_SEPARATOR, &rc_index);
	PARSER_error_check_print();
	// Read test mode parameter.
	parser_status =  PARSER_get_parameter(&at_bus_ctx.parser, STRING_FORMAT_DECIMAL, STRING_CHAR_NULL, &test_mode);
	PARSER_error_check_print();
	// Configure test mode.
	node_status = NODE_write_field(NODE_REQUEST_SOURCE_EXTERNAL, UHFM_REG_ADDR_SIGFOX_EP_CONFIGURATION_0, UHFM_REG_SIGFOX_EP_CONFIGURATION_0_MASK_RC, (uint32_t) rc_index);
	NODE_error_check_print();
	node_status = NODE_write_field(NODE_REQUEST_SOURCE_EXTERNAL, UHFM_REG_ADDR_SIGFOX_EP_CONFIGURATION_0, UHFM_REG_SIGFOX_EP_CONFIGURATION_0_MASK_TEST_MODE, (uint32_t) test_mode);
	NODE_error_check_print();
	// Perform Sigfox test mode.
	node_status = NODE_write_field(NODE_REQUEST_SOURCE_EXTERNAL, UHFM_REG_ADDR_STATUS_CONTROL_1, UHFM_REG_STATUS_CONTROL_1_MASK_TTRG, 0b1);
	NODE_error_check_print();
	_AT_BUS_print_ok();
errors:
	return;
}
#endif

#if (defined UHFM) && (defined ATM)
/* AT$CW EXECUTION CALLBACK.
 * @param:	None.
 * @return:	None.
 */
static void _AT_BUS_cw_callback(void) {
	// Local variables.
	PARSER_status_t parser_status = PARSER_ERROR_UNKNOWN_COMMAND;
	NODE_status_t node_status = NODE_SUCCESS;
	int32_t enable = 0;
	int32_t frequency_hz = 0;
	int32_t power_dbm = 0;
	uint8_t power_given = 0;
	// Read frequency parameter.
	parser_status = PARSER_get_parameter(&at_bus_ctx.parser, STRING_FORMAT_DECIMAL, AT_BUS_CHAR_SEPARATOR, &frequency_hz);
	PARSER_error_check_print();
	// First try with 3 parameters.
	parser_status = PARSER_get_parameter(&at_bus_ctx.parser, STRING_FORMAT_BOOLEAN, AT_BUS_CHAR_SEPARATOR, &enable);
	if (parser_status == PARSER_SUCCESS) {
		// There is a third parameter, try to parse power.
		parser_status = PARSER_get_parameter(&at_bus_ctx.parser, STRING_FORMAT_DECIMAL, STRING_CHAR_NULL, &power_dbm);
		PARSER_error_check_print();
		power_given = 1;
	}
	else {
		// Power is not given, try to parse enable as last parameter.
		parser_status = PARSER_get_parameter(&at_bus_ctx.parser, STRING_FORMAT_BOOLEAN, STRING_CHAR_NULL, &enable);
		PARSER_error_check_print();
	}
	// Configure CW mode.
	node_status = NODE_write_field(NODE_REQUEST_SOURCE_EXTERNAL, UHFM_REG_ADDR_RADIO_TEST_0, UHFM_REG_RADIO_TEST_0_MASK_RF_FREQUENCY, (uint32_t) frequency_hz);
	NODE_error_check_print();
	if (power_given != 0) {
		node_status = NODE_write_field(NODE_REQUEST_SOURCE_EXTERNAL, UHFM_REG_ADDR_RADIO_TEST_1, UHFM_REG_RADIO_TEST_1_MASK_TX_POWER, (uint32_t) DINFOX_convert_dbm(power_dbm));
		NODE_error_check_print();
	}
	// Start or stop mode.
	node_status = NODE_write_field(NODE_REQUEST_SOURCE_EXTERNAL, UHFM_REG_ADDR_STATUS_CONTROL_1, UHFM_REG_STATUS_CONTROL_1_MASK_CWEN, (uint32_t) enable);
	NODE_error_check_print();
	_AT_BUS_print_ok();
errors:
	return;
}
#endif

#if (defined UHFM) && (defined ATM)
/* AT$DL EXECUTION CALLBACK.
 * @param:	None.
 * @return:	None.
 */
static void _AT_BUS_dl_callback(void) {
	// Local variables.
	PARSER_status_t parser_status = PARSER_ERROR_UNKNOWN_COMMAND;
	NODE_status_t node_status = NODE_SUCCESS;
	UHFM_message_status_t message_status;
	uint8_t dl_phy_content[SIGFOX_DOWNLINK_PHY_SIZE_BYTES];
	int32_t frequency_hz = 0;
	uint32_t generic_u32 = 0;
	// Read frequency parameter.
	parser_status = PARSER_get_parameter(&at_bus_ctx.parser, STRING_FORMAT_DECIMAL, STRING_CHAR_NULL, &frequency_hz);
	PARSER_error_check_print();
	// Configure DL decoder.
	node_status = NODE_write_field(NODE_REQUEST_SOURCE_EXTERNAL, UHFM_REG_ADDR_RADIO_TEST_0, UHFM_REG_RADIO_TEST_0_MASK_RF_FREQUENCY, (uint32_t) frequency_hz);
	NODE_error_check_print();
	// Test loop.
	do {
		// Start DL decoder.
		node_status = NODE_write_field(NODE_REQUEST_SOURCE_EXTERNAL, UHFM_REG_ADDR_STATUS_CONTROL_1, UHFM_REG_STATUS_CONTROL_1_MASK_DTRG, (uint32_t) 0b1);
		NODE_error_check_print();
		// Read message status.
		node_status = NODE_read_field(NODE_REQUEST_SOURCE_EXTERNAL, UHFM_REG_ADDR_STATUS_CONTROL_1, UHFM_REG_STATUS_CONTROL_1_MASK_MESSAGE_STATUS, &generic_u32);
		NODE_error_check_print();
		message_status.all = (uint8_t) generic_u32;
		// Print DL frame if received.
		if (message_status.dl_frame != 0) {
			// Read frame.
			node_status = NODE_read_byte_array(NODE_REQUEST_SOURCE_EXTERNAL, UHFM_REG_ADDR_SIGFOX_DL_PHY_CONTENT_0, dl_phy_content, SIGFOX_DOWNLINK_PHY_SIZE_BYTES);
			NODE_error_check_print();
			// Read RSSI.
			node_status = NODE_read_field(NODE_REQUEST_SOURCE_EXTERNAL, UHFM_REG_ADDR_STATUS_CONTROL_1, UHFM_REG_STATUS_CONTROL_1_MASK_DL_RSSI, &generic_u32);
			NODE_error_check_print();
			// Print data.
			_AT_BUS_print_dl_phy_content(dl_phy_content, DINFOX_get_dbm(generic_u32));
		}
	}
	while (message_status.dl_frame != 0);
	_AT_BUS_print_ok();
errors:
	return;
}
#endif

#if (defined UHFM) && (defined ATM)
/* AT$RSSI EXECUTION CALLBACK.
 * @param:	None.
 * @return:	None.
 */
static void _AT_BUS_rssi_callback(void) {
	// Local variables.
	PARSER_status_t parser_status = PARSER_ERROR_UNKNOWN_COMMAND;
	NODE_status_t node_status = NODE_SUCCESS;
	LPTIM_status_t lptim1_status = LPTIM_SUCCESS;
	int32_t frequency_hz = 0;
	int32_t duration_seconds = 0;
	uint32_t generic_u32;
	uint32_t report_loop = 0;
	// Read frequency parameter.
	parser_status = PARSER_get_parameter(&at_bus_ctx.parser, STRING_FORMAT_DECIMAL, AT_BUS_CHAR_SEPARATOR, &frequency_hz);
	PARSER_error_check_print();
	// Read duration parameters.
	parser_status = PARSER_get_parameter(&at_bus_ctx.parser, STRING_FORMAT_DECIMAL, STRING_CHAR_NULL, &duration_seconds);
	PARSER_error_check_print();
	// Configure RSSI mode.
	node_status = NODE_write_field(NODE_REQUEST_SOURCE_EXTERNAL, UHFM_REG_ADDR_RADIO_TEST_0, UHFM_REG_RADIO_TEST_0_MASK_RF_FREQUENCY, (uint32_t) frequency_hz);
	NODE_error_check_print();
	// Start continuous listening.
	node_status = NODE_write_field(NODE_REQUEST_SOURCE_EXTERNAL, UHFM_REG_ADDR_STATUS_CONTROL_1, UHFM_REG_STATUS_CONTROL_1_MASK_RSEN, (uint32_t) 0b1);
	NODE_error_check_print();
	// Measurement loop.
	while (report_loop < ((duration_seconds * 1000) / AT_BUS_RSSI_REPORT_PERIOD_MS)) {
		// Read RSSI.
		node_status = NODE_read_field(NODE_REQUEST_SOURCE_EXTERNAL, UHFM_REG_ADDR_RADIO_TEST_1, UHFM_REG_RADIO_TEST_1_MASK_RSSI, &generic_u32);
		NODE_error_check_print();
		// Print RSSI.
		_AT_BUS_reply_add_string("+RSSI=");
		_AT_BUS_reply_add_value(DINFOX_get_dbm(generic_u32), STRING_FORMAT_DECIMAL, 0);
		_AT_BUS_reply_add_string("dBm");
		_AT_BUS_reply_send();
		// Report delay.
		lptim1_status = LPTIM1_delay_milliseconds(AT_BUS_RSSI_REPORT_PERIOD_MS, LPTIM_DELAY_MODE_ACTIVE);
		LPTIM1_error_check_print();
		report_loop++;
	}
	node_status = NODE_write_field(NODE_REQUEST_SOURCE_EXTERNAL, UHFM_REG_ADDR_STATUS_CONTROL_1, UHFM_REG_STATUS_CONTROL_1_MASK_RSEN, (uint32_t) 0b0);
	NODE_error_check_print();
	_AT_BUS_print_ok();
errors:
	// Stop RSSI mode.
	NODE_write_field(NODE_REQUEST_SOURCE_EXTERNAL, UHFM_REG_ADDR_STATUS_CONTROL_1, UHFM_REG_STATUS_CONTROL_1_MASK_RSEN, (uint32_t) 0b0);
	return;
}
#endif

#if (defined GPSM) && (defined ATM)
/* AT$TIME EXECUTION CALLBACK.
 * @param:	None.
 * @return:	None.
 */
static void _AT_BUS_time_callback(void) {
	// Local variables.
	PARSER_status_t parser_status = PARSER_ERROR_UNKNOWN_COMMAND;
	NODE_status_t node_status = NODE_SUCCESS;
	int32_t timeout_seconds = 0;
	uint32_t generic_u32 = 0;
	// Read timeout parameter.
	parser_status = PARSER_get_parameter(&at_bus_ctx.parser, STRING_FORMAT_DECIMAL, STRING_CHAR_NULL, &timeout_seconds);
	PARSER_error_check_print();
	// Configure timeout.
	node_status = NODE_write_field(NODE_REQUEST_SOURCE_EXTERNAL, GPSM_REG_ADDR_TIMEOUT, GPSM_REG_TIMEOUT_MASK_TIME_TIMEOUT, (uint32_t) timeout_seconds);
	NODE_error_check_print();
	// Start GPS acquisition.
	_AT_BUS_reply_add_string("GPS running...");
	_AT_BUS_reply_send();
	node_status = NODE_write_field(NODE_REQUEST_SOURCE_EXTERNAL, GPSM_REG_ADDR_STATUS_CONTROL_1, GPSM_REG_STATUS_CONTROL_1_MASK_TTRG, (uint32_t) 0b1);
	NODE_error_check_print();
	// Read status.
	node_status = NODE_read_field(NODE_REQUEST_SOURCE_EXTERNAL, GPSM_REG_ADDR_STATUS_CONTROL_1, GPSM_REG_STATUS_CONTROL_1_MASK_TFS, &generic_u32);
	NODE_error_check_print();
	// Check status.
	if (generic_u32 == 0) {
		_AT_BUS_reply_add_string("Timeout");
	}
	else {
		// Year.
		node_status = NODE_read_field(NODE_REQUEST_SOURCE_EXTERNAL, GPSM_REG_ADDR_TIME_DATA_0, GPSM_REG_TIME_DATA_0_MASK_YEAR, &generic_u32);
		NODE_error_check_print();
		_AT_BUS_reply_add_value((int32_t) (generic_u32 + 2000), STRING_FORMAT_DECIMAL, 0);
		_AT_BUS_reply_add_string("-");
		// Month.
		node_status = NODE_read_field(NODE_REQUEST_SOURCE_EXTERNAL, GPSM_REG_ADDR_TIME_DATA_0, GPSM_REG_TIME_DATA_0_MASK_MONTH, &generic_u32);
		NODE_error_check_print();
		if (generic_u32 < 10) {
			_AT_BUS_reply_add_value(0, STRING_FORMAT_DECIMAL, 0);
		}
		_AT_BUS_reply_add_value((int32_t) generic_u32, STRING_FORMAT_DECIMAL, 0);
		_AT_BUS_reply_add_string("-");
		// Date.
		node_status = NODE_read_field(NODE_REQUEST_SOURCE_EXTERNAL, GPSM_REG_ADDR_TIME_DATA_0, GPSM_REG_TIME_DATA_0_MASK_DATE, &generic_u32);
		NODE_error_check_print();
		if (generic_u32 < 10) {
			_AT_BUS_reply_add_value(0, STRING_FORMAT_DECIMAL, 0);
		}
		_AT_BUS_reply_add_value((int32_t) generic_u32, STRING_FORMAT_DECIMAL, 0);
		_AT_BUS_reply_add_string(" ");
		// Hours.
		node_status = NODE_read_field(NODE_REQUEST_SOURCE_EXTERNAL, GPSM_REG_ADDR_TIME_DATA_1, GPSM_REG_TIME_DATA_1_MASK_HOUR, &generic_u32);
		NODE_error_check_print();
		if (generic_u32 < 10) {
			_AT_BUS_reply_add_value(0, STRING_FORMAT_DECIMAL, 0);
		}
		_AT_BUS_reply_add_value((int32_t) generic_u32, STRING_FORMAT_DECIMAL, 0);
		_AT_BUS_reply_add_string(":");
		// Minutes.
		node_status = NODE_read_field(NODE_REQUEST_SOURCE_EXTERNAL, GPSM_REG_ADDR_TIME_DATA_1, GPSM_REG_TIME_DATA_1_MASK_MINUTE, &generic_u32);
		NODE_error_check_print();
		if (generic_u32 < 10) {
			_AT_BUS_reply_add_value(0, STRING_FORMAT_DECIMAL, 0);
		}
		_AT_BUS_reply_add_value((int32_t) generic_u32, STRING_FORMAT_DECIMAL, 0);
		_AT_BUS_reply_add_string(":");
		// Seconds.
		node_status = NODE_read_field(NODE_REQUEST_SOURCE_EXTERNAL, GPSM_REG_ADDR_TIME_DATA_1, GPSM_REG_TIME_DATA_1_MASK_SECOND, &generic_u32);
		NODE_error_check_print();
		if (generic_u32 < 10) {
			_AT_BUS_reply_add_value(0, STRING_FORMAT_DECIMAL, 0);
		}
		_AT_BUS_reply_add_value((int32_t) generic_u32, STRING_FORMAT_DECIMAL, 0);
		// Fix duration.
		node_status = NODE_read_field(NODE_REQUEST_SOURCE_EXTERNAL, GPSM_REG_ADDR_TIME_DATA_2, GPSM_REG_TIME_DATA_2_MASK_FIX_DURATION, &generic_u32);
		NODE_error_check_print();
		_AT_BUS_reply_add_string(" (fix ");
		_AT_BUS_reply_add_value((int32_t) generic_u32, STRING_FORMAT_DECIMAL, 0);
		_AT_BUS_reply_add_string("s)");
	}
	_AT_BUS_reply_send();
	_AT_BUS_print_ok();
errors:
	return;
}
#endif

#if (defined GPSM) && (defined ATM)
/* AT$GPS EXECUTION CALLBACK.
 * @param:	None.
 * @return:	None.
 */
static void _AT_BUS_gps_callback(void) {
	// Local variables.
	PARSER_status_t parser_status = PARSER_ERROR_UNKNOWN_COMMAND;
	NODE_status_t node_status = NODE_SUCCESS;
	int32_t timeout_seconds = 0;
	uint32_t generic_u32 = 0;
	// Read timeout parameter.
	parser_status = PARSER_get_parameter(&at_bus_ctx.parser, STRING_FORMAT_DECIMAL, STRING_CHAR_NULL, &timeout_seconds);
	PARSER_error_check_print();
	// Configure timeout.
	node_status = NODE_write_field(NODE_REQUEST_SOURCE_EXTERNAL, GPSM_REG_ADDR_TIMEOUT, GPSM_REG_TIMEOUT_MASK_GEOLOC_TIMEOUT, (uint32_t) timeout_seconds);
	NODE_error_check_print();
	// Start GPS acquisition.
	_AT_BUS_reply_add_string("GPS running...");
	_AT_BUS_reply_send();
	node_status = NODE_write_field(NODE_REQUEST_SOURCE_EXTERNAL, GPSM_REG_ADDR_STATUS_CONTROL_1, GPSM_REG_STATUS_CONTROL_1_MASK_GTRG, (uint32_t) 0b1);
	NODE_error_check_print();
	// Read status.
	node_status = NODE_read_field(NODE_REQUEST_SOURCE_EXTERNAL, GPSM_REG_ADDR_STATUS_CONTROL_1, GPSM_REG_STATUS_CONTROL_1_MASK_GFS, &generic_u32);
	NODE_error_check_print();
	// Check status.
	if (generic_u32 == 0) {
		_AT_BUS_reply_add_string("Timeout");
	}
	else {
		// Latitude degrees.
		_AT_BUS_reply_add_string("Lat=");
		node_status = NODE_read_field(NODE_REQUEST_SOURCE_EXTERNAL, GPSM_REG_ADDR_GEOLOC_DATA_0, GPSM_REG_GEOLOC_DATA_0_MASK_DEGREE, &generic_u32);
		NODE_error_check_print();
		_AT_BUS_reply_add_value((int32_t) generic_u32, STRING_FORMAT_DECIMAL, 0);
		_AT_BUS_reply_add_string("d");
		// Latitude minutes.
		node_status = NODE_read_field(NODE_REQUEST_SOURCE_EXTERNAL, GPSM_REG_ADDR_GEOLOC_DATA_0, GPSM_REG_GEOLOC_DATA_0_MASK_MINUTE, &generic_u32);
		NODE_error_check_print();
		_AT_BUS_reply_add_value((int32_t) generic_u32, STRING_FORMAT_DECIMAL, 0);
		_AT_BUS_reply_add_string("'");
		// Latitude seconds.
		node_status = NODE_read_field(NODE_REQUEST_SOURCE_EXTERNAL, GPSM_REG_ADDR_GEOLOC_DATA_0, GPSM_REG_GEOLOC_DATA_0_MASK_SECOND, &generic_u32);
		NODE_error_check_print();
		_AT_BUS_reply_add_value((int32_t) generic_u32, STRING_FORMAT_DECIMAL, 0);
		_AT_BUS_reply_add_string("''");
		// Latitude north flag.
		node_status = NODE_read_field(NODE_REQUEST_SOURCE_EXTERNAL, GPSM_REG_ADDR_GEOLOC_DATA_0, GPSM_REG_GEOLOC_DATA_0_MASK_NF, &generic_u32);
		NODE_error_check_print();
		_AT_BUS_reply_add_string((generic_u32 == 0) ? "S" : "N");
		// Longitude degrees.
		_AT_BUS_reply_add_string(" Long=");
		node_status = NODE_read_field(NODE_REQUEST_SOURCE_EXTERNAL, GPSM_REG_ADDR_GEOLOC_DATA_1, GPSM_REG_GEOLOC_DATA_1_MASK_DEGREE, &generic_u32);
		NODE_error_check_print();
		_AT_BUS_reply_add_value((int32_t) generic_u32, STRING_FORMAT_DECIMAL, 0);
		_AT_BUS_reply_add_string("d");
		// Longitude minutes.
		node_status = NODE_read_field(NODE_REQUEST_SOURCE_EXTERNAL, GPSM_REG_ADDR_GEOLOC_DATA_1, GPSM_REG_GEOLOC_DATA_1_MASK_MINUTE, &generic_u32);
		NODE_error_check_print();
		_AT_BUS_reply_add_value((int32_t) generic_u32, STRING_FORMAT_DECIMAL, 0);
		_AT_BUS_reply_add_string("'");
		// Longitude seconds.
		node_status = NODE_read_field(NODE_REQUEST_SOURCE_EXTERNAL, GPSM_REG_ADDR_GEOLOC_DATA_1, GPSM_REG_GEOLOC_DATA_1_MASK_SECOND, &generic_u32);
		NODE_error_check_print();
		_AT_BUS_reply_add_value((int32_t) generic_u32, STRING_FORMAT_DECIMAL, 0);
		_AT_BUS_reply_add_string("''");
		// Longitude east flag.
		node_status = NODE_read_field(NODE_REQUEST_SOURCE_EXTERNAL, GPSM_REG_ADDR_GEOLOC_DATA_1, GPSM_REG_GEOLOC_DATA_1_MASK_EF, &generic_u32);
		NODE_error_check_print();
		_AT_BUS_reply_add_string((generic_u32 == 0) ? "W" : "E");
		// Altitude.
		node_status = NODE_read_field(NODE_REQUEST_SOURCE_EXTERNAL, GPSM_REG_ADDR_GEOLOC_DATA_2, GPSM_REG_GEOLOC_DATA_2_MASK_ALTITUDE, &generic_u32);
		NODE_error_check_print();
		_AT_BUS_reply_add_string(" Alt=");
		_AT_BUS_reply_add_value((int32_t) generic_u32, STRING_FORMAT_DECIMAL, 0);
		_AT_BUS_reply_add_string("m");
		// Fix duration.
		node_status = NODE_read_field(NODE_REQUEST_SOURCE_EXTERNAL, GPSM_REG_ADDR_GEOLOC_DATA_3, GPSM_REG_GEOLOC_DATA_3_MASK_FIX_DURATION, &generic_u32);
		NODE_error_check_print();
		_AT_BUS_reply_add_string(" (fix ");
		_AT_BUS_reply_add_value((int32_t) generic_u32, STRING_FORMAT_DECIMAL, 0);
		_AT_BUS_reply_add_string("s)");
	}
	_AT_BUS_reply_send();
	_AT_BUS_print_ok();
errors:
	return;
}
#endif

#if (defined GPSM) && (defined ATM)
/* AT$PULSE EXECUTION CALLBACK.
 * @param:	None.
 * @return:	None.
 */
static void _AT_BUS_pulse_callback(void) {
	// Local variables.
	PARSER_status_t parser_status = PARSER_ERROR_UNKNOWN_COMMAND;
	NODE_status_t node_status = NODE_SUCCESS;
	int32_t active = 0;
	int32_t frequency_hz = 0;
	int32_t duty_cycle_percent;
	// Read parameters.
	parser_status = PARSER_get_parameter(&at_bus_ctx.parser, STRING_FORMAT_BOOLEAN, AT_BUS_CHAR_SEPARATOR, &active);
	PARSER_error_check_print();
	parser_status = PARSER_get_parameter(&at_bus_ctx.parser, STRING_FORMAT_DECIMAL, AT_BUS_CHAR_SEPARATOR, &frequency_hz);
	PARSER_error_check_print();
	parser_status = PARSER_get_parameter(&at_bus_ctx.parser, STRING_FORMAT_DECIMAL, STRING_CHAR_NULL, &duty_cycle_percent);
	PARSER_error_check_print();
	// Configure timepulse signal.
	node_status = NODE_write_field(NODE_REQUEST_SOURCE_EXTERNAL, GPSM_REG_ADDR_TIMEPULSE_CONFIGURATION_0, GPSM_REG_TIMEPULSE_CONFIGURATION_0_MASK_FREQUENCY, (uint32_t) frequency_hz);
	NODE_error_check_print();
	node_status = NODE_write_field(NODE_REQUEST_SOURCE_EXTERNAL, GPSM_REG_ADDR_TIMEPULSE_CONFIGURATION_1, GPSM_REG_TIMEPULSE_CONFIGURATION_1_MASK_DUTY_CUCLE, (uint32_t) duty_cycle_percent);
	NODE_error_check_print();
	// Start or stop timepulse signal.
	node_status = NODE_write_field(NODE_REQUEST_SOURCE_EXTERNAL, GPSM_REG_ADDR_STATUS_CONTROL_1, GPSM_REG_STATUS_CONTROL_1_MASK_TPEN, (uint32_t) active);
	NODE_error_check_print();
	_AT_BUS_print_ok();
errors:
	return;
}
#endif

/* RESET AT PARSER.
 * @param:	None.
 * @return:	None.
 */
static void _AT_BUS_reset_parser(void) {
	// Flush buffers.
	at_bus_ctx.command_size = 0;
	at_bus_ctx.reply_size = 0;
	// Reset flag.
	at_bus_ctx.line_end_flag = 0;
	// Reset parser.
	at_bus_ctx.parser.buffer = (char_t*) at_bus_ctx.command;
	at_bus_ctx.parser.buffer_size = 0;
	at_bus_ctx.parser.separator_idx = 0;
	at_bus_ctx.parser.start_idx = 0;
}

/* PARSE THE CURRENT AT COMMAND BUFFER.
 * @param:	None.
 * @return:	None.
 */
static void _AT_BUS_decode(void) {
	// Local variables.
	uint8_t idx = 0;
	uint8_t decode_success = 0;
	// Update parser length.
	at_bus_ctx.parser.buffer_size = at_bus_ctx.command_size;
	// Loop on available commands.
	for (idx=0 ; idx<(sizeof(AT_BUS_COMMAND_LIST) / sizeof(AT_BUS_command_t)) ; idx++) {
		// Check type.
		if (PARSER_compare(&at_bus_ctx.parser, AT_BUS_COMMAND_LIST[idx].mode, AT_BUS_COMMAND_LIST[idx].syntax) == PARSER_SUCCESS) {
			// Execute callback and exit.
			AT_BUS_COMMAND_LIST[idx].callback();
			decode_success = 1;
			break;
		}
	}
	if (decode_success == 0) {
		_AT_BUS_print_error(ERROR_BASE_PARSER + PARSER_ERROR_UNKNOWN_COMMAND); // Unknown command.
		goto errors;
	}
errors:
	_AT_BUS_reset_parser();
	return;
}

/*** AT functions ***/

/* INIT AT MANAGER.
 * @param self_address:	Self bus address.
 * @return:				None.
 */
void AT_BUS_init(NODE_address_t self_address) {
	// Local variables.
	LBUS_status_t lbus_status = LBUS_SUCCESS;
	// Init context.
	_AT_BUS_reset_parser();
	// Init LBUS layer.
	lbus_status = LBUS_init(self_address);
	LBUS_error_check();
	// Enable LPUART.
	LPUART1_enable_rx();
}

/* MAIN TASK OF AT COMMAND MANAGER.
 * @param:	None.
 * @return:	None.
 */
void AT_BUS_task(void) {
	// Trigger decoding function if line end found.
	if (at_bus_ctx.line_end_flag != 0) {
		// Decode and execute command.
		LPUART1_disable_rx();
		_AT_BUS_decode();
		LPUART1_enable_rx();
	}
}

/* FILL AT COMMAND BUFFER WITH A NEW BYTE (CALLED BY LPUART INTERRUPT).
 * @param rx_byte:	Incoming byte.
 * @return:			None.
 */
void AT_BUS_fill_rx_buffer(uint8_t rx_byte) {
	// Append byte if line end flag is not allready set.
	if (at_bus_ctx.line_end_flag == 0) {
		// Check ending characters.
		if (rx_byte == AT_BUS_FRAME_END) {
			at_bus_ctx.command[at_bus_ctx.command_size] = STRING_CHAR_NULL;
			at_bus_ctx.line_end_flag = 1;
		}
		else {
			// Store new byte.
			at_bus_ctx.command[at_bus_ctx.command_size] = rx_byte;
			// Manage index.
			at_bus_ctx.command_size = (at_bus_ctx.command_size + 1) % AT_BUS_COMMAND_BUFFER_SIZE;
		}
	}
}

/* PRINT SIGFOX LIBRARY RESULT.
 * @param test_result:	Test result.
 * @param rssi:			Downlink signal rssi in dBm.
 */
void AT_BUS_print_test_result(uint8_t test_result, int16_t rssi_dbm) {
	// Check result.
	if (test_result == 0) {
		_AT_BUS_reply_add_string("Test failed.");
	}
	else {
		_AT_BUS_reply_add_string("Test passed. RSSI=");
		_AT_BUS_reply_add_value(rssi_dbm, STRING_FORMAT_DECIMAL, 0);
		_AT_BUS_reply_add_string("dBm");
	}
	_AT_BUS_reply_send();
}

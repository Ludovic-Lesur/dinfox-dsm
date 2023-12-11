/*
 * at.c
 *
 *  Created on: 18 apr. 2020
 *      Author: Ludo
 */

#include "at_bus.h"

#include "bpsm_reg.h"
#include "ddrm_reg.h"
#include "common_reg.h"
#include "dinfox.h"
#include "error.h"
#include "gpsm_reg.h"
#include "lbus.h"
#include "lvrm_reg.h"
#include "mode.h"
#include "node.h"
#include "node_common.h"
#include "parser.h"
#include "pwr.h"
#include "rrm_reg.h"
#include "sm_reg.h"
#include "string.h"
#include "types.h"
#include "uhfm_reg.h"
#ifdef UHFM
#include "sigfox_ep_api.h"
#include "sigfox_types.h"
#endif

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
#if (defined ATM) && (defined UHFM)
#define AT_BUS_COMMAND_NVM
#define AT_BUS_COMMAND_SIGFOX_EP_LIB
#define AT_BUS_COMMAND_SIGFOX_ADDON_RFP
#define AT_BUS_COMMAND_CW
#define AT_BUS_COMMAND_DL
#define AT_BUS_COMMAND_RSSI
#define AT_BUS_RSSI_REPORT_PERIOD_MS		500
#endif

/*** AT callbacks declaration ***/

/*******************************************************************/
static void _AT_BUS_read_callback(void);
static void _AT_BUS_write_callback(void);

#ifdef ATM
/*******************************************************************/
static void _AT_BUS_print_ok(void);
static void _AT_BUS_print_command_list(void);
static void _AT_BUS_print_sw_version(void);
static void _AT_BUS_print_error_stack(void);
static void _AT_BUS_adc_callback(void);
#endif

#if (defined ATM) && (defined AT_BUS_COMMAND_NVM)
/*******************************************************************/
static void _AT_BUS_nvm_read_callback(void);
static void _AT_BUS_nvm_write_callback(void);
#endif

#if (defined ATM) && (defined UHFM) && (defined AT_BUS_COMMAND_NVM)
/*******************************************************************/
static void _AT_BUS_get_id_callback(void);
static void _AT_BUS_set_id_callback(void);
static void _AT_BUS_get_key_callback(void);
static void _AT_BUS_set_key_callback(void);
#endif

#if (defined ATM) && (defined UHFM) && (defined AT_BUS_COMMAND_SIGFOX_EP_LIB)
/*******************************************************************/
static void _AT_BUS_print_dl_payload(void);
static void _AT_BUS_so_callback(void);
static void _AT_BUS_sb_callback(void);
static void _AT_BUS_sf_callback(void);
#endif

#if (defined ATM) && (defined UHFM) && (defined AT_BUS_COMMAND_SIGFOX_ADDON_RFP)
/*******************************************************************/
static void _AT_BUS_tm_callback(void);
#endif

#if (defined ATM) && (defined UHFM) && (defined  AT_BUS_COMMAND_CW)
/*******************************************************************/
static void _AT_BUS_cw_callback(void);
#endif

#if (defined ATM) && (defined UHFM) && (defined  AT_BUS_COMMAND_RSSI)
/*******************************************************************/
static void _AT_BUS_rssi_callback(void);
#endif

#if (defined ATM) && (defined GPSM)
/*******************************************************************/
static void _AT_BUS_time_callback(void);
static void _AT_BUS_gps_callback(void);
static void _AT_BUS_pulse_callback(void);
#endif

/*** AT local structures ***/

/*******************************************************************/
typedef struct {
	PARSER_mode_t mode;
	char_t* syntax;
	char_t* parameters;
	char_t* description;
	void (*callback)(void);
} AT_BUS_command_t;

/*******************************************************************/
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
	{PARSER_MODE_HEADER,  "AT$R=", "reg_addr[hex]", "Read register", _AT_BUS_read_callback},
	{PARSER_MODE_HEADER,  "AT$W=", "reg_addr[hex],reg_value[hex],(reg_mask[hex])", "Write register",_AT_BUS_write_callback},
#ifdef ATM
	{PARSER_MODE_COMMAND, "AT", STRING_NULL, "Ping command", _AT_BUS_print_ok},
	{PARSER_MODE_COMMAND, "AT?", STRING_NULL, "AT commands list", _AT_BUS_print_command_list},
	{PARSER_MODE_COMMAND, "AT$V?", STRING_NULL, "Get SW version", _AT_BUS_print_sw_version},
	{PARSER_MODE_COMMAND, "AT$ERROR?", STRING_NULL, "Read error stack", _AT_BUS_print_error_stack},
	{PARSER_MODE_COMMAND, "AT$RST", STRING_NULL, "Reset MCU", PWR_software_reset},
	{PARSER_MODE_COMMAND, "AT$ADC?", STRING_NULL, "Get ADC data", _AT_BUS_adc_callback},
#endif
#if (defined ATM) && (defined AT_BUS_COMMAND_NVM)
	{PARSER_MODE_HEADER,  "AT$NVMR=", "address[dec]", "Read NVM byte", _AT_BUS_nvm_read_callback},
	{PARSER_MODE_HEADER, "AT$NVMW=", "address[hex],value[hex]", "Write NVM byte", _AT_BUS_nvm_write_callback},
#endif
#if (defined ATM) && (defined UHFM) && (defined AT_BUS_COMMAND_NVM)
	{PARSER_MODE_COMMAND, "AT$ID?", STRING_NULL, "Get Sigfox EP ID", _AT_BUS_get_id_callback},
	{PARSER_MODE_HEADER,  "AT$ID=", "id[hex]", "Set Sigfox EP ID", _AT_BUS_set_id_callback},
	{PARSER_MODE_COMMAND, "AT$KEY?", STRING_NULL, "Get Sigfox EP key", _AT_BUS_get_key_callback},
	{PARSER_MODE_HEADER,  "AT$KEY=", "key[hex]", "Set Sigfox EP key", _AT_BUS_set_key_callback},
#endif
#if (defined ATM) && (defined UHFM) && (defined AT_BUS_COMMAND_SIGFOX_EP_LIB)
	{PARSER_MODE_COMMAND, "AT$SO", STRING_NULL, "Sigfox send control message", _AT_BUS_so_callback},
	{PARSER_MODE_HEADER,  "AT$SB=", "data[bit],(bidir_flag[bit])", "Sigfox send bit", _AT_BUS_sb_callback},
	{PARSER_MODE_HEADER,  "AT$SF=", "data[hex],(bidir_flag[bit])", "Sigfox send frame", _AT_BUS_sf_callback},
	{PARSER_MODE_COMMAND, "AT$DL?", STRING_NULL, "Read last DL payload", _AT_BUS_print_dl_payload},
#endif
#if (defined ATM) && (defined UHFM) && (defined AT_BUS_COMMAND_SIGFOX_ADDON_RFP)
	{PARSER_MODE_HEADER,  "AT$TM=", "rc_index[dec],test_mode[dec]", "Sigfox RFP test mode", _AT_BUS_tm_callback},
#endif
#if (defined ATM) && (defined UHFM) && (defined  AT_BUS_COMMAND_CW)
	{PARSER_MODE_HEADER,  "AT$CW=", "frequency[hz],enable[bit],(output_power[dbm])", "Continuous wave", _AT_BUS_cw_callback},
#endif
#if (defined ATM) && (defined UHFM) && (defined  AT_BUS_COMMAND_RSSI)
	{PARSER_MODE_HEADER,  "AT$RSSI=", "frequency[hz],duration[s]", "Continuous RSSI measurement", _AT_BUS_rssi_callback},
#endif
#if (defined ATM) && (defined GPSM)
	{PARSER_MODE_HEADER,  "AT$TIME=", "timeout[s]", "Get GPS time", _AT_BUS_time_callback},
	{PARSER_MODE_HEADER,  "AT$GPS=", "timeout[s]", "Get GPS position", _AT_BUS_gps_callback},
	{PARSER_MODE_HEADER,  "AT$PULSE=", "enable[bit],frequency[hz],duty_cycle[percent]", "Start or stop GPS timepulse signal", _AT_BUS_pulse_callback},
#endif
};
static AT_BUS_context_t at_bus_ctx;

/*** AT local functions ***/

/*******************************************************************/
#define _AT_BUS_reply_add_char(character) { \
	at_bus_ctx.reply[at_bus_ctx.reply_size] = character; \
	at_bus_ctx.reply_size = (at_bus_ctx.reply_size + 1) % AT_BUS_REPLY_BUFFER_SIZE; \
}

/*******************************************************************/
static void _AT_BUS_fill_rx_buffer(uint8_t rx_byte) {
	// Append byte if line end flag is not already set.
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

/*******************************************************************/
static void _AT_BUS_reply_add_string(char_t* tx_string) {
	// Fill reply buffer with new bytes.
	while (*tx_string) {
		_AT_BUS_reply_add_char(*(tx_string++));
		// Detect rollover.
		if (at_bus_ctx.reply_size == 0) break;
	}
}

/*******************************************************************/
static void _AT_BUS_reply_add_value(int32_t tx_value, STRING_format_t format, uint8_t print_prefix) {
	// Local variables.
	STRING_status_t string_status = STRING_SUCCESS;
	char_t str_value[AT_BUS_STRING_VALUE_BUFFER_SIZE];
	uint8_t idx = 0;
	// Reset string.
	for (idx=0 ; idx<AT_BUS_STRING_VALUE_BUFFER_SIZE ; idx++) str_value[idx] = STRING_CHAR_NULL;
	// Convert value to string.
	string_status = STRING_value_to_string(tx_value, format, print_prefix, str_value);
	STRING_stack_error();
	// Add string.
	_AT_BUS_reply_add_string(str_value);
}

/*******************************************************************/
static void _AT_BUS_reply_add_register(uint32_t reg_value) {
	// Local variables.
	STRING_status_t string_status = STRING_SUCCESS;
	char_t str_value[AT_BUS_STRING_VALUE_BUFFER_SIZE] = {STRING_CHAR_NULL};
	// Convert register to string.
	string_status = DINFOX_register_to_string(reg_value, str_value);
	STRING_stack_error();
	// Add string.
	_AT_BUS_reply_add_string(str_value);
}

/*******************************************************************/
static void _AT_BUS_reply_send(void) {
	// Local variables.
	LBUS_status_t lbus_status = LBUS_SUCCESS;
	// Add ending character.
	_AT_BUS_reply_add_char(AT_BUS_FRAME_END);
	// Send reply.
	lbus_status = LBUS_send((uint8_t*) at_bus_ctx.reply, at_bus_ctx.reply_size);
	LBUS_stack_error();
	// Flush response buffer.
	at_bus_ctx.reply_size = 0;
}

/*******************************************************************/
static void _AT_BUS_print_ok(void) {
	_AT_BUS_reply_add_string("OK");
	_AT_BUS_reply_send();
}

/*******************************************************************/
static void _AT_BUS_print_error(ERROR_code_t error) {
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

/*******************************************************************/
static void _AT_BUS_read_callback(void) {
	// Local variables.
	ERROR_code_t status = SUCCESS;
	PARSER_status_t parser_status = PARSER_SUCCESS;
	NODE_status_t node_status = NODE_SUCCESS;
	uint32_t reg_addr = 0;
	uint32_t reg_value = 0;
	// Read address parameter.
	parser_status = DINFOX_parse_register(&at_bus_ctx.parser, STRING_CHAR_NULL, &reg_addr);
	PARSER_stack_exit_error(ERROR_BASE_PARSER + parser_status);
	// Read register.
	node_status = NODE_read_register(NODE_REQUEST_SOURCE_EXTERNAL, reg_addr, &reg_value);
	NODE_stack_exit_error(ERROR_BASE_NODE + node_status);
	// Send reply.
	_AT_BUS_reply_add_register(reg_value);
	_AT_BUS_reply_send();
	return;
errors:
	_AT_BUS_print_error(status);
	return;
}

/*******************************************************************/
static void _AT_BUS_write_callback(void) {
	// Local variables.
	ERROR_code_t status = SUCCESS;
	PARSER_status_t parser_status = PARSER_SUCCESS;
	NODE_status_t node_status = NODE_SUCCESS;
	uint32_t reg_addr = 0;
	uint32_t reg_value = 0;
	uint32_t reg_mask = 0;
	// Read address parameter.
	parser_status = DINFOX_parse_register(&at_bus_ctx.parser, AT_BUS_CHAR_SEPARATOR, &reg_addr);
	PARSER_stack_exit_error(ERROR_BASE_PARSER + parser_status);
	// First try with 3 parameters.
	parser_status = DINFOX_parse_register(&at_bus_ctx.parser, AT_BUS_CHAR_SEPARATOR, &reg_value);
	if (parser_status == PARSER_SUCCESS) {
		// Try parsing register mask parameter.
		parser_status = DINFOX_parse_register(&at_bus_ctx.parser, STRING_CHAR_NULL, &reg_mask);
		PARSER_stack_exit_error(ERROR_BASE_PARSER + parser_status);
	}
	else {
		// Try with only 2 parameters.
		parser_status = DINFOX_parse_register(&at_bus_ctx.parser, STRING_CHAR_NULL, &reg_value);
		PARSER_stack_exit_error(ERROR_BASE_PARSER + parser_status);
		// Perform full write operation since mask is not given.
		reg_mask = DINFOX_REG_MASK_ALL;
	}
	// Write register.
	node_status = NODE_write_register(NODE_REQUEST_SOURCE_EXTERNAL, reg_addr, reg_mask, reg_value);
	NODE_stack_exit_error(ERROR_BASE_NODE + node_status);
	// Operation completed.
	_AT_BUS_print_ok();
	return;
errors:
	_AT_BUS_print_error(status);
	return;
}

#ifdef ATM
/*******************************************************************/
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
/*******************************************************************/
static void _AT_BUS_print_sw_version(void) {
	// Local variables.
	uint32_t reg_value = 0;
	_AT_BUS_reply_add_string("SW");
	// Read software version register 0.
	NODE_read_register(NODE_REQUEST_SOURCE_INTERNAL, COMMON_REG_ADDR_SW_VERSION_0, &reg_value);
	// Major version.
	_AT_BUS_reply_add_value((int32_t) DINFOX_read_field(reg_value, COMMON_REG_SW_VERSION_0_MASK_MAJOR), STRING_FORMAT_DECIMAL, 0);
	// Minor version.
	_AT_BUS_reply_add_string(".");
	_AT_BUS_reply_add_value((int32_t) DINFOX_read_field(reg_value, COMMON_REG_SW_VERSION_0_MASK_MINOR), STRING_FORMAT_DECIMAL, 0);
	// Commit index.
	_AT_BUS_reply_add_string(".");
	_AT_BUS_reply_add_value((int32_t) DINFOX_read_field(reg_value, COMMON_REG_SW_VERSION_0_MASK_COMMIT_INDEX), STRING_FORMAT_DECIMAL, 0);
	// Dirty flag.
	if (DINFOX_read_field(reg_value, COMMON_REG_SW_VERSION_0_MASK_DTYF) != 0) {
		_AT_BUS_reply_add_string(".d");
	}
	// Commit ID.
	NODE_read_register(NODE_REQUEST_SOURCE_INTERNAL, COMMON_REG_ADDR_SW_VERSION_1, &reg_value);
	_AT_BUS_reply_add_string(" (");
	_AT_BUS_reply_add_value((int32_t) DINFOX_read_field(reg_value, COMMON_REG_SW_VERSION_1_MASK_COMMIT_ID), STRING_FORMAT_HEXADECIMAL, 1);
	_AT_BUS_reply_add_string(")");
	_AT_BUS_reply_send();
	_AT_BUS_print_ok();
}
#endif

#ifdef ATM
/*******************************************************************/
static void _AT_BUS_print_error_stack(void) {
	// Local variables.
	uint32_t reg_value = 0;
	ERROR_code_t error = SUCCESS;
#ifdef UHFM
	// Import Sigfox errors into MCU stack.
	ERROR_import_sigfox_stack();
#endif
	// Unstack all errors.
	_AT_BUS_reply_add_string("[ ");
	do {
		// Read error stack.
		NODE_read_register(NODE_REQUEST_SOURCE_INTERNAL, COMMON_REG_ADDR_ERROR_STACK, &reg_value);
		error = (ERROR_code_t) DINFOX_read_field(reg_value, COMMON_REG_ERROR_STACK_MASK_ERROR);
		// Check value.
		if (error != SUCCESS) {
			_AT_BUS_reply_add_value((int32_t) error, STRING_FORMAT_HEXADECIMAL, 1);
			_AT_BUS_reply_add_string(" ");
		}
	}
	while (error != SUCCESS);
	_AT_BUS_reply_add_string("]");
	_AT_BUS_reply_send();
	_AT_BUS_print_ok();
}
#endif

#ifdef ATM
/*******************************************************************/
static void _AT_BUS_adc_callback(void) {
	// Local variables.
	ERROR_code_t status = SUCCESS;
	NODE_status_t node_status = NODE_SUCCESS;
	uint32_t reg_control_0 = 0;
	uint32_t reg_control_0_mask = 0;
	uint32_t reg_analog_data_0 = 0;
	uint32_t reg_analog_data_1 = 0;
#if !(defined UHFM) && !(defined GPSM)
	uint32_t reg_analog_data_2 = 0;
#endif
	int32_t value = 0;
	// Trigger measurements.
	DINFOX_write_field(&reg_control_0, &reg_control_0_mask, 0b1, COMMON_REG_CONTROL_0_MASK_MTRG);
	node_status = NODE_write_register(NODE_REQUEST_SOURCE_EXTERNAL, COMMON_REG_ADDR_CONTROL_0, reg_control_0_mask, reg_control_0);
	NODE_stack_exit_error(ERROR_BASE_NODE + node_status);
	// Read data.
	NODE_read_register(NODE_REQUEST_SOURCE_INTERNAL, COMMON_REG_ADDR_ANALOG_DATA_0, &reg_analog_data_0);
	// MCU voltage.
	_AT_BUS_reply_add_string("Vmcu=");
	_AT_BUS_reply_add_value((int32_t) DINFOX_get_mv((DINFOX_voltage_representation_t) DINFOX_read_field(reg_analog_data_0, COMMON_REG_ANALOG_DATA_0_MASK_VMCU)), STRING_FORMAT_DECIMAL, 0);
	_AT_BUS_reply_add_string("mV");
	_AT_BUS_reply_send();
	// MCU temperature.
	_AT_BUS_reply_add_string("Tmcu=");
	_AT_BUS_reply_add_value((int32_t) DINFOX_get_degrees((DINFOX_temperature_representation_t) DINFOX_read_field(reg_analog_data_0, COMMON_REG_ANALOG_DATA_0_MASK_TMCU)), STRING_FORMAT_DECIMAL, 0);
	_AT_BUS_reply_add_string("dC");
	_AT_BUS_reply_send();
	// Input voltage.
#ifdef LVRM
	NODE_read_register(NODE_REQUEST_SOURCE_INTERNAL, LVRM_REG_ADDR_ANALOG_DATA_1, &reg_analog_data_1);
	value = (int32_t) DINFOX_get_mv((DINFOX_voltage_representation_t) DINFOX_read_field(reg_analog_data_1, LVRM_REG_ANALOG_DATA_1_MASK_VCOM));
	_AT_BUS_reply_add_string("Vcom=");
#endif
#ifdef BPSM
	NODE_read_register(NODE_REQUEST_SOURCE_INTERNAL, BPSM_REG_ADDR_ANALOG_DATA_1, &reg_analog_data_1);
	value = (int32_t) DINFOX_get_mv((DINFOX_voltage_representation_t) DINFOX_read_field(reg_analog_data_1, BPSM_REG_ANALOG_DATA_1_MASK_VSRC));
	_AT_BUS_reply_add_string("Vsrc=");
#endif
#ifdef DDRM
	NODE_read_register(NODE_REQUEST_SOURCE_INTERNAL, DDRM_REG_ADDR_ANALOG_DATA_1, &reg_analog_data_1);
	value = (int32_t) DINFOX_get_mv((DINFOX_voltage_representation_t) DINFOX_read_field(reg_analog_data_1, DDRM_REG_ANALOG_DATA_1_MASK_VIN));
	NODE_stack_exit_error(ERROR_BASE_NODE + node_status);
	_AT_BUS_reply_add_string("Vin=");
#endif
#ifdef RRM
	NODE_read_register(NODE_REQUEST_SOURCE_INTERNAL, RRM_REG_ADDR_ANALOG_DATA_1, &reg_analog_data_1);
	value = (int32_t) DINFOX_get_mv((DINFOX_voltage_representation_t) DINFOX_read_field(reg_analog_data_1, RRM_REG_ANALOG_DATA_1_MASK_VIN));
	_AT_BUS_reply_add_string("Vin=");
#endif
#if (defined LVRM) || (defined BPSM) || (defined DDRM) || (defined RRM)
	_AT_BUS_reply_add_value(value, STRING_FORMAT_DECIMAL, 0);
	_AT_BUS_reply_add_string("mV");
	_AT_BUS_reply_send();
#endif
	// Storage element voltage.
#ifdef BPSM
	value = (int32_t) DINFOX_get_mv((DINFOX_voltage_representation_t) DINFOX_read_field(reg_analog_data_1, BPSM_REG_ANALOG_DATA_1_MASK_VSTR));
	_AT_BUS_reply_add_string("Vstr=");
	_AT_BUS_reply_add_value(value, STRING_FORMAT_DECIMAL, 0);
	_AT_BUS_reply_add_string("mV");
	_AT_BUS_reply_send();
#endif
	// Output voltage.
#ifdef LVRM
	value = (int32_t) DINFOX_get_mv((DINFOX_voltage_representation_t) DINFOX_read_field(reg_analog_data_1, LVRM_REG_ANALOG_DATA_1_MASK_VOUT));
	_AT_BUS_reply_add_string("Vout=");
#endif
#ifdef DDRM
	value = (int32_t) DINFOX_get_mv((DINFOX_voltage_representation_t) DINFOX_read_field(reg_analog_data_1, DDRM_REG_ANALOG_DATA_1_MASK_VOUT));
	_AT_BUS_reply_add_string("Vout=");
#endif
#ifdef RRM
	value = (int32_t) DINFOX_get_mv((DINFOX_voltage_representation_t) DINFOX_read_field(reg_analog_data_1, RRM_REG_ANALOG_DATA_1_MASK_VOUT));
	_AT_BUS_reply_add_string("Vout=");
#endif
#ifdef BPSM
	NODE_read_register(NODE_REQUEST_SOURCE_INTERNAL, BPSM_REG_ADDR_ANALOG_DATA_2, &reg_analog_data_2);
	value = (int32_t) DINFOX_get_mv((DINFOX_voltage_representation_t) DINFOX_read_field(reg_analog_data_2, BPSM_REG_ANALOG_DATA_2_MASK_VBKP));
	_AT_BUS_reply_add_string("Vbkp=");
#endif
#if (defined LVRM) || (defined BPSM) || (defined DDRM) || (defined RRM)
	_AT_BUS_reply_add_value(value, STRING_FORMAT_DECIMAL, 0);
	_AT_BUS_reply_add_string("mV");
	_AT_BUS_reply_send();
#endif
	// Output current.
#ifdef LVRM
	NODE_read_register(NODE_REQUEST_SOURCE_INTERNAL, LVRM_REG_ADDR_ANALOG_DATA_2, &reg_analog_data_2);
	value = (int32_t) DINFOX_get_mv((DINFOX_voltage_representation_t) DINFOX_read_field(reg_analog_data_2, LVRM_REG_ANALOG_DATA_2_MASK_IOUT));
	_AT_BUS_reply_add_string("Iout=");
#endif
#ifdef DDRM
	NODE_read_register(NODE_REQUEST_SOURCE_INTERNAL, DDRM_REG_ADDR_ANALOG_DATA_2, &reg_analog_data_2);
	value = (int32_t) DINFOX_get_mv((DINFOX_voltage_representation_t) DINFOX_read_field(reg_analog_data_2, DDRM_REG_ANALOG_DATA_2_MASK_IOUT));
	_AT_BUS_reply_add_string("Iout=");
#endif
#ifdef RRM
	NODE_read_register(NODE_REQUEST_SOURCE_INTERNAL, RRM_REG_ADDR_ANALOG_DATA_2, &reg_analog_data_2);
	value = (int32_t) DINFOX_get_mv((DINFOX_voltage_representation_t) DINFOX_read_field(reg_analog_data_2, RRM_REG_ANALOG_DATA_2_MASK_IOUT));
	_AT_BUS_reply_add_string("Iout=");
#endif
#if (defined LVRM) || (defined DDRM) || (defined RRM)
	_AT_BUS_reply_add_value(value, STRING_FORMAT_DECIMAL, 0);
	_AT_BUS_reply_add_string("uA");
	_AT_BUS_reply_send();
#endif
	// AINx voltage.
#if (defined SM) && (defined SM_AIN_ENABLE)
	// Read data.
	NODE_read_register(NODE_REQUEST_SOURCE_INTERNAL, SM_REG_ADDR_ANALOG_DATA_1, &reg_analog_data_1);
	// AIN0 voltage.
	value = (int32_t) DINFOX_get_mv((DINFOX_voltage_representation_t) DINFOX_read_field(reg_analog_data_1, SM_REG_ANALOG_DATA_1_MASK_VAIN0));
	_AT_BUS_reply_add_string("AIN0=");
	_AT_BUS_reply_add_value(value, STRING_FORMAT_DECIMAL, 0);
	_AT_BUS_reply_add_string("mV");	NODE_stack_exit_error(ERROR_BASE_NODE + node_status);
	_AT_BUS_reply_send();
	// AIN1 voltage.
	value = (int32_t) DINFOX_get_mv((DINFOX_voltage_representation_t) DINFOX_read_field(reg_analog_data_1, SM_REG_ANALOG_DATA_1_MASK_VAIN1));
	_AT_BUS_reply_add_string("AIN1=");
	_AT_BUS_reply_add_value(value, STRING_FORMAT_DECIMAL, 0);
	_AT_BUS_reply_add_string("mV");
	_AT_BUS_reply_send();
	// Read data.
	NODE_read_register(NODE_REQUEST_SOURCE_INTERNAL, SM_REG_ADDR_ANALOG_DATA_2, &reg_analog_data_2);
	// AIN2 voltage.
	value = (int32_t) DINFOX_get_mv((DINFOX_voltage_representation_t) DINFOX_read_field(reg_analog_data_2, SM_REG_ANALOG_DATA_2_MASK_VAIN2));
	_AT_BUS_reply_add_string("AIN2=");
	_AT_BUS_reply_add_value(value, STRING_FORMAT_DECIMAL, 0);
	_AT_BUS_reply_add_string("mV");
	_AT_BUS_reply_send();
	// AIN3 voltage.
	value = (int32_t) DINFOX_get_mv((DINFOX_voltage_representation_t) DINFOX_read_field(reg_analog_data_2, SM_REG_ANALOG_DATA_2_MASK_VAIN3));
	_AT_BUS_reply_add_string("AIN3=");
	_AT_BUS_reply_add_value(value, STRING_FORMAT_DECIMAL, 0);
	_AT_BUS_reply_add_string("mV");
	_AT_BUS_reply_send();
#endif
	// Radio voltages.
#ifdef UHFM
	// Read data
	NODE_read_register(NODE_REQUEST_SOURCE_INTERNAL, UHFM_REG_ADDR_ANALOG_DATA_1, &reg_analog_data_1);
	// TX voltage.
	value = (int32_t) DINFOX_get_mv((DINFOX_voltage_representation_t) DINFOX_read_field(reg_analog_data_1, UHFM_REG_ANALOG_DATA_1_MASK_VRF_TX));
	_AT_BUS_reply_add_string("Vrf_tx=");
	_AT_BUS_reply_add_value(value, STRING_FORMAT_DECIMAL, 0);
	_AT_BUS_reply_add_string("mV");
	_AT_BUS_reply_send();
	// RX voltage.
	value = (int32_t) DINFOX_get_mv((DINFOX_voltage_representation_t) DINFOX_read_field(reg_analog_data_1, UHFM_REG_ANALOG_DATA_1_MASK_VRF_RX));
	_AT_BUS_reply_add_string("Vrf_rx=");
	_AT_BUS_reply_add_value(value, STRING_FORMAT_DECIMAL, 0);
	_AT_BUS_reply_add_string("mV");
	_AT_BUS_reply_send();
#endif
#ifdef GPSM
	// Read data
	NODE_read_register(NODE_REQUEST_SOURCE_INTERNAL, GPSM_REG_ADDR_ANALOG_DATA_1, &reg_analog_data_1);
	// GPS voltage.
	value = (int32_t) DINFOX_get_mv((DINFOX_voltage_representation_t) DINFOX_read_field(reg_analog_data_1, GPSM_REG_ANALOG_DATA_1_MASK_VGPS));
	_AT_BUS_reply_add_string("Vgps=");
	_AT_BUS_reply_add_value(value, STRING_FORMAT_DECIMAL, 0);
	_AT_BUS_reply_add_string("mV");
	_AT_BUS_reply_send();
#ifdef GPSM_ACTIVE_ANTENNA
	// Active antenna voltage.
	value = (int32_t) DINFOX_get_mv((DINFOX_voltage_representation_t) DINFOX_read_field(reg_analog_data_1, GPSM_REG_ANALOG_DATA_1_MASK_VANT));
	_AT_BUS_reply_add_string("Vant=");
	_AT_BUS_reply_add_value(value, STRING_FORMAT_DECIMAL, 0);
	_AT_BUS_reply_add_string("mV");
	_AT_BUS_reply_send();
#endif
#endif
	_AT_BUS_print_ok();
	return;
errors:
	_AT_BUS_print_error(status);
	return;
}
#endif

#if (defined ATM) && (defined AT_BUS_COMMAND_NVM)
//*******************************************************************/
static void _AT_BUS_nvm_read_callback(void) {
	// Local variables.
	ERROR_code_t status = SUCCESS;
	PARSER_status_t parser_status = PARSER_ERROR_UNKNOWN_COMMAND;
	NVM_status_t nvm_status = NVM_SUCCESS;
	int32_t address = 0;
	uint8_t nvm_data = 0;
	// Read address parameters.
	parser_status = PARSER_get_parameter(&at_bus_ctx.parser, STRING_FORMAT_HEXADECIMAL, STRING_CHAR_NULL, &address);
	PARSER_stack_exit_error(ERROR_BASE_PARSER + parser_status);
	// Read byte at requested address.
	nvm_status = NVM_read_byte((NVM_address_t) address, &nvm_data);
	NVM_stack_exit_error(ERROR_BASE_NVM + nvm_status);
	// Print data.
	_AT_BUS_reply_add_value(nvm_data, STRING_FORMAT_HEXADECIMAL, 1);
	_AT_BUS_reply_send();
	_AT_BUS_print_ok();
	return;
errors:
	_AT_BUS_print_error(status);
	return;
}
#endif

#if (defined ATM) && (defined AT_BUS_COMMAND_NVM)
/*******************************************************************/
static void _AT_BUS_nvm_write_callback(void) {
	// Local variables.
	ERROR_code_t status = SUCCESS;
	PARSER_status_t parser_status = PARSER_ERROR_UNKNOWN_COMMAND;
	NVM_status_t nvm_status = NVM_SUCCESS;
	int32_t address = 0;
	int32_t value = 0;
	// Read address parameters.
	parser_status = PARSER_get_parameter(&at_bus_ctx.parser, STRING_FORMAT_HEXADECIMAL, AT_BUS_CHAR_SEPARATOR, &address);
	PARSER_stack_exit_error(ERROR_BASE_PARSER + parser_status);
	// Read value.
	parser_status = PARSER_get_parameter(&at_bus_ctx.parser, STRING_FORMAT_HEXADECIMAL, STRING_CHAR_NULL, &value);
	PARSER_stack_exit_error(ERROR_BASE_PARSER + parser_status);
	// Read byte at requested address.
	nvm_status = NVM_write_byte((NVM_address_t) address, (uint8_t) value);
	NVM_stack_exit_error(ERROR_BASE_NVM + nvm_status);
	_AT_BUS_print_ok();
	return;
errors:
	_AT_BUS_print_error(status);
	return;
}
#endif

#if (defined ATM) && (defined UHFM) && (defined AT_BUS_COMMAND_NVM)
/*******************************************************************/
static void _AT_BUS_get_id_callback(void) {
	// Local variables.
	ERROR_code_t status = SUCCESS;
	NODE_status_t node_status = NODE_SUCCESS;
	uint8_t sigfox_ep_id[SIGFOX_EP_ID_SIZE_BYTES];
	uint8_t idx = 0;
	// Read ID.
	node_status = NODE_read_byte_array(NODE_REQUEST_SOURCE_INTERNAL, UHFM_REG_ADDR_SIGFOX_EP_ID, sigfox_ep_id, SIGFOX_EP_ID_SIZE_BYTES);
	NODE_stack_exit_error(ERROR_BASE_NODE + node_status);
	// Print ID.
	for (idx=0 ; idx<SIGFOX_EP_ID_SIZE_BYTES ; idx++) {
		_AT_BUS_reply_add_value(sigfox_ep_id[idx], STRING_FORMAT_HEXADECIMAL, (idx==0 ? 1 : 0));
	}
	_AT_BUS_reply_send();
	_AT_BUS_print_ok();
	return;
errors:
	_AT_BUS_print_error(status);
	return;
}
#endif

#if (defined ATM) && (defined UHFM) && (defined AT_BUS_COMMAND_NVM)
/*******************************************************************/
static void _AT_BUS_set_id_callback(void) {
	// Local variables.
	ERROR_code_t status = SUCCESS;
	PARSER_status_t parser_status = PARSER_ERROR_UNKNOWN_COMMAND;
	NODE_status_t node_status = NODE_SUCCESS;
	NVM_status_t nvm_status = NVM_SUCCESS;
	uint8_t sigfox_ep_id[SIGFOX_EP_ID_SIZE_BYTES];
	uint8_t extracted_length = 0;
	uint8_t idx = 0;
	// Read ID parameter.
	parser_status = PARSER_get_byte_array(&at_bus_ctx.parser, STRING_CHAR_NULL, SIGFOX_EP_ID_SIZE_BYTES, 1, sigfox_ep_id, &extracted_length);
	PARSER_stack_exit_error(ERROR_BASE_PARSER + parser_status);
	// Write device ID in NVM.
	for (idx=0 ; idx<SIGFOX_EP_ID_SIZE_BYTES ; idx++) {
		nvm_status = NVM_write_byte((NVM_ADDRESS_SIGFOX_EP_ID + idx), sigfox_ep_id[idx]);
		NVM_stack_exit_error(ERROR_BASE_NVM + nvm_status);
	}
	// Update register.
	node_status = NODE_write_byte_array(NODE_REQUEST_SOURCE_INTERNAL, UHFM_REG_ADDR_SIGFOX_EP_ID, (uint8_t*) sigfox_ep_id, SIGFOX_EP_ID_SIZE_BYTES);
	NODE_stack_exit_error(ERROR_BASE_NODE + node_status);
	_AT_BUS_print_ok();
	return;
errors:
	_AT_BUS_print_error(status);
	return;
}
#endif

#if (defined ATM) && (defined UHFM) && (defined AT_BUS_COMMAND_NVM)
/*******************************************************************/
static void _AT_BUS_get_key_callback(void) {
	// Local variables.
	ERROR_code_t status = SUCCESS;
	NODE_status_t node_status = NODE_SUCCESS;
	uint8_t sigfox_ep_key[SIGFOX_EP_KEY_SIZE_BYTES];
	uint8_t idx = 0;
	// Read key.
	node_status = NODE_read_byte_array(NODE_REQUEST_SOURCE_INTERNAL, UHFM_REG_ADDR_SIGFOX_EP_KEY_0, sigfox_ep_key, SIGFOX_EP_KEY_SIZE_BYTES);
	NODE_stack_exit_error(ERROR_BASE_NODE + node_status);
	// Print key.
	for (idx=0 ; idx<SIGFOX_EP_KEY_SIZE_BYTES ; idx++) {
		_AT_BUS_reply_add_value(sigfox_ep_key[idx], STRING_FORMAT_HEXADECIMAL, (idx==0 ? 1 : 0));
	}
	_AT_BUS_reply_send();
	_AT_BUS_print_ok();
	return;
errors:
	_AT_BUS_print_error(status);
	return;
}
#endif

#if (defined ATM) && (defined UHFM) && (defined AT_BUS_COMMAND_NVM)
/*******************************************************************/
static void _AT_BUS_set_key_callback(void) {
	// Local variables.
	ERROR_code_t status = SUCCESS;
	PARSER_status_t parser_status = PARSER_ERROR_UNKNOWN_COMMAND;
	NODE_status_t node_status = NODE_SUCCESS;
	NVM_status_t nvm_status = NVM_SUCCESS;
	uint8_t sigfox_ep_key[SIGFOX_EP_KEY_SIZE_BYTES];
	uint8_t extracted_length = 0;
	uint8_t idx = 0;
	// Read key parameter.
	parser_status = PARSER_get_byte_array(&at_bus_ctx.parser, STRING_CHAR_NULL, SIGFOX_EP_KEY_SIZE_BYTES, 1, sigfox_ep_key, &extracted_length);
	PARSER_stack_exit_error(ERROR_BASE_PARSER + parser_status);
	// Write device ID in NVM.
	for (idx=0 ; idx<SIGFOX_EP_KEY_SIZE_BYTES ; idx++) {
		nvm_status = NVM_write_byte((NVM_ADDRESS_SIGFOX_EP_KEY + idx), sigfox_ep_key[idx]);
		NVM_stack_exit_error(ERROR_BASE_NVM + nvm_status);
	}
	// Update registers.
	node_status = NODE_write_byte_array(NODE_REQUEST_SOURCE_INTERNAL, UHFM_REG_ADDR_SIGFOX_EP_KEY_0, (uint8_t*) sigfox_ep_key, SIGFOX_EP_KEY_SIZE_BYTES);
	NODE_stack_exit_error(ERROR_BASE_NODE + node_status);
	_AT_BUS_print_ok();
	return;
errors:
	_AT_BUS_print_error(status);
	return;
}
#endif

#if (defined ATM) && (defined UHFM) && (defined AT_BUS_COMMAND_SIGFOX_EP_LIB)
/*******************************************************************/
static void _AT_BUS_print_dl_payload(void) {
	// Local variables.
	uint8_t dl_payload[SIGFOX_DL_PAYLOAD_SIZE_BYTES];
	uint8_t idx = 0;
	// Read registers.
	NODE_read_byte_array(NODE_REQUEST_SOURCE_INTERNAL, UHFM_REG_ADDR_SIGFOX_DL_PAYLOAD_0, dl_payload, SIGFOX_DL_PAYLOAD_SIZE_BYTES);
	// Print DL payload.
	_AT_BUS_reply_add_string("+RX=");
	for (idx=0 ; idx<SIGFOX_DL_PAYLOAD_SIZE_BYTES ; idx++) {
		_AT_BUS_reply_add_value(dl_payload[idx], STRING_FORMAT_HEXADECIMAL, 0);
	}
	_AT_BUS_reply_send();
	return;
}
#endif

#if (defined ATM) && (defined UHFM) && (defined AT_BUS_COMMAND_SIGFOX_EP_LIB)
/*******************************************************************/
static void _AT_BUS_send_sigfox_message(uint8_t bidir_flag) {
	// Local variables.
	ERROR_code_t status = SUCCESS;
	NODE_status_t node_status = NODE_SUCCESS;
	SIGFOX_EP_API_message_status_t message_status;
	uint32_t reg_control_1 = 0;
	uint32_t reg_control_1_mask = 0;
	uint32_t reg_status = 0;
	// Send Sigfox message.
	DINFOX_write_field(&reg_control_1, &reg_control_1_mask, 0b1, UHFM_REG_CONTROL_1_MASK_STRG);
	node_status = NODE_write_register(NODE_REQUEST_SOURCE_EXTERNAL, UHFM_REG_ADDR_CONTROL_1, reg_control_1_mask, reg_control_1);
	NODE_stack_exit_error(ERROR_BASE_NODE + node_status);
	// Read message status.
	NODE_read_register(NODE_REQUEST_SOURCE_INTERNAL, UHFM_REG_ADDR_STATUS_1, &reg_status);
	message_status.all = (uint8_t) DINFOX_read_field(reg_status, UHFM_REG_STATUS_1_MASK_MESSAGE_STATUS);
	// Print message status.
	_AT_BUS_reply_add_string("+MSG_STAT=");
	_AT_BUS_reply_add_value(message_status.all, STRING_FORMAT_HEXADECIMAL, 1);
	_AT_BUS_reply_send();
	// Print DL payload if required.
	if ((bidir_flag != 0) && (message_status.field.dl_frame != 0)) {
		_AT_BUS_print_dl_payload();
	}
	_AT_BUS_print_ok();
	return;
errors:
	_AT_BUS_print_error(status);
	return;
}
#endif

#if (defined ATM) && (defined UHFM) && (defined AT_BUS_COMMAND_SIGFOX_EP_LIB)
/*******************************************************************/
static void _AT_BUS_so_callback(void) {
	// Local variables.
	uint32_t reg_config_2 = 0;
	uint32_t reg_config_2_mask = 0;
	// Configure message.
	DINFOX_write_field(&reg_config_2, &reg_config_2_mask, 0b01, UHFM_REG_CONFIGURATION_2_MASK_PRT);
	DINFOX_write_field(&reg_config_2, &reg_config_2_mask, 0b1, UHFM_REG_CONFIGURATION_2_MASK_CMSG);
	DINFOX_write_field(&reg_config_2, &reg_config_2_mask, (uint32_t) SIGFOX_CONTROL_MESSAGE_TYPE_KEEP_ALIVE, UHFM_REG_CONFIGURATION_2_MASK_MSGT);
	// Write register.
	NODE_write_register(NODE_REQUEST_SOURCE_INTERNAL, UHFM_REG_ADDR_CONFIGURATION_2, reg_config_2_mask, reg_config_2);
	// Send Sigfox message.
	_AT_BUS_send_sigfox_message(0);
	return;
}
#endif

#if (defined ATM) && (defined UHFM) && (defined AT_BUS_COMMAND_SIGFOX_EP_LIB)
/*******************************************************************/
static void _AT_BUS_sb_callback(void) {
	// Local variables.
	ERROR_code_t status = SUCCESS;
	PARSER_status_t parser_status = PARSER_ERROR_UNKNOWN_COMMAND;
	int32_t ul_bit = 0;
	int32_t bidir_flag = 0;
	uint32_t reg_config_2 = 0;
	uint32_t reg_config_2_mask = 0;
	// First try with 2 parameters.
	parser_status = PARSER_get_parameter(&at_bus_ctx.parser, STRING_FORMAT_BOOLEAN, AT_BUS_CHAR_SEPARATOR, &ul_bit);
	if (parser_status == PARSER_SUCCESS) {
		// Try parsing downlink request parameter.
		parser_status =  PARSER_get_parameter(&at_bus_ctx.parser, STRING_FORMAT_BOOLEAN, STRING_CHAR_NULL, &bidir_flag);
		PARSER_stack_exit_error(ERROR_BASE_PARSER + parser_status);
	}
	else {
		// Try with 1 parameter.
		parser_status = PARSER_get_parameter(&at_bus_ctx.parser, STRING_FORMAT_BOOLEAN, STRING_CHAR_NULL, &ul_bit);
		PARSER_stack_exit_error(ERROR_BASE_PARSER + parser_status);
	}
	// Configure message.
	DINFOX_write_field(&reg_config_2, &reg_config_2_mask, 0b01, UHFM_REG_CONFIGURATION_2_MASK_PRT);
	DINFOX_write_field(&reg_config_2, &reg_config_2_mask, 0b0, UHFM_REG_CONFIGURATION_2_MASK_CMSG);
	DINFOX_write_field(&reg_config_2, &reg_config_2_mask, (uint32_t) (SIGFOX_APPLICATION_MESSAGE_TYPE_BIT0 + ul_bit), UHFM_REG_CONFIGURATION_2_MASK_MSGT);
	DINFOX_write_field(&reg_config_2, &reg_config_2_mask, (uint32_t) bidir_flag, UHFM_REG_CONFIGURATION_2_MASK_BF);
	// Write register.
	NODE_write_register(NODE_REQUEST_SOURCE_INTERNAL, UHFM_REG_ADDR_CONFIGURATION_2, reg_config_2_mask, reg_config_2);
	// Send Sigfox message.
	_AT_BUS_send_sigfox_message(bidir_flag);
	return;
errors:
	_AT_BUS_print_error(status);
	return;
}
#endif

#if (defined ATM) && (defined UHFM) && (defined AT_BUS_COMMAND_SIGFOX_EP_LIB)
/*******************************************************************/
static void _AT_BUS_sf_callback(void) {
	// Local variables.
	ERROR_code_t status = SUCCESS;
	PARSER_status_t parser_status = PARSER_ERROR_UNKNOWN_COMMAND;
	uint8_t ul_payload[SIGFOX_UL_PAYLOAD_MAX_SIZE_BYTES];
	uint8_t ul_payload_size = 0;
	int32_t bidir_flag = 0;
	uint32_t reg_config_2 = 0;
	uint32_t reg_config_2_mask = 0;
	// First try with 2 parameters.
	parser_status = PARSER_get_byte_array(&at_bus_ctx.parser, AT_BUS_CHAR_SEPARATOR, SIGFOX_UL_PAYLOAD_MAX_SIZE_BYTES, 0, ul_payload, &ul_payload_size);
	if (parser_status == PARSER_SUCCESS) {
		// Try parsing downlink request parameter.
		parser_status =  PARSER_get_parameter(&at_bus_ctx.parser, STRING_FORMAT_BOOLEAN, STRING_CHAR_NULL, &bidir_flag);
		PARSER_stack_exit_error(ERROR_BASE_PARSER + parser_status);
	}
	else {
		// Try with 1 parameter.
		parser_status = PARSER_get_byte_array(&at_bus_ctx.parser, STRING_CHAR_NULL, SIGFOX_UL_PAYLOAD_MAX_SIZE_BYTES, 0, ul_payload, &ul_payload_size);
		PARSER_stack_exit_error(ERROR_BASE_PARSER + parser_status);
	}
	// Configure message.
	DINFOX_write_field(&reg_config_2, &reg_config_2_mask, 0b01, UHFM_REG_CONFIGURATION_2_MASK_PRT);
	DINFOX_write_field(&reg_config_2, &reg_config_2_mask, 0b0, UHFM_REG_CONFIGURATION_2_MASK_CMSG);
	DINFOX_write_field(&reg_config_2, &reg_config_2_mask, (uint32_t) (SIGFOX_APPLICATION_MESSAGE_TYPE_BYTE_ARRAY), UHFM_REG_CONFIGURATION_2_MASK_MSGT);
	DINFOX_write_field(&reg_config_2, &reg_config_2_mask, (uint32_t) bidir_flag, UHFM_REG_CONFIGURATION_2_MASK_BF);
	DINFOX_write_field(&reg_config_2, &reg_config_2_mask, (uint32_t) ul_payload_size, UHFM_REG_CONFIGURATION_2_MASK_UL_PAYLOAD_SIZE);
	// Write register.
	NODE_write_register(NODE_REQUEST_SOURCE_INTERNAL, UHFM_REG_ADDR_CONFIGURATION_2, reg_config_2_mask, reg_config_2);
	// Write uplink payload.
	NODE_write_byte_array(NODE_REQUEST_SOURCE_INTERNAL, UHFM_REG_ADDR_SIGFOX_UL_PAYLOAD_0, (uint8_t*) ul_payload, ul_payload_size);
	// Send Sigfox message.
	_AT_BUS_send_sigfox_message(bidir_flag);
	return;
errors:
	_AT_BUS_print_error(status);
	return;
}
#endif

#if (defined ATM) && (defined UHFM) && (defined AT_BUS_COMMAND_SIGFOX_ADDON_RFP)
/*******************************************************************/
static void _AT_BUS_tm_callback(void) {
	// Local variables.
	ERROR_code_t status = SUCCESS;
	PARSER_status_t parser_status = PARSER_ERROR_UNKNOWN_COMMAND;
	NODE_status_t node_status = NODE_SUCCESS;
	int32_t rc_index = 0;
	int32_t test_mode = 0;
	uint32_t reg_config_0 = 0;
	uint32_t reg_config_0_mask = 0;
	uint32_t reg_control_1 = 0;
	uint32_t reg_control_1_mask = 0;
	// Read RC parameter.
	parser_status = PARSER_get_parameter(&at_bus_ctx.parser, STRING_FORMAT_DECIMAL, AT_BUS_CHAR_SEPARATOR, &rc_index);
	PARSER_stack_exit_error(ERROR_BASE_PARSER + parser_status);
	// Read test mode parameter.
	parser_status =  PARSER_get_parameter(&at_bus_ctx.parser, STRING_FORMAT_DECIMAL, STRING_CHAR_NULL, &test_mode);
	PARSER_stack_exit_error(ERROR_BASE_PARSER + parser_status);
	// Configure test mode.
	DINFOX_write_field(&reg_config_0, &reg_config_0_mask, (uint32_t) rc_index, UHFM_REG_CONFIGURATION_0_MASK_RC);
	DINFOX_write_field(&reg_config_0, &reg_config_0_mask, (uint32_t) test_mode, UHFM_REG_CONFIGURATION_0_MASK_TEST_MODE);
	DINFOX_write_field(&reg_control_1, &reg_control_1_mask, 0b1, UHFM_REG_CONTROL_1_MASK_TTRG);
	// Write registers.
	NODE_write_register(NODE_REQUEST_SOURCE_INTERNAL, UHFM_REG_ADDR_CONFIGURATION_0, reg_config_0_mask, reg_config_0);
	// Trigger test mode.
	node_status = NODE_write_register(NODE_REQUEST_SOURCE_EXTERNAL, UHFM_REG_ADDR_CONTROL_1, reg_control_1_mask, reg_control_1);
	NODE_stack_exit_error(ERROR_BASE_NODE + node_status);
	_AT_BUS_print_ok();
	return;
errors:
	_AT_BUS_print_error(status);
	return;
}
#endif

#if (defined ATM) && (defined UHFM) && (defined  AT_BUS_COMMAND_CW)
/*******************************************************************/
static void _AT_BUS_cw_callback(void) {
	// Local variables.
	ERROR_code_t status = SUCCESS;
	PARSER_status_t parser_status = PARSER_ERROR_UNKNOWN_COMMAND;
	NODE_status_t node_status = NODE_SUCCESS;
	int32_t enable = 0;
	int32_t frequency_hz = 0;
	int32_t power_dbm = 0;
	uint8_t power_given = 0;
	uint32_t reg_radio_test_0 = 0;
	uint32_t reg_radio_test_0_mask = 0;
	uint32_t reg_radio_test_1 = 0;
	uint32_t reg_radio_test_1_mask = 0;
	uint32_t reg_control_1 = 0;
	uint32_t reg_control_1_mask = 0;
	// Read frequency parameter.
	parser_status = PARSER_get_parameter(&at_bus_ctx.parser, STRING_FORMAT_DECIMAL, AT_BUS_CHAR_SEPARATOR, &frequency_hz);
	PARSER_stack_exit_error(ERROR_BASE_PARSER + parser_status);
	// First try with 3 parameters.
	parser_status = PARSER_get_parameter(&at_bus_ctx.parser, STRING_FORMAT_BOOLEAN, AT_BUS_CHAR_SEPARATOR, &enable);
	if (parser_status == PARSER_SUCCESS) {
		// There is a third parameter, try to parse power.
		parser_status = PARSER_get_parameter(&at_bus_ctx.parser, STRING_FORMAT_DECIMAL, STRING_CHAR_NULL, &power_dbm);
		PARSER_stack_exit_error(ERROR_BASE_PARSER + parser_status);
		power_given = 1;
	}
	else {
		// Power is not given, try to parse enable as last parameter.
		parser_status = PARSER_get_parameter(&at_bus_ctx.parser, STRING_FORMAT_BOOLEAN, STRING_CHAR_NULL, &enable);
		PARSER_stack_exit_error(ERROR_BASE_PARSER + parser_status);
	}
	// Configure CW mode.
	DINFOX_write_field(&reg_radio_test_0, &reg_radio_test_0_mask, (uint32_t) frequency_hz, UHFM_REG_RADIO_TEST_0_MASK_RF_FREQUENCY);
	DINFOX_write_field(&reg_radio_test_1, &reg_radio_test_1_mask, (uint32_t) DINFOX_convert_dbm(power_dbm), UHFM_REG_RADIO_TEST_1_MASK_TX_POWER);
	DINFOX_write_field(&reg_control_1, &reg_control_1_mask, (uint32_t) enable, UHFM_REG_CONTROL_1_MASK_CWEN);
	// Write configuration.
	NODE_write_register(NODE_REQUEST_SOURCE_INTERNAL, UHFM_REG_ADDR_RADIO_TEST_0, reg_radio_test_0_mask, reg_radio_test_0);
	if (power_given != 0) {
		NODE_write_register(NODE_REQUEST_SOURCE_INTERNAL, UHFM_REG_ADDR_RADIO_TEST_1, reg_radio_test_1_mask, reg_radio_test_1);
	}
	// Control CW mode.
	node_status = NODE_write_register(NODE_REQUEST_SOURCE_EXTERNAL, UHFM_REG_ADDR_CONTROL_1, reg_control_1_mask, reg_control_1);
	NODE_stack_exit_error(ERROR_BASE_NODE + node_status);
	_AT_BUS_print_ok();
	return;
errors:
	_AT_BUS_print_error(status);
	return;
}
#endif

#if (defined ATM) && (defined UHFM) && (defined  AT_BUS_COMMAND_RSSI)
/*******************************************************************/
static void _AT_BUS_rssi_callback(void) {
	// Local variables.
	ERROR_code_t status = SUCCESS;
	PARSER_status_t parser_status = PARSER_ERROR_UNKNOWN_COMMAND;
	NODE_status_t node_status = NODE_SUCCESS;
	LPTIM_status_t lptim1_status = LPTIM_SUCCESS;
	int32_t frequency_hz = 0;
	int32_t duration_seconds = 0;
	int16_t rssi_dbm = 0;
	uint32_t reg_radio_test_0 = 0;
	uint32_t reg_radio_test_0_mask = 0;
	uint32_t reg_radio_test_1 = 0;
	uint32_t reg_control_1 = 0;
	uint32_t reg_control_1_mask = 0;
	uint32_t report_loop = 0;
	// Read frequency parameter.
	parser_status = PARSER_get_parameter(&at_bus_ctx.parser, STRING_FORMAT_DECIMAL, AT_BUS_CHAR_SEPARATOR, &frequency_hz);
	PARSER_stack_exit_error(ERROR_BASE_PARSER + parser_status);
	// Read duration parameters.
	parser_status = PARSER_get_parameter(&at_bus_ctx.parser, STRING_FORMAT_DECIMAL, STRING_CHAR_NULL, &duration_seconds);
	PARSER_stack_exit_error(ERROR_BASE_PARSER + parser_status);
	// Configure RSSI mode.
	DINFOX_write_field(&reg_radio_test_0, &reg_radio_test_0_mask, (uint32_t) frequency_hz, UHFM_REG_RADIO_TEST_0_MASK_RF_FREQUENCY);
	DINFOX_write_field(&reg_control_1, &reg_control_1_mask, 0b1, UHFM_REG_CONTROL_1_MASK_RSEN);
	// Write configuration.
	NODE_write_register(NODE_REQUEST_SOURCE_INTERNAL, UHFM_REG_ADDR_RADIO_TEST_0, reg_radio_test_0_mask, reg_radio_test_0);
	// Start RSSI measurement.
	node_status = NODE_write_register(NODE_REQUEST_SOURCE_EXTERNAL, UHFM_REG_ADDR_CONTROL_1, reg_control_1_mask, reg_control_1);
	NODE_stack_exit_error(ERROR_BASE_NODE + node_status);
	// Measurement loop.
	while (report_loop < ((duration_seconds * 1000) / AT_BUS_RSSI_REPORT_PERIOD_MS)) {
		// Read RSSI.
		node_status = NODE_read_register(NODE_REQUEST_SOURCE_INTERNAL, UHFM_REG_ADDR_RADIO_TEST_1, &reg_radio_test_1);
		NODE_stack_exit_error(ERROR_BASE_NODE + node_status);
		rssi_dbm = DINFOX_get_dbm((DINFOX_rf_power_representation_t) DINFOX_read_field(reg_radio_test_1, UHFM_REG_RADIO_TEST_1_MASK_RSSI));
		// Print RSSI.
		_AT_BUS_reply_add_string("+RSSI=");
		_AT_BUS_reply_add_value((int32_t) rssi_dbm, STRING_FORMAT_DECIMAL, 0);
		_AT_BUS_reply_add_string("dBm");
		_AT_BUS_reply_send();
		// Report delay.
		lptim1_status = LPTIM1_delay_milliseconds(AT_BUS_RSSI_REPORT_PERIOD_MS, LPTIM_DELAY_MODE_ACTIVE);
		LPTIM1_stack_exit_error(ERROR_BASE_LPTIM1 + lptim1_status);
		report_loop++;
	}
	// Stop RSSI mode.
	reg_control_1 = 0;
	reg_control_1_mask = 0;
	DINFOX_write_field(&reg_control_1, &reg_control_1_mask, 0b0, UHFM_REG_CONTROL_1_MASK_RSEN);
	node_status = NODE_write_register(NODE_REQUEST_SOURCE_EXTERNAL, UHFM_REG_ADDR_CONTROL_1, reg_control_1_mask, reg_control_1);
	NODE_stack_exit_error(ERROR_BASE_NODE + node_status);
	_AT_BUS_print_ok();
	return;
errors:
	// Stop RSSI mode.
	reg_control_1 = 0;
	reg_control_1_mask = 0;
	DINFOX_write_field(&reg_control_1, &reg_control_1_mask, 0b0, UHFM_REG_CONTROL_1_MASK_RSEN);
	NODE_write_register(NODE_REQUEST_SOURCE_EXTERNAL, UHFM_REG_ADDR_CONTROL_1, reg_control_1_mask, reg_control_1);
	// Print error.
	_AT_BUS_print_error(status);
	return;
}
#endif

#if (defined ATM) && (defined GPSM)
/*******************************************************************/
static void _AT_BUS_time_callback(void) {
	// Local variables.
	ERROR_code_t status = SUCCESS;
	PARSER_status_t parser_status = PARSER_ERROR_UNKNOWN_COMMAND;
	NODE_status_t node_status = NODE_SUCCESS;
	int32_t timeout_seconds = 0;
	uint32_t reg_config_1 = 0;
	uint32_t reg_config_1_mask = 0;
	uint32_t reg_control_1 = 0;
	uint32_t reg_control_1_mask = 0;
	uint32_t reg_status = 0;
	uint32_t reg_time_data_0 = 0;
	uint32_t reg_time_data_1 = 0;
	uint32_t reg_time_data_2 = 0;
	int32_t value = 0;
	// Read timeout parameter.
	parser_status = PARSER_get_parameter(&at_bus_ctx.parser, STRING_FORMAT_DECIMAL, STRING_CHAR_NULL, &timeout_seconds);
	PARSER_stack_exit_error(ERROR_BASE_PARSER + parser_status);
	// Configure time acquisition.
	DINFOX_write_field(&reg_config_1, &reg_config_1_mask, (uint32_t) timeout_seconds, GPSM_REG_CONFIGURATION_1_MASK_TIME_TIMEOUT);
	DINFOX_write_field(&reg_control_1, &reg_control_1_mask, 0b1, GPSM_REG_CONTROL_1_MASK_TTRG);
	// Write configuration.
	NODE_write_register(NODE_REQUEST_SOURCE_INTERNAL, GPSM_REG_ADDR_CONFIGURATION_1, reg_config_1_mask, reg_config_1);
	// Start GPS acquisition.
	_AT_BUS_reply_add_string("GPS running...");
	_AT_BUS_reply_send();
	node_status = NODE_write_register(NODE_REQUEST_SOURCE_EXTERNAL, GPSM_REG_ADDR_CONTROL_1, reg_control_1_mask, reg_control_1);
	NODE_stack_exit_error(ERROR_BASE_NODE + node_status);
	// Read status.
	NODE_read_register(NODE_REQUEST_SOURCE_INTERNAL, GPSM_REG_ADDR_STATUS_1, &reg_status);
	// Check status.
	if (DINFOX_read_field(reg_status, GPSM_REG_STATUS_1_MASK_TFS) == 0) {
		_AT_BUS_reply_add_string("Timeout");
	}
	else {
		// Read registers.
		NODE_read_register(NODE_REQUEST_SOURCE_INTERNAL, GPSM_REG_ADDR_TIME_DATA_0, &reg_time_data_0);
		NODE_read_register(NODE_REQUEST_SOURCE_INTERNAL, GPSM_REG_ADDR_TIME_DATA_1, &reg_time_data_1);
		NODE_read_register(NODE_REQUEST_SOURCE_INTERNAL, GPSM_REG_ADDR_TIME_DATA_2, &reg_time_data_2);
		// Year.
		value = (int32_t) DINFOX_get_year(DINFOX_read_field(reg_time_data_0, GPSM_REG_TIME_DATA_0_MASK_YEAR));
		_AT_BUS_reply_add_value(value, STRING_FORMAT_DECIMAL, 0);
		_AT_BUS_reply_add_string("-");
		// Month.
		value = (int32_t) DINFOX_read_field(reg_time_data_0, GPSM_REG_TIME_DATA_0_MASK_MONTH);
		if (value < 10) {
			_AT_BUS_reply_add_value(0, STRING_FORMAT_DECIMAL, 0);
		}
		_AT_BUS_reply_add_value(value, STRING_FORMAT_DECIMAL, 0);
		_AT_BUS_reply_add_string("-");
		// Date.
		value = (int32_t) DINFOX_read_field(reg_time_data_0, GPSM_REG_TIME_DATA_0_MASK_DATE);
		if (value < 10) {
			_AT_BUS_reply_add_value(0, STRING_FORMAT_DECIMAL, 0);
		}
		_AT_BUS_reply_add_value(value, STRING_FORMAT_DECIMAL, 0);
		_AT_BUS_reply_add_string(" ");
		// Hours.
		value = (int32_t) DINFOX_read_field(reg_time_data_1, GPSM_REG_TIME_DATA_1_MASK_HOUR);
		if (value < 10) {
			_AT_BUS_reply_add_value(0, STRING_FORMAT_DECIMAL, 0);
		}
		_AT_BUS_reply_add_value(value, STRING_FORMAT_DECIMAL, 0);
		_AT_BUS_reply_add_string(":");
		// Minutes.
		value = (int32_t) DINFOX_read_field(reg_time_data_1, GPSM_REG_TIME_DATA_1_MASK_MINUTE);
		if (value < 10) {
			_AT_BUS_reply_add_value(0, STRING_FORMAT_DECIMAL, 0);
		}
		_AT_BUS_reply_add_value(value, STRING_FORMAT_DECIMAL, 0);
		_AT_BUS_reply_add_string(":");
		// Seconds.
		value = (int32_t) DINFOX_read_field(reg_time_data_1, GPSM_REG_TIME_DATA_1_MASK_SECOND);
		if (value < 10) {
			_AT_BUS_reply_add_value(0, STRING_FORMAT_DECIMAL, 0);
		}
		_AT_BUS_reply_add_value(value, STRING_FORMAT_DECIMAL, 0);
		// Fix duration.
		value = (int32_t) DINFOX_read_field(reg_time_data_2, GPSM_REG_TIME_DATA_2_MASK_FIX_DURATION);
		_AT_BUS_reply_add_string(" UTC (fix ");
		_AT_BUS_reply_add_value(value, STRING_FORMAT_DECIMAL, 0);
		_AT_BUS_reply_add_string("s)");
	}
	_AT_BUS_reply_send();
	_AT_BUS_print_ok();
	return;
errors:
	_AT_BUS_print_error(status);
	return;
}
#endif

#if (defined ATM) && (defined GPSM)
/*******************************************************************/
static void _AT_BUS_gps_callback(void) {
	// Local variables.
	ERROR_code_t status = SUCCESS;
	PARSER_status_t parser_status = PARSER_ERROR_UNKNOWN_COMMAND;
	NODE_status_t node_status = NODE_SUCCESS;
	int32_t timeout_seconds = 0;
	uint32_t reg_config_1 = 0;
	uint32_t reg_config_1_mask = 0;
	uint32_t reg_control_1 = 0;
	uint32_t reg_control_1_mask = 0;
	uint32_t reg_status = 0;
	uint32_t reg_geoloc_data_0 = 0;
	uint32_t reg_geoloc_data_1 = 0;
	uint32_t reg_geoloc_data_2 = 0;
	uint32_t reg_geoloc_data_3 = 0;
	// Read timeout parameter.
	parser_status = PARSER_get_parameter(&at_bus_ctx.parser, STRING_FORMAT_DECIMAL, STRING_CHAR_NULL, &timeout_seconds);
	PARSER_stack_exit_error(ERROR_BASE_PARSER + parser_status);
	// Configure GPS acquisition.
	DINFOX_write_field(&reg_config_1, &reg_config_1_mask, (uint32_t) timeout_seconds, GPSM_REG_CONFIGURATION_1_MASK_GEOLOC_TIMEOUT);
	DINFOX_write_field(&reg_control_1, &reg_control_1_mask, 0b1, GPSM_REG_CONTROL_1_MASK_GTRG);
	// Write configuration.
	NODE_write_register(NODE_REQUEST_SOURCE_INTERNAL, GPSM_REG_ADDR_CONFIGURATION_1, reg_config_1_mask, reg_config_1);
	// Start GPS acquisition.
	_AT_BUS_reply_add_string("GPS running...");
	_AT_BUS_reply_send();
	node_status = NODE_write_register(NODE_REQUEST_SOURCE_EXTERNAL, GPSM_REG_ADDR_CONTROL_1, reg_control_1_mask, reg_control_1);
	NODE_stack_exit_error(ERROR_BASE_NODE + node_status);
	// Read status.
	NODE_read_register(NODE_REQUEST_SOURCE_INTERNAL, GPSM_REG_ADDR_STATUS_1, &reg_status);
	// Check status.
	if (DINFOX_read_field(reg_status, GPSM_REG_STATUS_1_MASK_GFS) == 0) {
		_AT_BUS_reply_add_string("Timeout");
	}
	else {
		// Read registers.
		NODE_read_register(NODE_REQUEST_SOURCE_INTERNAL, GPSM_REG_ADDR_GEOLOC_DATA_0, &reg_geoloc_data_0);
		NODE_read_register(NODE_REQUEST_SOURCE_INTERNAL, GPSM_REG_ADDR_GEOLOC_DATA_1, &reg_geoloc_data_1);
		NODE_read_register(NODE_REQUEST_SOURCE_INTERNAL, GPSM_REG_ADDR_GEOLOC_DATA_2, &reg_geoloc_data_2);
		NODE_read_register(NODE_REQUEST_SOURCE_INTERNAL, GPSM_REG_ADDR_GEOLOC_DATA_3, &reg_geoloc_data_3);
		// Latitude degrees.
		_AT_BUS_reply_add_string("Lat=");
		_AT_BUS_reply_add_value((int32_t) DINFOX_read_field(reg_geoloc_data_0, GPSM_REG_GEOLOC_DATA_0_MASK_DEGREE), STRING_FORMAT_DECIMAL, 0);
		_AT_BUS_reply_add_string("d");
		// Latitude minutes.
		_AT_BUS_reply_add_value((int32_t) DINFOX_read_field(reg_geoloc_data_0, GPSM_REG_GEOLOC_DATA_0_MASK_MINUTE), STRING_FORMAT_DECIMAL, 0);
		_AT_BUS_reply_add_string("'");
		// Latitude seconds.
		_AT_BUS_reply_add_value((int32_t) DINFOX_read_field(reg_geoloc_data_0, GPSM_REG_GEOLOC_DATA_0_MASK_SECOND), STRING_FORMAT_DECIMAL, 0);
		_AT_BUS_reply_add_string("''");
		// Latitude north flag.
		_AT_BUS_reply_add_string((DINFOX_read_field(reg_geoloc_data_0, GPSM_REG_GEOLOC_DATA_0_MASK_NF) == 0) ? "S" : "N");
		// Longitude degrees.
		_AT_BUS_reply_add_string(" Long=");
		_AT_BUS_reply_add_value((int32_t) DINFOX_read_field(reg_geoloc_data_1, GPSM_REG_GEOLOC_DATA_1_MASK_DEGREE), STRING_FORMAT_DECIMAL, 0);
		_AT_BUS_reply_add_string("d");
		// Longitude minutes.
		_AT_BUS_reply_add_value((int32_t) DINFOX_read_field(reg_geoloc_data_1, GPSM_REG_GEOLOC_DATA_1_MASK_MINUTE), STRING_FORMAT_DECIMAL, 0);
		_AT_BUS_reply_add_string("'");
		// Longitude seconds.
		_AT_BUS_reply_add_value((int32_t) DINFOX_read_field(reg_geoloc_data_1, GPSM_REG_GEOLOC_DATA_1_MASK_SECOND), STRING_FORMAT_DECIMAL, 0);
		_AT_BUS_reply_add_string("''");
		// Longitude east flag.
		_AT_BUS_reply_add_string((DINFOX_read_field(reg_geoloc_data_1, GPSM_REG_GEOLOC_DATA_1_MASK_EF) == 0) ? "W" : "E");
		// Altitude.
		_AT_BUS_reply_add_string(" Alt=");
		_AT_BUS_reply_add_value((int32_t) DINFOX_read_field(reg_geoloc_data_2, GPSM_REG_GEOLOC_DATA_2_MASK_ALTITUDE), STRING_FORMAT_DECIMAL, 0);
		_AT_BUS_reply_add_string("m");
		// Fix duration.
		_AT_BUS_reply_add_string(" (fix ");
		_AT_BUS_reply_add_value((int32_t) DINFOX_read_field(reg_geoloc_data_3, GPSM_REG_GEOLOC_DATA_3_MASK_FIX_DURATION), STRING_FORMAT_DECIMAL, 0);
		_AT_BUS_reply_add_string("s)");
	}
	_AT_BUS_reply_send();
	_AT_BUS_print_ok();
	return;
errors:
	_AT_BUS_print_error(status);
	return;
}
#endif

#if (defined ATM) && (defined GPSM)
/*******************************************************************/
static void _AT_BUS_pulse_callback(void) {
	// Local variables.
	ERROR_code_t status = SUCCESS;
	PARSER_status_t parser_status = PARSER_ERROR_UNKNOWN_COMMAND;
	NODE_status_t node_status = NODE_SUCCESS;
	int32_t active = 0;
	int32_t frequency_hz = 0;
	int32_t duty_cycle_percent;
	uint32_t reg_config_2 = 0;
	uint32_t reg_config_2_mask = 0;
	uint32_t reg_config_3 = 0;
	uint32_t reg_config_3_mask = 0;
	uint32_t reg_control_1 = 0;
	uint32_t reg_control_1_mask = 0;
	// Read parameters.
	parser_status = PARSER_get_parameter(&at_bus_ctx.parser, STRING_FORMAT_BOOLEAN, AT_BUS_CHAR_SEPARATOR, &active);
	PARSER_stack_exit_error(ERROR_BASE_PARSER + parser_status);
	parser_status = PARSER_get_parameter(&at_bus_ctx.parser, STRING_FORMAT_DECIMAL, AT_BUS_CHAR_SEPARATOR, &frequency_hz);
	PARSER_stack_exit_error(ERROR_BASE_PARSER + parser_status);
	parser_status = PARSER_get_parameter(&at_bus_ctx.parser, STRING_FORMAT_DECIMAL, STRING_CHAR_NULL, &duty_cycle_percent);
	PARSER_stack_exit_error(ERROR_BASE_PARSER + parser_status);
	// Configure timepulse signal.
	DINFOX_write_field(&reg_config_2, &reg_config_2_mask, (uint32_t) frequency_hz, GPSM_REG_CONFIGURATION_2_MASK_TP_FREQUENCY);
	DINFOX_write_field(&reg_config_3, &reg_config_3_mask, (uint32_t) duty_cycle_percent, GPSM_REG_RADIO_TEST_0_MASK_TP_DUTY_CYCLE);
	DINFOX_write_field(&reg_control_1, &reg_control_1_mask, (uint32_t) active, GPSM_REG_CONTROL_1_MASK_TPEN);
	// Write configuration.
	node_status = NODE_write_register(NODE_REQUEST_SOURCE_EXTERNAL, GPSM_REG_ADDR_CONFIGURATION_2, reg_config_2_mask, reg_config_2);
	NODE_stack_exit_error(ERROR_BASE_NODE + node_status);
	node_status = NODE_write_register(NODE_REQUEST_SOURCE_EXTERNAL, GPSM_REG_ADDR_CONFIGURATION_3, reg_config_3_mask, reg_config_3);
	NODE_stack_exit_error(ERROR_BASE_NODE + node_status);
	// Control timepulse output.
	node_status = NODE_write_register(NODE_REQUEST_SOURCE_EXTERNAL, GPSM_REG_ADDR_CONTROL_1, reg_control_1_mask, reg_control_1);
	NODE_stack_exit_error(ERROR_BASE_NODE + node_status);
	_AT_BUS_print_ok();
	return;
errors:
	_AT_BUS_print_error(status);
	return;
}
#endif

/*******************************************************************/
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

/*******************************************************************/
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

/*******************************************************************/
void AT_BUS_init(void) {
	// Local variables.
	NVM_status_t nvm_status = NVM_SUCCESS;
	LBUS_status_t lbus_status = LBUS_SUCCESS;
	NODE_address_t self_address = 0;
	// Read self address in NVM.
	nvm_status = NVM_read_byte(NVM_ADDRESS_SELF_ADDRESS, &self_address);
	NVM_stack_error();
	// Init LBUS layer.
	lbus_status = LBUS_init(self_address, &_AT_BUS_fill_rx_buffer);
	LBUS_stack_error();
	// Init registers.
	NODE_init(self_address);
	// Init context.
	_AT_BUS_reset_parser();
	// Enable receiver.
	LBUS_enable_rx();
}

/*******************************************************************/
void AT_BUS_task(void) {
	// Trigger decoding function if line end found.
	if (at_bus_ctx.line_end_flag != 0) {
		// Decode and execute command.
		LBUS_disable_rx();
		_AT_BUS_decode();
		LBUS_enable_rx();
	}
}

#if (defined ATM) && (defined UHFM)
/*******************************************************************/
void AT_BUS_print_dl_payload(sfx_u8 *dl_payload, sfx_u8 dl_payload_size, sfx_s16 rssi_dbm) {
	// Local variables.
	uint8_t idx = 0;
	// Print DL payload.
	_AT_BUS_reply_add_string("+RX=");
	for (idx=0 ; idx<dl_payload_size ; idx++) {
		_AT_BUS_reply_add_value(dl_payload[idx], STRING_FORMAT_HEXADECIMAL, 0);
	}
	_AT_BUS_reply_add_string(" (RSSI=");
	_AT_BUS_reply_add_value(rssi_dbm, STRING_FORMAT_DECIMAL, 0);
	_AT_BUS_reply_add_string("dBm)");
	_AT_BUS_reply_send();
}
#endif

/*
 * uhfm.c
 *
 *  Created on: Jun 4, 2023
 *      Author: ludo
 */

#include "uhfm.h"

#include "adc.h"
#include "aes.h"
#include "dinfox.h"
#include "error.h"
#include "load.h"
#include "node.h"
#include "nvm.h"
#ifdef UHFM
#include "manuf/rf_api.h"
#include "s2lp.h"
#include "sigfox_ep_addon_rfp_api.h"
#include "sigfox_ep_api.h"
#include "sigfox_rc.h"
#include "sigfox_types.h"
#endif

#ifdef UHFM

/*** UHFM local macros ***/

#define UHFM_REG_SIGFOX_EP_CONFIGURATION_0_DEFAULT_VALUE	0x00000EC0
#define UHFM_REG_SIGFOX_EP_CONFIGURATION_1_DEFAULT_VALUE	0x07D001F4
#define UHFM_REG_SIGFOX_EP_CONFIGURATION_2_DEFAULT_VALUE	0x00000001

#define UHFM_REG_RADIO_TEST_0_DEFAULT_VALUE					0x33AD5EC0
#define UHFM_REG_RADIO_TEST_1_DEFAULT_VALUE					0x000000BC

#define UHFM_NUMBER_OF_SIGFOX_RC							7

#define UHFM_ADC_MEASUREMENTS_RF_FREQUENCY_HZ				830000000
#define UHFM_ADC_MEASUREMENTS_TX_POWER_DBM					14

#define UHFM_FEATURE_ADDON_RFP
#define UHFM_FEATURE_CW
#define UHFM_FEATURE_DL
#define UHFM_FEATURE_RSSI

/*** UHFM local structures ***/

typedef union {
	struct {
		unsigned radio_state : 1;
		unsigned cwen : 1;
		unsigned rsen : 1;
	};
	uint8_t all;
} UHFM_flags_t;

/*** UHFM local global variables ***/

static UHFM_flags_t uhfm_flags;

/*** UHFM local functions ***/

/* RESET UHFM ANALOG DATA.
 * @param:	None.
 * @return:	None.
 */
static void _UHFM_reset_analog_data(void) {
	// Local variables.
	NODE_status_t node_status = NODE_SUCCESS;
	// Reset fields to error value.
	node_status = NODE_write_field(NODE_REQUEST_SOURCE_INTERNAL, UHFM_REG_ADDR_ANALOG_DATA_1, UHFM_REG_ANALOG_DATA_1_MASK_VRF_TX, DINFOX_VOLTAGE_ERROR_VALUE);
	NODE_stack_error();
	node_status = NODE_write_field(NODE_REQUEST_SOURCE_INTERNAL, UHFM_REG_ADDR_ANALOG_DATA_1, UHFM_REG_ANALOG_DATA_1_MASK_VRF_RX, DINFOX_VOLTAGE_ERROR_VALUE);
	NODE_stack_error();
}

/* CHECK RADIO STATE.
 * @param expected_state:	Expected radio state.
 * @return status:			Function execution status.
 */
static NODE_status_t _UHFM_check_radio_state(uint8_t expected_state) {
	// Local variables.
	NODE_status_t status = NODE_SUCCESS;
	// Compare state.
	if (expected_state != uhfm_flags.radio_state) {
		status = NODE_ERROR_RADIO_STATE;
		goto errors;
	}
errors:
	return status;
}

/* SEND SIGFOX MESSAGE.
 * @param:			None.
 * @return status:	Function execution status.
 */
static NODE_status_t _UHFM_strg_callback(void) {
	// Local variables.
	NODE_status_t status = NODE_SUCCESS;
	NODE_status_t node_status = NODE_SUCCESS;
	SIGFOX_EP_API_status_t sigfox_ep_api_status = SIGFOX_EP_API_SUCCESS;
	SIGFOX_EP_API_config_t lib_config;
	SIGFOX_EP_API_application_message_t application_message;
	SIGFOX_EP_API_message_status_t message_status;
	uint32_t bidir_flag = 0;
	uint32_t number_of_frames = 0;
	uint32_t ul_bit_rate = 0;
	uint32_t message_type = 0;
	uint32_t ul_payload_size = 0;
	sfx_u8 ul_payload[SIGFOX_UL_PAYLOAD_MAX_SIZE_BYTES];
	sfx_u8 dl_payload[SIGFOX_DL_PAYLOAD_SIZE_BYTES];
	sfx_s16 dl_rssi_dbm = 0;
	// Reset status.
	message_status.all = 0;
	// Read number of frames.
	node_status = NODE_read_field(NODE_REQUEST_SOURCE_INTERNAL, UHFM_REG_ADDR_SIGFOX_EP_CONFIGURATION_0, UHFM_REG_SIGFOX_EP_CONFIGURATION_0_MASK_NFR, &number_of_frames);
	NODE_stack_error();
	// Read bit rate.
	node_status = NODE_read_field(NODE_REQUEST_SOURCE_INTERNAL, UHFM_REG_ADDR_SIGFOX_EP_CONFIGURATION_0, UHFM_REG_SIGFOX_EP_CONFIGURATION_0_MASK_BR, &ul_bit_rate);
	NODE_stack_error();
	// Read message type.
	node_status = NODE_read_field(NODE_REQUEST_SOURCE_INTERNAL, UHFM_REG_ADDR_SIGFOX_EP_CONFIGURATION_2, UHFM_REG_SIGFOX_EP_CONFIGURATION_2_MASK_MSGT, &message_type);
	NODE_stack_error();
	// Read bidirectional flag.
	node_status = NODE_read_field(NODE_REQUEST_SOURCE_INTERNAL, UHFM_REG_ADDR_SIGFOX_EP_CONFIGURATION_2, UHFM_REG_SIGFOX_EP_CONFIGURATION_2_MASK_BF, &bidir_flag);
	NODE_stack_error();
	// Read UL payload size.
	node_status = NODE_read_field(NODE_REQUEST_SOURCE_INTERNAL, UHFM_REG_ADDR_SIGFOX_EP_CONFIGURATION_2, UHFM_REG_SIGFOX_EP_CONFIGURATION_2_MASK_UL_PAYLOAD_SIZE, &ul_payload_size);
	NODE_stack_error();
	// Read UL payload.
	if (ul_payload_size > 0) {
		node_status = NODE_read_byte_array(NODE_REQUEST_SOURCE_INTERNAL, UHFM_REG_ADDR_SIGFOX_UL_PAYLOAD_0, (uint8_t*) ul_payload, ul_payload_size);
		NODE_stack_error();
	}
	// Check radio state.
	status = _UHFM_check_radio_state(0);
	if (status != NODE_SUCCESS) goto errors;
	// Open library.
	lib_config.rc = &SIGFOX_RC1;
	sigfox_ep_api_status = SIGFOX_EP_API_open(&lib_config);
	SIGFOX_EP_API_check_status(NODE_ERROR_SIGFOX_RF_API);
	// Update radio state.
	uhfm_flags.radio_state = 1;
	// Build message structure.
	application_message.common_parameters.number_of_frames = (sfx_u8) number_of_frames;
	application_message.common_parameters.ul_bit_rate = (SIGFOX_ul_bit_rate_t) ul_bit_rate;
#ifdef PUBLIC_KEY_CAPABLE
	application_message.common_parameters.ep_key_type = SIGFOX_EP_KEY_PRIVATE;
#endif
	application_message.type = (SIGFOX_application_message_type_t) message_type;
	application_message.bidirectional_flag = (sfx_u8) bidir_flag;
	application_message.ul_payload = (sfx_u8*) ul_payload;
	application_message.ul_payload_size_bytes = (sfx_u8) ul_payload_size;
	// Send message.
	sigfox_ep_api_status = SIGFOX_EP_API_send_application_message(&application_message);
	SIGFOX_EP_API_check_status(NODE_ERROR_SIGFOX_RF_API);
	// Read message status.
	message_status = SIGFOX_EP_API_get_message_status();
	// Check bidirectional flag.
	if ((bidir_flag != 0) && (message_status.field.dl_frame != 0)) {
		// Read downlink data.
		sigfox_ep_api_status = SIGFOX_EP_API_get_dl_payload(dl_payload, SIGFOX_DL_PAYLOAD_SIZE_BYTES, &dl_rssi_dbm);
		SIGFOX_EP_API_check_status(NODE_ERROR_SIGFOX_RF_API);
		// Write DL payload registers.
		node_status = NODE_write_byte_array(NODE_REQUEST_SOURCE_INTERNAL, UHFM_REG_ADDR_SIGFOX_DL_PAYLOAD_0, (uint8_t*) dl_payload, SIGFOX_DL_PAYLOAD_SIZE_BYTES);
		NODE_stack_error();
		// Write DL RSSI.
		node_status = NODE_write_field(NODE_REQUEST_SOURCE_INTERNAL, UHFM_REG_ADDR_STATUS_CONTROL_1, UHFM_REG_STATUS_CONTROL_1_MASK_DL_RSSI, (uint32_t) DINFOX_convert_dbm(dl_rssi_dbm));
		NODE_stack_error();
	}
errors:
	// Close library.
	SIGFOX_EP_API_close();
	// Update radio state.
	uhfm_flags.radio_state = 0;
	// Update message status in register.
	node_status = NODE_write_field(NODE_REQUEST_SOURCE_INTERNAL, UHFM_REG_ADDR_STATUS_CONTROL_1, UHFM_REG_STATUS_CONTROL_1_MASK_MESSAGE_STATUS, (uint32_t) (message_status.all));
	NODE_stack_error();
	// Clear flag.
	node_status = NODE_write_field(NODE_REQUEST_SOURCE_INTERNAL, UHFM_REG_ADDR_STATUS_CONTROL_1, UHFM_REG_STATUS_CONTROL_1_MASK_STRG, 0);
	NODE_stack_error();
	// Return status.
	return status;
}

#ifdef UHFM_FEATURE_ADDON_RFP
/* PERFORM SIGFOX TEST MODE.
 * @param:			None.
 * @return status:	Function execution status.
 */
static NODE_status_t _UHFM_ttrg_callback(void) {
	// Local variables.
	NODE_status_t status = NODE_SUCCESS;
	NODE_status_t node_status = NODE_SUCCESS;
	SIGFOX_EP_ADDON_RFP_API_status_t sigfox_ep_addon_rfp_status = SIGFOX_EP_ADDON_RFP_API_SUCCESS;
	SIGFOX_EP_ADDON_RFP_API_config_t addon_config;
	SIGFOX_EP_ADDON_RFP_API_test_mode_t test_mode;
	uint32_t ul_bit_rate = 0;
	uint32_t test_mode_ref = 0;
	// Read bit rate.
	node_status = NODE_read_field(NODE_REQUEST_SOURCE_INTERNAL, UHFM_REG_ADDR_SIGFOX_EP_CONFIGURATION_0, UHFM_REG_SIGFOX_EP_CONFIGURATION_0_MASK_BR, &ul_bit_rate);
	NODE_stack_error();
	// Read test mode.
	node_status = NODE_read_field(NODE_REQUEST_SOURCE_INTERNAL, UHFM_REG_ADDR_SIGFOX_EP_CONFIGURATION_0, UHFM_REG_SIGFOX_EP_CONFIGURATION_0_MASK_TEST_MODE, &test_mode_ref);
	NODE_stack_error();
	// Check radio state.
	status = _UHFM_check_radio_state(0);
	if (status != NODE_SUCCESS) goto errors;
	// Update radio state.
	uhfm_flags.radio_state = 1;
	// Open addon.
	addon_config.rc = &SIGFOX_RC1;
	sigfox_ep_addon_rfp_status = SIGFOX_EP_ADDON_RFP_API_open(&addon_config);
	if (sigfox_ep_addon_rfp_status != SIGFOX_EP_ADDON_RFP_API_SUCCESS) {
		status = NODE_ERROR_SIGFOX_EP_ADDON_RFP;
		goto errors;
	}
	// Call test mode function.
	test_mode.test_mode_reference = (SIGFOX_EP_ADDON_RFP_API_test_mode_reference_t) test_mode_ref;
	test_mode.ul_bit_rate = (SIGFOX_ul_bit_rate_t) ul_bit_rate;
	sigfox_ep_addon_rfp_status = SIGFOX_EP_ADDON_RFP_API_test_mode(&test_mode);
	if (sigfox_ep_addon_rfp_status != SIGFOX_EP_ADDON_RFP_API_SUCCESS) {
		status = NODE_ERROR_SIGFOX_EP_ADDON_RFP;
		goto errors;
	}
errors:
	// Close addon.
	SIGFOX_EP_ADDON_RFP_API_close();
	// Update radio state.
	uhfm_flags.radio_state = 0;
	// Clear flag.
	node_status = NODE_write_field(NODE_REQUEST_SOURCE_INTERNAL, UHFM_REG_ADDR_STATUS_CONTROL_1, UHFM_REG_STATUS_CONTROL_1_MASK_TTRG, 0);
	NODE_stack_error();
	return status;
}
#endif

#ifdef UHFM_FEATURE_DL
/* START DOWNLINK DECODER.
 * @param:			None.
 * @return status:	Function execution status.
 */
static NODE_status_t _UHFM_dtrg_callback(void) {
	// Local variables.
	NODE_status_t status = NODE_SUCCESS;
	NODE_status_t node_status = NODE_SUCCESS;
	RF_API_status_t rf_api_status = RF_API_SUCCESS;
	RF_API_radio_parameters_t radio_params;
	RF_API_rx_data_t rx_data;
	SIGFOX_EP_API_message_status_t  message_status;
	uint32_t rf_frequency_hz = 0;
	sfx_u8 dl_phy_content[SIGFOX_DL_PHY_CONTENT_SIZE_BYTES];
	sfx_s16 dl_rssi_dbm = 0;
	// Reset status.
	message_status.all = 0;
	// Read RF frequency.
	node_status = NODE_read_field(NODE_REQUEST_SOURCE_INTERNAL, UHFM_REG_ADDR_RADIO_TEST_0, UHFM_REG_RADIO_TEST_0_MASK_RF_FREQUENCY, &rf_frequency_hz);
	NODE_stack_error();
	// Check radio state.
	status = _UHFM_check_radio_state(0);
	if (status != NODE_SUCCESS) goto errors;
	// Update radio state.
	uhfm_flags.radio_state = 1;
	// Wake-up radio.
	rf_api_status = RF_API_wake_up();
	RF_API_check_status(NODE_ERROR_SIGFOX_RF_API);
	// Init radio.
	radio_params.rf_mode = RF_API_MODE_RX;
	radio_params.frequency_hz = (sfx_u32) rf_frequency_hz;
	radio_params.modulation = RF_API_MODULATION_GFSK;
	radio_params.bit_rate_bps = SIGFOX_DL_BIT_RATE_BPS;
	radio_params.tx_power_dbm_eirp = TX_POWER_DBM_EIRP;
	radio_params.deviation_hz = SIGFOX_DL_GFSK_DEVIATION_HZ;
	rf_api_status = RF_API_init(&radio_params);
	RF_API_check_status(NODE_ERROR_SIGFOX_RF_API);
	// Start downlink frame listening.
	rf_api_status = RF_API_receive(&rx_data);
	RF_API_check_status(NODE_ERROR_SIGFOX_RF_API);
	// Check status.
	if (rx_data.data_received == SFX_TRUE) {
		// Write DL PHY content registers.
		node_status = NODE_write_byte_array(NODE_REQUEST_SOURCE_INTERNAL, UHFM_REG_ADDR_SIGFOX_DL_PHY_CONTENT_0, (uint8_t*) dl_phy_content, SIGFOX_DL_PHY_CONTENT_SIZE_BYTES);
		NODE_stack_error();
		// Write DL RSSI.
		node_status = NODE_write_field(NODE_REQUEST_SOURCE_INTERNAL, UHFM_REG_ADDR_STATUS_CONTROL_1, UHFM_REG_STATUS_CONTROL_1_MASK_DL_RSSI, (uint32_t) DINFOX_convert_dbm(dl_rssi_dbm));
		NODE_stack_error();
		// Update message status.
		message_status.field.dl_frame = 1;
	}
errors:
	if (status != NODE_ERROR_RADIO_STATE) {
		// Stop radio.
		RF_API_de_init();
		RF_API_sleep();
	}
	// Update radio state.
	uhfm_flags.radio_state = 0;
	// Update message status in register.
	node_status = NODE_write_field(NODE_REQUEST_SOURCE_INTERNAL, UHFM_REG_ADDR_STATUS_CONTROL_1, UHFM_REG_STATUS_CONTROL_1_MASK_MESSAGE_STATUS, (uint32_t) (message_status.all));
	NODE_stack_error();
	// Clear flag.
	node_status = NODE_write_field(NODE_REQUEST_SOURCE_INTERNAL, UHFM_REG_ADDR_STATUS_CONTROL_1, UHFM_REG_STATUS_CONTROL_1_MASK_DTRG, 0);
	NODE_stack_error();
	// Return.
	return status;
}
#endif

#ifdef UHFM_FEATURE_CW
/* MANAGE CONTINUOUS WAVE MODE.
 * @param state:	Start (1) or stop (0) continuous wave generation.
 * @return status:	Function execution status.
 */
static NODE_status_t _UHFM_cwen_callback(uint8_t state) {
	// Local variables.
	NODE_status_t status = NODE_SUCCESS;
	NODE_status_t node_status = NODE_SUCCESS;
	RF_API_status_t rf_api_status = RF_API_SUCCESS;
	RF_API_radio_parameters_t radio_params;
	S2LP_status_t s2lp_status = S2LP_SUCCESS;
	uint32_t rf_frequency_hz = 0;
	uint32_t tx_power = 0;
	int8_t tx_power_dbm = 0;
	// Read RF frequency.
	node_status = NODE_read_field(NODE_REQUEST_SOURCE_INTERNAL, UHFM_REG_ADDR_RADIO_TEST_0, UHFM_REG_RADIO_TEST_0_MASK_RF_FREQUENCY, &rf_frequency_hz);
	NODE_stack_error();
	// Read CW power.
	node_status = NODE_read_field(NODE_REQUEST_SOURCE_INTERNAL, UHFM_REG_ADDR_RADIO_TEST_1, UHFM_REG_RADIO_TEST_1_MASK_TX_POWER, &tx_power);
	NODE_stack_error();
	// Convert power.
	tx_power_dbm = (int8_t) DINFOX_get_dbm(tx_power);
	// Check state.
	if (state == 0) {
		// Check radio state.
		status = _UHFM_check_radio_state(1);
		if (status != NODE_SUCCESS) goto errors;
		// Stop CW.
		rf_api_status = RF_API_de_init();
		RF_API_check_status(NODE_ERROR_SIGFOX_RF_API);
		rf_api_status = RF_API_sleep();
		RF_API_check_status(NODE_ERROR_SIGFOX_RF_API);
		// Update radio state.
		uhfm_flags.radio_state = 0;
	}
	else {
		// Check radio state.
		status = _UHFM_check_radio_state(0);
		if (status != NODE_SUCCESS) goto errors;
		// Update radio state.
		uhfm_flags.radio_state = 1;
		// Wake-up radio.
		rf_api_status = RF_API_wake_up();
		RF_API_check_status(NODE_ERROR_SIGFOX_RF_API);
		// Init radio.
		radio_params.rf_mode = RF_API_MODE_TX;
		radio_params.frequency_hz = (sfx_u32) rf_frequency_hz;
		radio_params.modulation = RF_API_MODULATION_NONE;
		radio_params.bit_rate_bps = 0;
		radio_params.tx_power_dbm_eirp = tx_power_dbm;
		radio_params.deviation_hz = 0;
		rf_api_status = RF_API_init(&radio_params);
		RF_API_check_status(NODE_ERROR_SIGFOX_RF_API);
		// Start CW.
		s2lp_status = S2LP_send_command(S2LP_COMMAND_READY);
		if (s2lp_status != S2LP_SUCCESS) goto errors;
		s2lp_status = S2LP_wait_for_state(S2LP_STATE_READY);
		if (s2lp_status != S2LP_SUCCESS) goto errors;
		s2lp_status = S2LP_send_command(S2LP_COMMAND_TX);
		if (s2lp_status != S2LP_SUCCESS) goto errors;
	}
	return status;
errors:
	if (status != NODE_ERROR_RADIO_STATE) {
		// Stop radio.
		RF_API_de_init();
		RF_API_sleep();
		// Update radio state.
		uhfm_flags.radio_state = 0;
	}
	return status;
}
#endif

#ifdef UHFM_FEATURE_RSSI
/* MANAGE CONTINUOUS RSSI MODE.
 * @param state:	Start (1) or stop (0) continuous RSSI measurement.
 * @return status:	Function execution status.
 */
static NODE_status_t _UHFM_rsen_callback(uint8_t state) {
	// Local variables.
	NODE_status_t status = NODE_SUCCESS;
	NODE_status_t node_status = NODE_SUCCESS;
	RF_API_status_t rf_api_status = RF_API_SUCCESS;
	RF_API_radio_parameters_t radio_params;
	S2LP_status_t s2lp_status = S2LP_SUCCESS;
	uint32_t rf_frequency_hz = 0;
	// Read RF frequency.
	node_status = NODE_read_field(NODE_REQUEST_SOURCE_INTERNAL, UHFM_REG_ADDR_RADIO_TEST_0, UHFM_REG_RADIO_TEST_0_MASK_RF_FREQUENCY, &rf_frequency_hz);
	NODE_stack_error();
	// Check state.
	if (state == 0) {
		// Check radio state.
		status = _UHFM_check_radio_state(1);
		if (status != NODE_SUCCESS) goto errors;
		// Stop continuous listening.
		rf_api_status = RF_API_de_init();
		RF_API_check_status(NODE_ERROR_SIGFOX_RF_API);
		rf_api_status = RF_API_sleep();
		RF_API_check_status(NODE_ERROR_SIGFOX_RF_API);
		// Update radio state.
		uhfm_flags.radio_state = 0;
	}
	else {
		// Check radio state.
		status = _UHFM_check_radio_state(0);
		if (status != NODE_SUCCESS) goto errors;
		// Update radio state.
		uhfm_flags.radio_state = 1;
		// Wake-up radio.
		rf_api_status = RF_API_wake_up();
		RF_API_check_status(NODE_ERROR_SIGFOX_RF_API);
		// Init radio.
		radio_params.rf_mode = RF_API_MODE_RX;
		radio_params.frequency_hz = (sfx_u32) rf_frequency_hz;
		radio_params.modulation = RF_API_MODULATION_NONE;
		radio_params.bit_rate_bps = 0;
		radio_params.tx_power_dbm_eirp = 0;
		radio_params.deviation_hz = 0;
		rf_api_status = RF_API_init(&radio_params);
		RF_API_check_status(NODE_ERROR_SIGFOX_RF_API);
		// Start continuous listening.
		s2lp_status = S2LP_send_command(S2LP_COMMAND_READY);
		S2LP_stack_error();
		s2lp_status = S2LP_wait_for_state(S2LP_STATE_READY);
		S2LP_stack_error();
		s2lp_status = S2LP_send_command(S2LP_COMMAND_RX);
		S2LP_stack_error();
	}
	return status;
errors:
	if (status != NODE_ERROR_RADIO_STATE) {
		// Stop radio.
		RF_API_de_init();
		RF_API_sleep();
	}
	// Update local flag.
	uhfm_flags.rsen = 0;
	return status;
}
#endif

/*** UHFM functions ***/

/* INIT UHFM REGISTERS.
 * @param:	None.
 * @return:	None.
 */
void UHFM_init_registers(void) {
	// Local variables.
	NODE_status_t node_status = NODE_SUCCESS;
	NVM_status_t nvm_status = NVM_SUCCESS;
	uint8_t idx = 0;
	uint8_t sigfox_ep_tab[SIGFOX_EP_KEY_SIZE_BYTES];
	// Init flags.
	uhfm_flags.all = 0;
	// Sigfox EP ID register.
	for (idx=0 ; idx<SIGFOX_EP_ID_SIZE_BYTES ; idx++) {
		// Read byte.
		nvm_status = NVM_read_byte((NVM_ADDRESS_SIGFOX_EP_ID + idx), &(sigfox_ep_tab[idx]));
		NVM_stack_error();
	}
	// Write registers.
	node_status = NODE_write_byte_array(NODE_REQUEST_SOURCE_INTERNAL, UHFM_REG_ADDR_SIGFOX_EP_ID, (uint8_t*) sigfox_ep_tab, SIGFOX_EP_ID_SIZE_BYTES);
	NODE_stack_error();
	// Sigfox EP key register.
	for (idx=0 ; idx<SIGFOX_EP_KEY_SIZE_BYTES ; idx++) {
		// Read byte.
		nvm_status = NVM_read_byte((NVM_ADDRESS_SIGFOX_EP_KEY + idx), &(sigfox_ep_tab[idx]));
		NVM_stack_error();
	}
	// Write registers.
	node_status = NODE_write_byte_array(NODE_REQUEST_SOURCE_INTERNAL, UHFM_REG_ADDR_SIGFOX_EP_KEY_0, (uint8_t*) sigfox_ep_tab, SIGFOX_EP_KEY_SIZE_BYTES);
	NODE_stack_error();
	// Load default values.
	_UHFM_reset_analog_data();
	node_status = NODE_write_register(NODE_REQUEST_SOURCE_INTERNAL, UHFM_REG_ADDR_SIGFOX_EP_CONFIGURATION_0, DINFOX_REG_MASK_ALL, UHFM_REG_SIGFOX_EP_CONFIGURATION_0_DEFAULT_VALUE);
	NODE_stack_error();
	node_status = NODE_write_register(NODE_REQUEST_SOURCE_INTERNAL, UHFM_REG_ADDR_SIGFOX_EP_CONFIGURATION_1, DINFOX_REG_MASK_ALL, UHFM_REG_SIGFOX_EP_CONFIGURATION_1_DEFAULT_VALUE);
	NODE_stack_error();
	node_status = NODE_write_register(NODE_REQUEST_SOURCE_INTERNAL, UHFM_REG_ADDR_SIGFOX_EP_CONFIGURATION_2, DINFOX_REG_MASK_ALL, UHFM_REG_SIGFOX_EP_CONFIGURATION_2_DEFAULT_VALUE);
	NODE_stack_error();
	node_status = NODE_write_register(NODE_REQUEST_SOURCE_INTERNAL, UHFM_REG_ADDR_RADIO_TEST_0, DINFOX_REG_MASK_ALL, UHFM_REG_RADIO_TEST_0_DEFAULT_VALUE);
	NODE_stack_error();
	node_status = NODE_write_register(NODE_REQUEST_SOURCE_INTERNAL, UHFM_REG_ADDR_RADIO_TEST_1, DINFOX_REG_MASK_ALL, UHFM_REG_RADIO_TEST_1_DEFAULT_VALUE);
	NODE_stack_error();
}

/* UPDATE UHFM REGISTER.
 * @param reg_addr:	Address of the register to update.
 * @return status:	Function execution status.
 */
NODE_status_t UHFM_update_register(uint8_t reg_addr) {
	// Local variables.
	NODE_status_t status = NODE_SUCCESS;
	NODE_status_t node_status = NODE_SUCCESS;
	S2LP_status_t s2lp_status = S2LP_SUCCESS;
	int16_t rssi_dbm = 0;
	// Check address.
	switch (reg_addr) {
	case UHFM_REG_ADDR_RADIO_TEST_1:
		// Check radio state.
		if ((uhfm_flags.radio_state != 0) && (uhfm_flags.rsen != 0)) {
			// RSSI.
			s2lp_status = S2LP_get_rssi(S2LP_RSSI_TYPE_RUN, &rssi_dbm);
			S2LP_stack_error();
			// Write field.
			node_status = NODE_write_field(NODE_REQUEST_SOURCE_INTERNAL, UHFM_REG_ADDR_RADIO_TEST_1, UHFM_REG_RADIO_TEST_1_MASK_RSSI, (uint32_t) DINFOX_convert_dbm(rssi_dbm));
			NODE_stack_error();
		}
		break;
	default:
		// Nothing to do for other registers.
		break;
	}
	return status;
}

/* CHECK UHFM NODE ACTIONS.
 * @param reg_addr:	Address of the register to check.
 * @return status:	Function execution status.
 */
NODE_status_t UHFM_check_register(uint8_t reg_addr) {
	// Local variables.
	NODE_status_t status = NODE_SUCCESS;
	NODE_status_t node_status = NODE_SUCCESS;
	uint32_t bit = 0;
	// Check address.
	switch (reg_addr) {
	case UHFM_REG_ADDR_STATUS_CONTROL_1:
		// Read STRG bit.
		bit = 0;
		node_status = NODE_read_field(NODE_REQUEST_SOURCE_INTERNAL, UHFM_REG_ADDR_STATUS_CONTROL_1, UHFM_REG_STATUS_CONTROL_1_MASK_STRG, &bit);
		NODE_stack_error();
		// Check bit.
		if (bit != 0) {
			// Send Sigfox message.
			status = _UHFM_strg_callback();
			if (status != NODE_SUCCESS) goto errors;
		}
#ifdef UHFM_FEATURE_ADDON_RFP
		// Read TTRG bit.
		bit = 0;
		node_status = NODE_read_field(NODE_REQUEST_SOURCE_INTERNAL, UHFM_REG_ADDR_STATUS_CONTROL_1, UHFM_REG_STATUS_CONTROL_1_MASK_TTRG, &bit);
		NODE_stack_error();
		// Check bit.
		if (bit != 0) {
			// Perform Sigfox test mode.
			status = _UHFM_ttrg_callback();
			if (status != NODE_SUCCESS) goto errors;
		}
#endif
#ifdef UHFM_FEATURE_DL
		// Read DTRG bit.
		bit = 0;
		node_status = NODE_read_field(NODE_REQUEST_SOURCE_INTERNAL, UHFM_REG_ADDR_STATUS_CONTROL_1, UHFM_REG_STATUS_CONTROL_1_MASK_DTRG, &bit);
		NODE_stack_error();
		// Check bit.
		if (bit != 0) {
			// Start downlink decoder.
			status = _UHFM_dtrg_callback();
			if (status != NODE_SUCCESS) goto errors;
		}
#endif
#ifdef UHFM_FEATURE_CW
		// Read CWEN bit.
		bit = uhfm_flags.cwen;
		node_status = NODE_read_field(NODE_REQUEST_SOURCE_INTERNAL, UHFM_REG_ADDR_STATUS_CONTROL_1, UHFM_REG_STATUS_CONTROL_1_MASK_CWEN, &bit);
		NODE_stack_error();
		// Check bit change.
		if (bit != uhfm_flags.cwen) {
			// Start or stop CW.
			status = _UHFM_cwen_callback(bit);
			if (status != NODE_SUCCESS) {
				// Clear request.
				NODE_write_field(NODE_REQUEST_SOURCE_INTERNAL, UHFM_REG_ADDR_STATUS_CONTROL_1, UHFM_REG_STATUS_CONTROL_1_MASK_CWEN, uhfm_flags.cwen);
				goto errors;
			}
			// Update local flag.
			uhfm_flags.cwen = bit;
		}
#endif
#ifdef UHFM_FEATURE_RSSI
		// Read RSSI bit.
		bit = uhfm_flags.rsen;
		node_status = NODE_read_field(NODE_REQUEST_SOURCE_INTERNAL, UHFM_REG_ADDR_STATUS_CONTROL_1, UHFM_REG_STATUS_CONTROL_1_MASK_RSEN, &bit);
		NODE_stack_error();
		// Check bit change.
		if (bit != uhfm_flags.rsen) {
			// Start or stop RSSI measurement.
			status = _UHFM_rsen_callback(bit);
			if (status != NODE_SUCCESS) {
				// Clear request.
				NODE_write_field(NODE_REQUEST_SOURCE_INTERNAL, UHFM_REG_ADDR_STATUS_CONTROL_1, UHFM_REG_STATUS_CONTROL_1_MASK_RSEN, uhfm_flags.rsen);
				goto errors;
			}
			// Update local flag.
			uhfm_flags.rsen = bit;
		}
#endif
		break;
	default:
		// Nothing to do for other registers.
		break;
	}
errors:
	return status;
}

/* MEASURE TRIGGER CALLBACK.
 * @param adc_status:	Pointer to the ADC measurements status.
 * @return status:		Function execution status.
 */
NODE_status_t UHFM_mtrg_callback(ADC_status_t* adc_status) {
	// Local variables.
	NODE_status_t status = NODE_SUCCESS;
	NODE_status_t node_status = NODE_SUCCESS;
	ADC_status_t adc1_status = ADC_SUCCESS;
	uint32_t vrf_mv = 0;
	uint32_t radio_test_0 = 0;
	uint32_t radio_test_1 = 0;
	// Reset results.
	_UHFM_reset_analog_data();
	// Check radio state.
	status = _UHFM_check_radio_state(0);
	if (status != NODE_SUCCESS) goto errors;
	// Save radio test registers.
	node_status = NODE_read_register(NODE_REQUEST_SOURCE_INTERNAL, UHFM_REG_ADDR_RADIO_TEST_0, &radio_test_0);
	NODE_stack_error();
	node_status = NODE_read_register(NODE_REQUEST_SOURCE_INTERNAL, UHFM_REG_ADDR_RADIO_TEST_1, &radio_test_1);
	NODE_stack_error();
	// Configure frequency and TX power for measure.
	node_status = NODE_write_field(NODE_REQUEST_SOURCE_INTERNAL, UHFM_REG_ADDR_RADIO_TEST_0, UHFM_REG_RADIO_TEST_0_MASK_RF_FREQUENCY, UHFM_ADC_MEASUREMENTS_RF_FREQUENCY_HZ);
	NODE_stack_error();
	node_status = NODE_write_field(NODE_REQUEST_SOURCE_INTERNAL, UHFM_REG_ADDR_RADIO_TEST_1, UHFM_REG_RADIO_TEST_1_MASK_TX_POWER, (uint32_t) DINFOX_convert_dbm(UHFM_ADC_MEASUREMENTS_TX_POWER_DBM));
	NODE_stack_error();
#ifdef UHFM_FEATURE_CW
	// Start CW.
	node_status = _UHFM_cwen_callback(1);
	NODE_stack_error();
#endif
	// Perform measurements in TX state.
	adc1_status = ADC1_perform_measurements();
	ADC1_stack_error();
#ifdef UHFM_FEATURE_CW
	// Stop CW.
	node_status = _UHFM_cwen_callback(0);
	NODE_stack_error();
#endif
	// Check status.
	if (adc1_status == ADC_SUCCESS) {
		// VRF_TX.
		adc1_status = ADC1_get_data(ADC_DATA_INDEX_VRF_MV, &vrf_mv);
		ADC1_stack_error();
		if (adc1_status == ADC_SUCCESS) {
			node_status = NODE_write_field(NODE_REQUEST_SOURCE_INTERNAL, UHFM_REG_ADDR_ANALOG_DATA_1, UHFM_REG_ANALOG_DATA_1_MASK_VRF_TX, (uint32_t) DINFOX_convert_mv(vrf_mv));
			NODE_stack_error();
		}
	}
#ifdef UHFM_FEATURE_RSSI
	// Start RX.
	node_status = _UHFM_rsen_callback(1);
	NODE_stack_error();
#endif
	// Perform measurements in RX state.
	adc1_status = ADC1_perform_measurements();
	ADC1_stack_error();
#ifdef UHFM_FEATURE_RSSI
	// Stop RX.
	node_status = _UHFM_rsen_callback(0);
	NODE_stack_error();
#endif
	// Update parameter.
	if (adc_status != NULL) {
		(*adc_status) = adc1_status;
	}
	// Check status.
	if (adc1_status == ADC_SUCCESS) {
		// VRF_RX.
		adc1_status = ADC1_get_data(ADC_DATA_INDEX_VRF_MV, &vrf_mv);
		ADC1_stack_error();
		if (adc1_status == ADC_SUCCESS) {
			node_status = NODE_write_field(NODE_REQUEST_SOURCE_INTERNAL, UHFM_REG_ADDR_ANALOG_DATA_1, UHFM_REG_ANALOG_DATA_1_MASK_VRF_RX, (uint32_t) DINFOX_convert_mv(vrf_mv));
			NODE_stack_error();
		}
	}
errors:
	// Restore radio test registers.
	node_status = NODE_write_register(NODE_REQUEST_SOURCE_INTERNAL, UHFM_REG_ADDR_RADIO_TEST_0, DINFOX_REG_MASK_ALL, radio_test_0);
	NODE_stack_error();
	node_status = NODE_write_register(NODE_REQUEST_SOURCE_INTERNAL, UHFM_REG_ADDR_RADIO_TEST_1, DINFOX_REG_MASK_ALL, radio_test_1);
	NODE_stack_error();
	return status;
}

#endif /* UHFM */

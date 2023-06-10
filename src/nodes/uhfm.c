/*
 * uhfm.c
 *
 *  Created on: Jun 4, 2023
 *      Author: ludo
 */

#include "uhfm.h"

#include "adc.h"
#include "addon_sigfox_rf_protocol_api.h"
#include "aes.h"
#include "dinfox_common.h"
#include "load.h"
#include "lvrm_reg.h"
#include "node.h"
#include "nvm.h"
#include "rf_api.h"
#include "s2lp.h"
#include "sigfox_api.h"
#include "sigfox_types.h"

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

static sfx_rc_t UHFM_SIGFOX_RC[UHFM_NUMBER_OF_SIGFOX_RC] = {RC1, RC2, RC3C, RC4, RC5, RC6, RC7};
static UHFM_flags_t uhfm_flags;

/*** UHFM local functions ***/

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
	sfx_error_t sigfox_api_status = SFX_ERR_NONE;
	uint32_t rc_index = 0;
	uint32_t bidir_flag = 0;
	uint32_t number_of_frames = 0;
	uint32_t control_message_flag = 0;
	uint32_t message_type = 0;
	uint32_t ul_payload_size = 0;
	uint8_t ul_payload[SIGFOX_UPLINK_DATA_MAX_SIZE_BYTES];
	uint8_t dl_payload[SIGFOX_DOWNLINK_DATA_SIZE_BYTES];
	int16_t dl_rssi_dbm = 0;
	UHFM_message_status_t message_status;
	// Reset status.
	message_status.all = 0;
	// Read RC index.
	status = NODE_read_field(NODE_REQUEST_SOURCE_INTERNAL, UHFM_REG_ADDR_SIGFOX_EP_CONFIGURATION_0, UHFM_REG_SIGFOX_EP_CONFIGURATION_0_MASK_RC, &rc_index);
	if (status != NODE_SUCCESS) goto errors;
	// Read number of frames.
	status = NODE_read_field(NODE_REQUEST_SOURCE_INTERNAL, UHFM_REG_ADDR_SIGFOX_EP_CONFIGURATION_0, UHFM_REG_SIGFOX_EP_CONFIGURATION_0_MASK_NFR, &number_of_frames);
	if (status != NODE_SUCCESS) goto errors;
	// Read control message flag.
	status = NODE_read_field(NODE_REQUEST_SOURCE_INTERNAL, UHFM_REG_ADDR_SIGFOX_EP_CONFIGURATION_2, UHFM_REG_SIGFOX_EP_CONFIGURATION_2_MASK_CMSG, &control_message_flag);
	if (status != NODE_SUCCESS) goto errors;
	// Read message type.
	status = NODE_read_field(NODE_REQUEST_SOURCE_INTERNAL, UHFM_REG_ADDR_SIGFOX_EP_CONFIGURATION_2, UHFM_REG_SIGFOX_EP_CONFIGURATION_2_MASK_MSGT, &message_type);
	if (status != NODE_SUCCESS) goto errors;
	// Read bidirectional flag.
	status = NODE_read_field(NODE_REQUEST_SOURCE_INTERNAL, UHFM_REG_ADDR_SIGFOX_EP_CONFIGURATION_2, UHFM_REG_SIGFOX_EP_CONFIGURATION_2_MASK_BF, &bidir_flag);
	if (status != NODE_SUCCESS) goto errors;
	// Read UL payload size.
	status = NODE_read_field(NODE_REQUEST_SOURCE_INTERNAL, UHFM_REG_ADDR_SIGFOX_EP_CONFIGURATION_2, UHFM_REG_SIGFOX_EP_CONFIGURATION_2_MASK_UL_PAYLOAD_SIZE, &ul_payload_size);
	if (status != NODE_SUCCESS) goto errors;
	// Read UL payload.
	if (ul_payload_size > 0) {
		status = NODE_read_byte_array(NODE_REQUEST_SOURCE_INTERNAL, UHFM_REG_ADDR_SIGFOX_UL_PAYLOAD_0, (uint8_t*) ul_payload, ul_payload_size);
		if (status != NODE_SUCCESS) goto errors;
	}
	// Check radio state.
	status = _UHFM_check_radio_state(0);
	if (status != NODE_SUCCESS) goto errors;
	// Open library.
	sigfox_api_status = SIGFOX_API_open(&(UHFM_SIGFOX_RC[rc_index]));
	SIGFOX_API_status_check(NODE_ERROR_BASE_SIGFOX);
	// Update radio state.
	uhfm_flags.radio_state = 1;
	// Check control message flag.
	if (control_message_flag == 0) {
		// Check message type.
		switch (message_type) {
		case 1:
			sigfox_api_status = SIGFOX_API_send_bit(0, dl_payload, (number_of_frames - 1), (sfx_bool) bidir_flag);
			break;
		case 2:
			sigfox_api_status = SIGFOX_API_send_bit(1, dl_payload, (number_of_frames - 1), (sfx_bool) bidir_flag);
			break;
		case 3:
			sigfox_api_status = SIGFOX_API_send_frame(ul_payload, ul_payload_size, dl_payload, (number_of_frames - 1), (sfx_bool) bidir_flag);
			break;
		default:
			// Empty frame or unknown value.
			status = NODE_ERROR_MESSAGE_TYPE;
			goto errors;
		}
		// Do not consider downlink timeout error.
		if (((sfx_u8) (sigfox_api_status & 0x00FF)) == SFX_ERR_INT_GET_RECEIVED_FRAMES_TIMEOUT) {
			// Update message status.
			message_status.ul_frame_1 = 1;
			message_status.ul_frame_2 = 1;
			message_status.ul_frame_3 = 1;
			message_status.network_error = 1;
		}
		else {
			// Check status.
			if (sigfox_api_status != SFX_ERR_NONE) {
				message_status.execution_error = 1;
			}
			SIGFOX_API_status_check(NODE_ERROR_BASE_SIGFOX);
			// Update message status.
			message_status.ul_frame_1 = 1;
			message_status.ul_frame_2 = 1;
			message_status.ul_frame_3 = 1;
			// Check bidirectional flag.
			if (bidir_flag != 0) {
				// Write DL payload registers.
				status = NODE_write_byte_array(NODE_REQUEST_SOURCE_INTERNAL, UHFM_REG_ADDR_SIGFOX_DL_PAYLOAD_0, (uint8_t*) dl_payload, SIGFOX_DOWNLINK_DATA_SIZE_BYTES);
				if (status != NODE_SUCCESS) goto errors;
				// Write DL RSSI.
				status = NODE_write_field(NODE_REQUEST_SOURCE_INTERNAL, UHFM_REG_ADDR_STATUS_CONTROL_1, UHFM_REG_STATUS_CONTROL_1_MASK_DL_RSSI, (uint32_t) DINFOX_convert_dbm(dl_rssi_dbm));
				if (status != NODE_SUCCESS) goto errors;
				// Update message status.
				message_status.dl_frame = 1;
				message_status.dl_conf_frame = 1;
			}
		}
	}
	else {
		sigfox_api_status = SIGFOX_API_send_outofband(SFX_OOB_SERVICE);
		SIGFOX_API_status_check(NODE_ERROR_BASE_SIGFOX);
	}
errors:
	// Close library.
	SIGFOX_API_close();
	// Update radio state.
	uhfm_flags.radio_state = 0;
	// Update message status in register.
	NODE_write_field(NODE_REQUEST_SOURCE_INTERNAL, UHFM_REG_ADDR_STATUS_CONTROL_1, UHFM_REG_STATUS_CONTROL_1_MASK_MESSAGE_STATUS, (uint32_t) (message_status.all));
	// Clear flag.
	NODE_write_field(NODE_REQUEST_SOURCE_INTERNAL, UHFM_REG_ADDR_STATUS_CONTROL_1, UHFM_REG_STATUS_CONTROL_1_MASK_STRG, 0);
	// Return status.
	return status;
}

/* PERFORM SIGFOX TEST MODE.
 * @param:			None.
 * @return status:	Function execution status.
 */
static NODE_status_t _UHFM_ttrg_callback(void) {
	// Local variables.
	NODE_status_t status = NODE_SUCCESS;
	sfx_error_t sigfox_api_status = SFX_ERR_NONE;
	uint32_t rc_index = 0;
	uint32_t test_mode = 0;
	// Read RC index.
	status = NODE_read_field(NODE_REQUEST_SOURCE_INTERNAL, UHFM_REG_ADDR_SIGFOX_EP_CONFIGURATION_0, UHFM_REG_SIGFOX_EP_CONFIGURATION_0_MASK_RC, &rc_index);
	if (status != NODE_SUCCESS) goto errors;
	// Read test mode.
	status = NODE_read_field(NODE_REQUEST_SOURCE_INTERNAL, UHFM_REG_ADDR_SIGFOX_EP_CONFIGURATION_0, UHFM_REG_SIGFOX_EP_CONFIGURATION_0_MASK_TEST_MODE, &test_mode);
	if (status != NODE_SUCCESS) goto errors;
	// Check radio state.
	status = _UHFM_check_radio_state(0);
	if (status != NODE_SUCCESS) goto errors;
	// Update radio state.
	uhfm_flags.radio_state = 1;
	// Call test mode function wth public key.
	sigfox_api_status = ADDON_SIGFOX_RF_PROTOCOL_API_test_mode((sfx_rc_enum_t) rc_index, (sfx_test_mode_t) test_mode);
	SIGFOX_API_status_check(NODE_ERROR_BASE_SIGFOX);
errors:
	// Update radio state.
	uhfm_flags.radio_state = 0;
	// Clear flag.
	NODE_write_field(NODE_REQUEST_SOURCE_INTERNAL, UHFM_REG_ADDR_STATUS_CONTROL_1, UHFM_REG_STATUS_CONTROL_1_MASK_TTRG, 0);
	return status;
}

/* START DOWNLINK DECODER.
 * @param:			None.
 * @return status:	Function execution status.
 */
static NODE_status_t _UHFM_dtrg_callback(void) {
	// Local variables.
	NODE_status_t status = NODE_SUCCESS;
	sfx_error_t sigfox_api_status = SFX_ERR_NONE;
	uint32_t rf_frequency_hz = 0;
	sfx_rx_state_enum_t dl_status = DL_PASSED;
	sfx_u8 dl_phy_content[SIGFOX_DOWNLINK_PHY_SIZE_BYTES];
	int16_t dl_rssi_dbm = 0;
	UHFM_message_status_t message_status;
	// Reset status.
	message_status.all = 0;
	// Read RF frequency.
	status = NODE_read_field(NODE_REQUEST_SOURCE_INTERNAL, UHFM_REG_ADDR_RADIO_TEST_0, UHFM_REG_RADIO_TEST_0_MASK_RF_FREQUENCY, &rf_frequency_hz);
	if (status != NODE_SUCCESS) goto errors;
	// Check radio state.
	status = _UHFM_check_radio_state(0);
	if (status != NODE_SUCCESS) goto errors;
	// Update radio state.
	uhfm_flags.radio_state = 1;
	// Start radio.
	sigfox_api_status = RF_API_init(SFX_RF_MODE_RX);
	SIGFOX_API_status_check(NODE_ERROR_BASE_SIGFOX);
	sigfox_api_status = RF_API_change_frequency(rf_frequency_hz);
	SIGFOX_API_status_check(NODE_ERROR_BASE_SIGFOX);
	sigfox_api_status = RF_API_wait_frame(dl_phy_content, &dl_rssi_dbm, &dl_status);
	SIGFOX_API_status_check(NODE_ERROR_BASE_SIGFOX);
	// Check status.
	if (dl_status == DL_PASSED) {
		// Write DL PHY content registers.
		status = NODE_write_byte_array(NODE_REQUEST_SOURCE_INTERNAL, UHFM_REG_ADDR_SIGFOX_DL_PHY_CONTENT_0, (uint8_t*) dl_phy_content, SIGFOX_DOWNLINK_PHY_SIZE_BYTES);
		if (status != NODE_SUCCESS) goto errors;
		// Write DL RSSI.
		status = NODE_write_field(NODE_REQUEST_SOURCE_INTERNAL, UHFM_REG_ADDR_STATUS_CONTROL_1, UHFM_REG_STATUS_CONTROL_1_MASK_DL_RSSI, (uint32_t) DINFOX_convert_dbm(dl_rssi_dbm));
		if (status != NODE_SUCCESS) goto errors;
		// Update message status.
		message_status.dl_frame = 1;
	}
errors:
	if (status != NODE_ERROR_RADIO_STATE) {
		RF_API_stop();
	}
	// Update radio state.
	uhfm_flags.radio_state = 0;
	// Update message status in register.
	NODE_write_field(NODE_REQUEST_SOURCE_INTERNAL, UHFM_REG_ADDR_STATUS_CONTROL_1, UHFM_REG_STATUS_CONTROL_1_MASK_MESSAGE_STATUS, (uint32_t) (message_status.all));
	// Clear flag.
	NODE_write_field(NODE_REQUEST_SOURCE_INTERNAL, UHFM_REG_ADDR_STATUS_CONTROL_1, UHFM_REG_STATUS_CONTROL_1_MASK_DTRG, 0);
	// Return.
	return status;
}

/* MANAGE CONTINUOUS WAVE MODE.
 * @param state:	Start (1) or stop (0) continuous wave generation.
 * @return status:	Function execution status.
 */
static NODE_status_t _UHFM_cwen_callback(uint8_t state) {
	// Local variables.
	NODE_status_t status = NODE_SUCCESS;
	sfx_error_t sigfox_api_status = SFX_ERR_NONE;
	S2LP_status_t s2lp_status = S2LP_SUCCESS;
	uint32_t rf_frequency_hz = 0;
	uint32_t tx_power = 0;
	int8_t tx_power_dbm = 0;
	// Read RF frequency.
	status = NODE_read_field(NODE_REQUEST_SOURCE_INTERNAL, UHFM_REG_ADDR_RADIO_TEST_0, UHFM_REG_RADIO_TEST_0_MASK_RF_FREQUENCY, &rf_frequency_hz);
	if (status != NODE_SUCCESS) goto errors;
	// Read CW power.
	status = NODE_read_field(NODE_REQUEST_SOURCE_INTERNAL, UHFM_REG_ADDR_RADIO_TEST_1, UHFM_REG_RADIO_TEST_1_MASK_TX_POWER, &tx_power);
	if (status != NODE_SUCCESS) goto errors;
	// Convert power.
	tx_power_dbm = (int8_t) DINFOX_get_dbm(tx_power);
	// Check state.
	if (state == 0) {
		// Check radio state.
		status = _UHFM_check_radio_state(1);
		if (status != NODE_SUCCESS) goto errors;
		// Stop CW.
		sigfox_api_status = SIGFOX_API_stop_continuous_transmission();
		SIGFOX_API_status_check(NODE_ERROR_BASE_SIGFOX);
		// Update radio state.
		uhfm_flags.radio_state = 0;
	}
	else {
		// Check radio state.
		status = _UHFM_check_radio_state(0);
		if (status != NODE_SUCCESS) goto errors;
		// Update radio state.
		uhfm_flags.radio_state = 1;
		// Start CW.
		sigfox_api_status = SIGFOX_API_start_continuous_transmission((sfx_u32) rf_frequency_hz, SFX_NO_MODULATION);
		SIGFOX_API_status_check(NODE_ERROR_BASE_SIGFOX);
		s2lp_status = S2LP_set_rf_output_power(tx_power_dbm);
		S2LP_status_check(NODE_ERROR_BASE_S2LP);
	}
	return status;
errors:
	if (status != NODE_ERROR_RADIO_STATE) {
		SIGFOX_API_stop_continuous_transmission();
		// Update radio state.
		uhfm_flags.radio_state = 0;
	}
	return status;
}

/* MANAGE CONTINUOUS RSSI MODE.
 * @param state:	Start (1) or stop (0) continuous RSSI measurement.
 * @return status:	Function execution status.
 */
static NODE_status_t _UHFM_rsen_callback(uint8_t state) {
	// Local variables.
	NODE_status_t status = NODE_SUCCESS;
	sfx_error_t sigfox_api_status = SFX_ERR_NONE;
	S2LP_status_t s2lp_status = S2LP_SUCCESS;
	uint32_t rf_frequency_hz = 0;
	// Read RF frequency.
	status = NODE_read_field(NODE_REQUEST_SOURCE_INTERNAL, UHFM_REG_ADDR_RADIO_TEST_0, UHFM_REG_RADIO_TEST_0_MASK_RF_FREQUENCY, &rf_frequency_hz);
	if (status != NODE_SUCCESS) goto errors;
	// Check state.
	if (state == 0) {
		// Check radio state.
		status = _UHFM_check_radio_state(1);
		if (status != NODE_SUCCESS) goto errors;
		// Stop continuous listening.
		sigfox_api_status = RF_API_stop();
		SIGFOX_API_status_check(NODE_ERROR_BASE_SIGFOX);
		// Update radio state.
		uhfm_flags.radio_state = 0;
	}
	else {
		// Check radio state.
		status = _UHFM_check_radio_state(0);
		if (status != NODE_SUCCESS) goto errors;
		// Update radio state.
		uhfm_flags.radio_state = 1;
		// Init radio.
		sigfox_api_status = RF_API_init(SFX_RF_MODE_RX);
		SIGFOX_API_status_check(NODE_ERROR_BASE_SIGFOX);
		sigfox_api_status = RF_API_change_frequency((sfx_u32) rf_frequency_hz);
		SIGFOX_API_status_check(NODE_ERROR_BASE_SIGFOX);
		// Start continuous listening.
		s2lp_status = S2LP_send_command(S2LP_COMMAND_READY);
		S2LP_status_check(NODE_ERROR_BASE_S2LP);
		s2lp_status = S2LP_wait_for_state(S2LP_STATE_READY);
		S2LP_status_check(NODE_ERROR_BASE_S2LP);
		// Start radio.
		s2lp_status = S2LP_send_command(S2LP_COMMAND_RX);
		S2LP_status_check(NODE_ERROR_BASE_S2LP);
	}
	return status;
errors:
	if (status != NODE_ERROR_RADIO_STATE) {
		RF_API_stop();
	}
	// Update local flag.
	uhfm_flags.rsen = 0;
	return status;
}

/*** UHFM functions ***/

/* INIT UHFM REGISTERS.
 * @param:			None.
 * @return status:	Function execution status.
 */
NODE_status_t UHFM_init_registers(void) {
	// Local variables.
	NODE_status_t status = NODE_SUCCESS;
	NVM_status_t nvm_status = NVM_SUCCESS;
	uint8_t idx = 0;
	uint8_t sigfox_ep_tab[AES_BLOCK_SIZE];
	// Init flags.
	uhfm_flags.all = 0;
	// Sigfox EP ID register.
	for (idx=0 ; idx<ID_LENGTH ; idx++) {
		// Read byte.
		nvm_status = NVM_read_byte((NVM_ADDRESS_SIGFOX_DEVICE_ID + idx), &(sigfox_ep_tab[idx]));
		NVM_status_check(NODE_ERROR_BASE_NVM);
	}
	// Write registers.
	status = NODE_write_byte_array(NODE_REQUEST_SOURCE_INTERNAL, UHFM_REG_ADDR_SIGFOX_EP_ID, (uint8_t*) sigfox_ep_tab, ID_LENGTH);
	if (status != NODE_SUCCESS) goto errors;
	// Sigfox EP key register.
	for (idx=0 ; idx<AES_BLOCK_SIZE ; idx++) {
		// Read byte.
		nvm_status = NVM_read_byte((NVM_ADDRESS_SIGFOX_DEVICE_KEY + idx), &(sigfox_ep_tab[idx]));
		NVM_status_check(NODE_ERROR_BASE_NVM);
	}
	// Write registers.
	status = NODE_write_byte_array(NODE_REQUEST_SOURCE_INTERNAL, UHFM_REG_ADDR_SIGFOX_EP_ID, (uint8_t*) sigfox_ep_tab, AES_BLOCK_SIZE);
	if (status != NODE_SUCCESS) goto errors;
	// Load default values.
	status = NODE_write_register(NODE_REQUEST_SOURCE_INTERNAL, UHFM_REG_ADDR_SIGFOX_EP_CONFIGURATION_0, DINFOX_REG_MASK_ALL, UHFM_REG_SIGFOX_EP_CONFIGURATION_0_DEFAULT_VALUE);
	if (status != NODE_SUCCESS) goto errors;
	status = NODE_write_register(NODE_REQUEST_SOURCE_INTERNAL, UHFM_REG_ADDR_SIGFOX_EP_CONFIGURATION_1, DINFOX_REG_MASK_ALL, UHFM_REG_SIGFOX_EP_CONFIGURATION_1_DEFAULT_VALUE);
	if (status != NODE_SUCCESS) goto errors;
	status = NODE_write_register(NODE_REQUEST_SOURCE_INTERNAL, UHFM_REG_ADDR_SIGFOX_EP_CONFIGURATION_2, DINFOX_REG_MASK_ALL, UHFM_REG_SIGFOX_EP_CONFIGURATION_2_DEFAULT_VALUE);
	if (status != NODE_SUCCESS) goto errors;
	status = NODE_write_register(NODE_REQUEST_SOURCE_INTERNAL, UHFM_REG_ADDR_RADIO_TEST_0, DINFOX_REG_MASK_ALL, UHFM_REG_RADIO_TEST_0_DEFAULT_VALUE);
	if (status != NODE_SUCCESS) goto errors;
	status = NODE_write_register(NODE_REQUEST_SOURCE_INTERNAL, UHFM_REG_ADDR_RADIO_TEST_1, DINFOX_REG_MASK_ALL, UHFM_REG_RADIO_TEST_1_DEFAULT_VALUE);
	if (status != NODE_SUCCESS) goto errors;
errors:
	return status;
}

/* UPDATE UHFM REGISTER.
 * @param reg_addr:	Address of the register to update.
 * @return status:	Function execution status.
 */
NODE_status_t UHFM_update_register(uint8_t reg_addr) {
	// Local variables.
	NODE_status_t status = NODE_SUCCESS;
	S2LP_status_t s2lp_status = S2LP_SUCCESS;
	int16_t rssi_dbm = 0;
	// Check address.
	switch (reg_addr) {
	case UHFM_REG_ADDR_RADIO_TEST_1:
		// RSSI.
		s2lp_status = S2LP_get_rssi(S2LP_RSSI_TYPE_RUN, &rssi_dbm);
		S2LP_status_check(NODE_ERROR_BASE_S2LP);
		// Write field.
		status = NODE_write_field(NODE_REQUEST_SOURCE_INTERNAL, UHFM_REG_ADDR_RADIO_TEST_1, UHFM_REG_RADIO_TEST_1_MASK_RSSI, (uint32_t) DINFOX_convert_dbm(rssi_dbm));
		if (status != NODE_SUCCESS) goto errors;
		break;
	default:
		// Nothing to do for other registers.
		break;
	}
errors:
	return status;
}

/* CHECK UHFM NODE ACTIONS.
 * @param reg_addr:	Address of the register to check.
 * @return status:	Function execution status.
 */
NODE_status_t UHFM_check_register(uint8_t reg_addr) {
	// Local variables.
	NODE_status_t status = NODE_SUCCESS;
	uint32_t bit = 0;
	// Check address.
	switch (reg_addr) {
	case UHFM_REG_ADDR_STATUS_CONTROL_1:
		// Read STRG bit.
		status = NODE_read_field(NODE_REQUEST_SOURCE_INTERNAL, UHFM_REG_ADDR_STATUS_CONTROL_1, UHFM_REG_STATUS_CONTROL_1_MASK_STRG, &bit);
		if (status != NODE_SUCCESS) goto errors;
		// Check bit.
		if (bit != 0) {
			// Send Sigfox message.
			status = _UHFM_strg_callback();
			if (status != NODE_SUCCESS) goto errors;
		}
		// Read TTRG bit.
		status = NODE_read_field(NODE_REQUEST_SOURCE_INTERNAL, UHFM_REG_ADDR_STATUS_CONTROL_1, UHFM_REG_STATUS_CONTROL_1_MASK_TTRG, &bit);
		if (status != NODE_SUCCESS) goto errors;
		// Check bit.
		if (bit != 0) {
			// Perform Sigfox test mode.
			status = _UHFM_ttrg_callback();
			if (status != NODE_SUCCESS) goto errors;
		}
		// Read DTRG bit.
		status = NODE_read_field(NODE_REQUEST_SOURCE_INTERNAL, UHFM_REG_ADDR_STATUS_CONTROL_1, UHFM_REG_STATUS_CONTROL_1_MASK_DTRG, &bit);
		if (status != NODE_SUCCESS) goto errors;
		// Check bit.
		if (bit != 0) {
			// Start downlink decoder.
			status = _UHFM_dtrg_callback();
			if (status != NODE_SUCCESS) goto errors;
		}
		// Read CWEN bit.
		status = NODE_read_field(NODE_REQUEST_SOURCE_INTERNAL, UHFM_REG_ADDR_STATUS_CONTROL_1, UHFM_REG_STATUS_CONTROL_1_MASK_CWEN, &bit);
		if (status != NODE_SUCCESS) goto errors;
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
		// Read RSSI bit.
		status = NODE_read_field(NODE_REQUEST_SOURCE_INTERNAL, UHFM_REG_ADDR_STATUS_CONTROL_1, UHFM_REG_STATUS_CONTROL_1_MASK_RSEN, &bit);
		if (status != NODE_SUCCESS) goto errors;
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
		break;
	default:
		// Nothing to do for other registers.
		break;
	}
errors:
	return status;
}

/* MEASURE TRIGGER CALLBACK.
 * @param:			None.
 * @return status:	Function execution status.
 */
NODE_status_t UHFM_mtrg_callback(void) {
	// Local variables.
	NODE_status_t status = NODE_SUCCESS;
	ADC_status_t adc1_status = ADC_SUCCESS;
	uint32_t vrf_mv = 0;
	uint32_t radio_test_0 = 0;
	uint32_t radio_test_1 = 0;
	// Check radio state.
	status = _UHFM_check_radio_state(0);
	if (status != NODE_SUCCESS) goto errors;
	// Save radio test registers.
	status = NODE_read_register(NODE_REQUEST_SOURCE_INTERNAL, UHFM_REG_ADDR_RADIO_TEST_0, &radio_test_0);
	if (status != NODE_SUCCESS) goto errors;
	status = NODE_read_register(NODE_REQUEST_SOURCE_INTERNAL, UHFM_REG_ADDR_RADIO_TEST_1, &radio_test_1);
	if (status != NODE_SUCCESS) goto errors;
	// Configure frequency and TX power for measure.
	status = NODE_write_field(NODE_REQUEST_SOURCE_INTERNAL, UHFM_REG_ADDR_RADIO_TEST_0, UHFM_REG_RADIO_TEST_0_MASK_RF_FREQUENCY, UHFM_ADC_MEASUREMENTS_RF_FREQUENCY_HZ);
	if (status != NODE_SUCCESS) goto errors;
	status = NODE_write_field(NODE_REQUEST_SOURCE_INTERNAL, UHFM_REG_ADDR_RADIO_TEST_1, UHFM_REG_RADIO_TEST_1_MASK_TX_POWER, (uint32_t) DINFOX_convert_dbm(UHFM_ADC_MEASUREMENTS_TX_POWER_DBM));
	if (status != NODE_SUCCESS) goto errors;
	// Start CW.
	status = _UHFM_cwen_callback(1);
	if (status != NODE_SUCCESS) goto errors;
	// Perform measurements in TX state.
	adc1_status = ADC1_perform_measurements();
	ADC1_status_check(NODE_ERROR_BASE_ADC);
	// Stop CW.
	status = _UHFM_cwen_callback(0);
	if (status != NODE_SUCCESS) goto errors;
	// VRF_TX.
	adc1_status = ADC1_get_data(ADC_DATA_INDEX_VRF_MV, &vrf_mv);
	ADC1_status_check(NODE_ERROR_BASE_ADC);
	status = NODE_write_field(NODE_REQUEST_SOURCE_INTERNAL, UHFM_REG_ADDR_ANALOG_DATA_1, UHFM_REG_ANALOG_DATA_1_MASK_VRF_TX, (uint32_t) DINFOX_convert_mv(vrf_mv));
	if (status != NODE_SUCCESS) goto errors;
	// Start RX.
	status = _UHFM_rsen_callback(1);
	if (status != NODE_SUCCESS) goto errors;
	// Perform measurements in RX state.
	adc1_status = ADC1_perform_measurements();
	ADC1_status_check(NODE_ERROR_BASE_ADC);
	// Stop RX.
	status = _UHFM_rsen_callback(0);
	if (status != NODE_SUCCESS) goto errors;
	// VRF_RX.
	adc1_status = ADC1_get_data(ADC_DATA_INDEX_VRF_MV, &vrf_mv);
	ADC1_status_check(NODE_ERROR_BASE_ADC);
	status = NODE_write_field(NODE_REQUEST_SOURCE_INTERNAL, UHFM_REG_ADDR_ANALOG_DATA_1, UHFM_REG_ANALOG_DATA_1_MASK_VRF_RX, (uint32_t) DINFOX_convert_mv(vrf_mv));
	if (status != NODE_SUCCESS) goto errors;
errors:
	// Restore radio test registers.
	NODE_write_register(NODE_REQUEST_SOURCE_INTERNAL, UHFM_REG_ADDR_RADIO_TEST_0, DINFOX_REG_MASK_ALL, radio_test_0);
	NODE_write_register(NODE_REQUEST_SOURCE_INTERNAL, UHFM_REG_ADDR_RADIO_TEST_1, DINFOX_REG_MASK_ALL, radio_test_1);
	return status;
}

#endif /* UHFM */

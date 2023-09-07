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

/*** UHFM local macros ***/

#define UHFM_REG_SIGFOX_EP_CONFIGURATION_0_DEFAULT_VALUE	0x00000EC0
#define UHFM_REG_SIGFOX_EP_CONFIGURATION_1_DEFAULT_VALUE	0x07D001F4
#define UHFM_REG_SIGFOX_EP_CONFIGURATION_2_DEFAULT_VALUE	0x00000001

#define UHFM_REG_RADIO_TEST_0_DEFAULT_VALUE					0x33AD5EC0
#define UHFM_REG_RADIO_TEST_1_DEFAULT_VALUE					0x000000BC

#define UHFM_NUMBER_OF_SIGFOX_RC							7

#define UHFM_ADC_MEASUREMENTS_RF_FREQUENCY_HZ				830000000
#define UHFM_ADC_MEASUREMENTS_TX_POWER_DBM					14
#define UHFM_ADC_RADIO_STABILIZATION_DELAY_MS				100

/*** UHFM local structures ***/

/*******************************************************************/
typedef union {
	struct {
		unsigned cwen : 1;
		unsigned rsen : 1;
	};
	uint8_t all;
} UHFM_flags_t;

/*** UHFM local global variables ***/

#ifdef UHFM
static UHFM_flags_t uhfm_flags;
#endif

/*** UHFM local functions ***/

#ifdef UHFM
/*******************************************************************/
#define _UHFM_sigfox_ep_addon_rfp_exit_error(void) { \
	if (sigfox_ep_addon_rfp_status != SIGFOX_EP_ADDON_RFP_API_SUCCESS) { \
		status = (NODE_ERROR_BASE_SIGFOX_EP_ADDON_RFP_API + sigfox_ep_addon_rfp_status); \
		goto errors; \
	} \
}
#endif

#ifdef UHFM
/*******************************************************************/
static void _UHFM_reset_analog_data(void) {
	// Local variables.
	uint32_t analog_data_1 = 0;
	uint32_t analog_data_1_mask = 0;
	// Reset fields to error value.
	DINFOX_write_field(&analog_data_1, &analog_data_1_mask, DINFOX_VOLTAGE_ERROR_VALUE, UHFM_REG_ANALOG_DATA_1_MASK_VRF_TX);
	DINFOX_write_field(&analog_data_1, &analog_data_1_mask, DINFOX_VOLTAGE_ERROR_VALUE, UHFM_REG_ANALOG_DATA_1_MASK_VRF_RX);
	NODE_write_register(NODE_REQUEST_SOURCE_INTERNAL, UHFM_REG_ADDR_ANALOG_DATA_1, analog_data_1_mask, analog_data_1);
}
#endif

#ifdef UHFM
/*******************************************************************/
static NODE_status_t _UHFM_is_radio_free(void) {
	// Local variables.
	NODE_status_t status = NODE_SUCCESS;
	POWER_status_t power_status = POWER_SUCCESS;
	uint8_t power_state = 0;
	// Get current power state.
	power_status = POWER_get_state(POWER_DOMAIN_RADIO, &power_state);
	POWER_exit_error(NODE_ERROR_BASE_POWER);
	// Compare state.
	if (power_state != 0) {
		status = NODE_ERROR_RADIO_STATE;
		goto errors;
	}
errors:
	return status;
}
#endif

#ifdef UHFM
/*******************************************************************/
static NODE_status_t _UHFM_strg_callback(void) {
	// Local variables.
	NODE_status_t status = NODE_SUCCESS;
	SIGFOX_EP_API_status_t sigfox_ep_api_status = SIGFOX_EP_API_SUCCESS;
	SIGFOX_EP_API_config_t lib_config;
	SIGFOX_EP_API_application_message_t application_message;
	SIGFOX_EP_API_control_message_t control_message;
	SIGFOX_EP_API_message_status_t message_status;
	uint32_t status_control_1 = 0;
	uint32_t status_control_1_mask = 0;
	uint32_t ep_config_0 = 0;
	uint32_t ep_config_2 = 0;
	sfx_u8 ul_payload[SIGFOX_UL_PAYLOAD_MAX_SIZE_BYTES];
	sfx_u8 ul_payload_size = 0;
	sfx_u8 dl_payload[SIGFOX_DL_PAYLOAD_SIZE_BYTES];
	sfx_s16 dl_rssi_dbm = 0;
	// Reset status.
	message_status.all = 0;
	// Read configuration registers.
	NODE_read_register(NODE_REQUEST_SOURCE_INTERNAL, UHFM_REG_ADDR_SIGFOX_EP_CONFIGURATION_0, &ep_config_0);
	NODE_read_register(NODE_REQUEST_SOURCE_INTERNAL, UHFM_REG_ADDR_SIGFOX_EP_CONFIGURATION_2, &ep_config_2);
	// Check radio state.
	status = _UHFM_is_radio_free();
	if (status != NODE_SUCCESS) goto errors;
	// Open library.
	lib_config.rc = &SIGFOX_RC1;
	sigfox_ep_api_status = SIGFOX_EP_API_open(&lib_config);
	SIGFOX_EP_API_check_status(NODE_ERROR_SIGFOX_EP_API);
	// Check control message flag.
	if (DINFOX_read_field(ep_config_2, UHFM_REG_SIGFOX_EP_CONFIGURATION_2_MASK_CMSG) == 0) {
		// Get payload size.
		ul_payload_size = (sfx_u8) DINFOX_read_field(ep_config_2, UHFM_REG_SIGFOX_EP_CONFIGURATION_2_MASK_UL_PAYLOAD_SIZE);
		// Read UL payload.
		NODE_read_byte_array(NODE_REQUEST_SOURCE_INTERNAL, UHFM_REG_ADDR_SIGFOX_UL_PAYLOAD_0, (uint8_t*) ul_payload, ul_payload_size);
		// Build message structure.
		application_message.common_parameters.number_of_frames = (sfx_u8) DINFOX_read_field(ep_config_0, UHFM_REG_SIGFOX_EP_CONFIGURATION_0_MASK_NFR);
		application_message.common_parameters.ul_bit_rate = (SIGFOX_ul_bit_rate_t) DINFOX_read_field(ep_config_0, UHFM_REG_SIGFOX_EP_CONFIGURATION_0_MASK_BR);
		application_message.common_parameters.ep_key_type = SIGFOX_EP_KEY_PRIVATE;
		application_message.type = (SIGFOX_application_message_type_t) DINFOX_read_field(ep_config_2, UHFM_REG_SIGFOX_EP_CONFIGURATION_2_MASK_MSGT);
		application_message.bidirectional_flag = (sfx_u8) DINFOX_read_field(ep_config_2, UHFM_REG_SIGFOX_EP_CONFIGURATION_2_MASK_BF);
		application_message.ul_payload = (sfx_u8*) ul_payload;
		application_message.ul_payload_size_bytes = ul_payload_size;
		// Send message.
		sigfox_ep_api_status = SIGFOX_EP_API_send_application_message(&application_message);
		SIGFOX_EP_API_check_status(NODE_ERROR_SIGFOX_EP_API);
		// Read message status.
		message_status = SIGFOX_EP_API_get_message_status();
		// Check bidirectional flag.
		if ((application_message.bidirectional_flag != 0) && (message_status.field.dl_frame != 0)) {
			// Read downlink data.
			sigfox_ep_api_status = SIGFOX_EP_API_get_dl_payload(dl_payload, SIGFOX_DL_PAYLOAD_SIZE_BYTES, &dl_rssi_dbm);
			SIGFOX_EP_API_check_status(NODE_ERROR_SIGFOX_EP_API);
			// Write DL payload registers and RSSI.
			NODE_write_byte_array(NODE_REQUEST_SOURCE_INTERNAL, UHFM_REG_ADDR_SIGFOX_DL_PAYLOAD_0, (uint8_t*) dl_payload, SIGFOX_DL_PAYLOAD_SIZE_BYTES);
			DINFOX_write_field(&status_control_1, &status_control_1_mask, (uint32_t) DINFOX_convert_dbm(dl_rssi_dbm), UHFM_REG_STATUS_CONTROL_1_MASK_DL_RSSI);
		}
	}
	else {
		control_message.common_parameters.number_of_frames = (sfx_u8) DINFOX_read_field(ep_config_0, UHFM_REG_SIGFOX_EP_CONFIGURATION_0_MASK_NFR);
		control_message.common_parameters.ul_bit_rate = (SIGFOX_ul_bit_rate_t) DINFOX_read_field(ep_config_0, UHFM_REG_SIGFOX_EP_CONFIGURATION_0_MASK_BR);
		control_message.common_parameters.ep_key_type = SIGFOX_EP_KEY_PRIVATE;
		control_message.type = SIGFOX_CONTROL_MESSAGE_TYPE_KEEP_ALIVE;
		// Send message.
		sigfox_ep_api_status = SIGFOX_EP_API_send_control_message(&control_message);
		SIGFOX_EP_API_check_status(NODE_ERROR_SIGFOX_EP_API);
		// Read message status.
		message_status = SIGFOX_EP_API_get_message_status();
	}
	// Close library.
	sigfox_ep_api_status = SIGFOX_EP_API_close();
	SIGFOX_EP_API_check_status(NODE_ERROR_SIGFOX_EP_API);
	// Update message status and clear flag.
	DINFOX_write_field(&status_control_1, &status_control_1_mask, (uint32_t) (message_status.all), UHFM_REG_STATUS_CONTROL_1_MASK_MESSAGE_STATUS);
	DINFOX_write_field(&status_control_1, &status_control_1_mask, 0, UHFM_REG_STATUS_CONTROL_1_MASK_STRG);
	// Write register.
	NODE_write_register(NODE_REQUEST_SOURCE_INTERNAL, UHFM_REG_ADDR_STATUS_CONTROL_1, status_control_1_mask, status_control_1);
	// Return status.
	return status;
errors:
	// Close library.
	SIGFOX_EP_API_close();
	// Update message status and clear flag.
	DINFOX_write_field(&status_control_1, &status_control_1_mask, (uint32_t) (message_status.all), UHFM_REG_STATUS_CONTROL_1_MASK_MESSAGE_STATUS);
	DINFOX_write_field(&status_control_1, &status_control_1_mask, 0, UHFM_REG_STATUS_CONTROL_1_MASK_STRG);
	// Write register.
	NODE_write_register(NODE_REQUEST_SOURCE_INTERNAL, UHFM_REG_ADDR_STATUS_CONTROL_1, status_control_1_mask, status_control_1);
	// Return status.
	return status;
}
#endif

#ifdef UHFM
/*******************************************************************/
static NODE_status_t _UHFM_ttrg_callback(void) {
	// Local variables.
	NODE_status_t status = NODE_SUCCESS;
	SIGFOX_EP_ADDON_RFP_API_status_t sigfox_ep_addon_rfp_status = SIGFOX_EP_ADDON_RFP_API_SUCCESS;
	SIGFOX_EP_ADDON_RFP_API_config_t addon_config;
	SIGFOX_EP_ADDON_RFP_API_test_mode_t test_mode;
	uint32_t ep_config_0 = 0;
	uint32_t status_control_1 = 0;
	uint32_t status_control_1_mask = 0;
	// Read configuration register.
	NODE_read_register(NODE_REQUEST_SOURCE_INTERNAL, UHFM_REG_ADDR_SIGFOX_EP_CONFIGURATION_0, &ep_config_0);
	// Check radio state.
	status = _UHFM_is_radio_free();
	if (status != NODE_SUCCESS) goto errors;
	// Open addon.
	addon_config.rc = &SIGFOX_RC1;
	sigfox_ep_addon_rfp_status = SIGFOX_EP_ADDON_RFP_API_open(&addon_config);
	_UHFM_sigfox_ep_addon_rfp_exit_error();
	// Call test mode function.
	test_mode.test_mode_reference = (SIGFOX_EP_ADDON_RFP_API_test_mode_reference_t) DINFOX_read_field(ep_config_0, UHFM_REG_SIGFOX_EP_CONFIGURATION_0_MASK_TEST_MODE);
	test_mode.ul_bit_rate = (SIGFOX_ul_bit_rate_t) DINFOX_read_field(ep_config_0, UHFM_REG_SIGFOX_EP_CONFIGURATION_0_MASK_BR);
	sigfox_ep_addon_rfp_status = SIGFOX_EP_ADDON_RFP_API_test_mode(&test_mode);
	_UHFM_sigfox_ep_addon_rfp_exit_error();
	// Close addon.
	sigfox_ep_addon_rfp_status = SIGFOX_EP_ADDON_RFP_API_close();
	_UHFM_sigfox_ep_addon_rfp_exit_error();
	// Clear flag.
	DINFOX_write_field(&status_control_1, &status_control_1_mask, 0b0, UHFM_REG_STATUS_CONTROL_1_MASK_TTRG);
	NODE_write_register(NODE_REQUEST_SOURCE_INTERNAL, UHFM_REG_ADDR_STATUS_CONTROL_1, status_control_1_mask, status_control_1);
	// Return status.
	return status;
errors:
	// Close addon.
	SIGFOX_EP_ADDON_RFP_API_close();
	// Clear flag.
	DINFOX_write_field(&status_control_1, &status_control_1_mask, 0b0, UHFM_REG_STATUS_CONTROL_1_MASK_TTRG);
	NODE_write_register(NODE_REQUEST_SOURCE_INTERNAL, UHFM_REG_ADDR_STATUS_CONTROL_1, status_control_1_mask, status_control_1);
	// Return status.
	return status;
}
#endif

#ifdef UHFM
/*******************************************************************/
static NODE_status_t _UHFM_cwen_callback(uint8_t state) {
	// Local variables.
	NODE_status_t status = NODE_SUCCESS;
	RF_API_status_t rf_api_status = RF_API_SUCCESS;
	RF_API_radio_parameters_t radio_params;
	S2LP_status_t s2lp_status = S2LP_SUCCESS;
	uint32_t radio_test_0 = 0;
	uint32_t radio_test_1 = 0;
	// Read RF frequency and CW power.
	NODE_read_register(NODE_REQUEST_SOURCE_INTERNAL, UHFM_REG_ADDR_RADIO_TEST_0, &radio_test_0);
	NODE_read_register(NODE_REQUEST_SOURCE_INTERNAL, UHFM_REG_ADDR_RADIO_TEST_1, &radio_test_1);
	// Radio configuration.
	radio_params.rf_mode = RF_API_MODE_TX;
	radio_params.frequency_hz = (sfx_u32) DINFOX_read_field(radio_test_0, UHFM_REG_RADIO_TEST_0_MASK_RF_FREQUENCY);
	radio_params.modulation = RF_API_MODULATION_NONE;
	radio_params.bit_rate_bps = 0;
	radio_params.tx_power_dbm_eirp = (sfx_s8) DINFOX_get_dbm((DINFOX_rf_power_representation_t) DINFOX_read_field(radio_test_1, UHFM_REG_RADIO_TEST_1_MASK_TX_POWER));
	radio_params.deviation_hz = 0;
	// Check state.
	if (state == 0) {
		// Stop CW.
		rf_api_status = RF_API_de_init();
		RF_API_check_status(NODE_ERROR_SIGFOX_RF_API);
		rf_api_status = RF_API_sleep();
		RF_API_check_status(NODE_ERROR_SIGFOX_RF_API);
	}
	else {
		// Check radio state.
		status = _UHFM_is_radio_free();
		if (status != NODE_SUCCESS) goto errors;
		// Wake-up radio.
		rf_api_status = RF_API_wake_up();
		RF_API_check_status(NODE_ERROR_SIGFOX_RF_API);
		// Init radio.
		rf_api_status = RF_API_init(&radio_params);
		RF_API_check_status(NODE_ERROR_SIGFOX_RF_API);
		// Start CW.
		s2lp_status = S2LP_send_command(S2LP_COMMAND_READY);
		S2LP_exit_error(NODE_ERROR_BASE_S2LP);
		s2lp_status = S2LP_wait_for_state(S2LP_STATE_READY);
		S2LP_exit_error(NODE_ERROR_BASE_S2LP);
		s2lp_status = S2LP_send_command(S2LP_COMMAND_TX);
		S2LP_exit_error(NODE_ERROR_BASE_S2LP);
	}
	return status;
errors:
	if (status != NODE_ERROR_RADIO_STATE) {
		// Stop radio.
		RF_API_de_init();
		RF_API_sleep();
		// Update local flag.
		uhfm_flags.cwen = 0;
	}
	return status;
}
#endif

#ifdef UHFM
/*******************************************************************/
static NODE_status_t _UHFM_rsen_callback(uint8_t state) {
	// Local variables.
	NODE_status_t status = NODE_SUCCESS;
	RF_API_status_t rf_api_status = RF_API_SUCCESS;
	RF_API_radio_parameters_t radio_params;
	S2LP_status_t s2lp_status = S2LP_SUCCESS;
	uint32_t radio_test_0 = 0;
	// Read RF frequency.
	NODE_read_register(NODE_REQUEST_SOURCE_INTERNAL, UHFM_REG_ADDR_RADIO_TEST_0, &radio_test_0);
	// Radio configuration.
	radio_params.rf_mode = RF_API_MODE_RX;
	radio_params.frequency_hz = (sfx_u32) DINFOX_read_field(radio_test_0, UHFM_REG_RADIO_TEST_0_MASK_RF_FREQUENCY);
	radio_params.modulation = RF_API_MODULATION_NONE;
	radio_params.bit_rate_bps = 0;
	radio_params.tx_power_dbm_eirp = 0;
	radio_params.deviation_hz = 0;
	// Check state.
	if (state == 0) {
		// Stop continuous listening.
		rf_api_status = RF_API_de_init();
		RF_API_check_status(NODE_ERROR_SIGFOX_RF_API);
		rf_api_status = RF_API_sleep();
		RF_API_check_status(NODE_ERROR_SIGFOX_RF_API);
	}
	else {
		// Check radio state.
		status = _UHFM_is_radio_free();
		if (status != NODE_SUCCESS) goto errors;
		// Wake-up radio.
		rf_api_status = RF_API_wake_up();
		RF_API_check_status(NODE_ERROR_SIGFOX_RF_API);
		// Init radio.
		rf_api_status = RF_API_init(&radio_params);
		RF_API_check_status(NODE_ERROR_SIGFOX_RF_API);
		// Start continuous listening.
		s2lp_status = S2LP_send_command(S2LP_COMMAND_READY);
		S2LP_exit_error(NODE_ERROR_BASE_S2LP);
		s2lp_status = S2LP_wait_for_state(S2LP_STATE_READY);
		S2LP_exit_error(NODE_ERROR_BASE_S2LP);
		s2lp_status = S2LP_send_command(S2LP_COMMAND_RX);
		S2LP_exit_error(NODE_ERROR_BASE_S2LP);
	}
	return status;
errors:
	if (status != NODE_ERROR_RADIO_STATE) {
		// Stop radio.
		RF_API_de_init();
		RF_API_sleep();
		// Update local flag.
		uhfm_flags.rsen = 0;
	}
	return status;
}
#endif

/*** UHFM functions ***/

#ifdef UHFM
/*******************************************************************/
void UHFM_init_registers(void) {
	// Local variables.
	uint8_t idx = 0;
	uint8_t sigfox_ep_tab[SIGFOX_EP_KEY_SIZE_BYTES];
	// Init flags.
	uhfm_flags.all = 0;
	// Sigfox EP ID register.
	for (idx=0 ; idx<SIGFOX_EP_ID_SIZE_BYTES ; idx++) {
		NVM_read_byte((NVM_ADDRESS_SIGFOX_EP_ID + idx), &(sigfox_ep_tab[idx]));
	}
	// Write registers.
	NODE_write_byte_array(NODE_REQUEST_SOURCE_INTERNAL, UHFM_REG_ADDR_SIGFOX_EP_ID, (uint8_t*) sigfox_ep_tab, SIGFOX_EP_ID_SIZE_BYTES);
	// Sigfox EP key register.
	for (idx=0 ; idx<SIGFOX_EP_KEY_SIZE_BYTES ; idx++) {
		NVM_read_byte((NVM_ADDRESS_SIGFOX_EP_KEY + idx), &(sigfox_ep_tab[idx]));
	}
	// Write registers.
	NODE_write_byte_array(NODE_REQUEST_SOURCE_INTERNAL, UHFM_REG_ADDR_SIGFOX_EP_KEY_0, (uint8_t*) sigfox_ep_tab, SIGFOX_EP_KEY_SIZE_BYTES);
	// Load default values.
	_UHFM_reset_analog_data();
	NODE_write_register(NODE_REQUEST_SOURCE_INTERNAL, UHFM_REG_ADDR_SIGFOX_EP_CONFIGURATION_0, DINFOX_REG_MASK_ALL, UHFM_REG_SIGFOX_EP_CONFIGURATION_0_DEFAULT_VALUE);
	NODE_write_register(NODE_REQUEST_SOURCE_INTERNAL, UHFM_REG_ADDR_SIGFOX_EP_CONFIGURATION_1, DINFOX_REG_MASK_ALL, UHFM_REG_SIGFOX_EP_CONFIGURATION_1_DEFAULT_VALUE);
	NODE_write_register(NODE_REQUEST_SOURCE_INTERNAL, UHFM_REG_ADDR_SIGFOX_EP_CONFIGURATION_2, DINFOX_REG_MASK_ALL, UHFM_REG_SIGFOX_EP_CONFIGURATION_2_DEFAULT_VALUE);
	NODE_write_register(NODE_REQUEST_SOURCE_INTERNAL, UHFM_REG_ADDR_RADIO_TEST_0, DINFOX_REG_MASK_ALL, UHFM_REG_RADIO_TEST_0_DEFAULT_VALUE);
	NODE_write_register(NODE_REQUEST_SOURCE_INTERNAL, UHFM_REG_ADDR_RADIO_TEST_1, DINFOX_REG_MASK_ALL, UHFM_REG_RADIO_TEST_1_DEFAULT_VALUE);
}
#endif

#ifdef UHFM
/*******************************************************************/
NODE_status_t UHFM_update_register(uint8_t reg_addr) {
	// Local variables.
	NODE_status_t status = NODE_SUCCESS;
	S2LP_status_t s2lp_status = S2LP_SUCCESS;
	POWER_status_t power_status = POWER_SUCCESS;
	int16_t rssi_dbm = 0;
	uint32_t reg_value = 0;
	uint32_t reg_mask = 0;
	uint8_t power_state = 0;
	// Check address.
	switch (reg_addr) {
	case UHFM_REG_ADDR_RADIO_TEST_1:
		// Check if S2LP is powered.
		power_status = POWER_get_state(POWER_DOMAIN_RADIO, &power_state);
		POWER_exit_error(NODE_ERROR_BASE_POWER);
		// Check radio state.
		if ((power_state != 0) && (uhfm_flags.rsen != 0)) {
			// Read RSSI.
			s2lp_status = S2LP_get_rssi(S2LP_RSSI_TYPE_RUN, &rssi_dbm);
			S2LP_exit_error(NODE_ERROR_BASE_S2LP);
			// Write RSSI.
			DINFOX_write_field(&reg_value, &reg_mask, (uint32_t) DINFOX_convert_dbm(rssi_dbm), UHFM_REG_RADIO_TEST_1_MASK_RSSI);
		}
		break;
	default:
		// Nothing to do for other registers.
		break;
	}
errors:
	NODE_write_register(NODE_REQUEST_SOURCE_INTERNAL, reg_addr, reg_mask, reg_value);
	return status;
}
#endif

#ifdef UHFM
/*******************************************************************/
NODE_status_t UHFM_check_register(uint8_t reg_addr) {
	// Local variables.
	NODE_status_t status = NODE_SUCCESS;
	uint32_t cwen = 0;
	uint32_t rsen = 0;
	uint32_t reg_value = 0;
	uint32_t new_reg_value = 0;
	uint32_t new_reg_mask = 0;
	// Read register.
	NODE_read_register(NODE_REQUEST_SOURCE_INTERNAL, reg_addr, &reg_value);
	// Check address.
	switch (reg_addr) {
	case UHFM_REG_ADDR_STATUS_CONTROL_1:
		// Read bits.
		cwen = DINFOX_read_field(reg_value, UHFM_REG_STATUS_CONTROL_1_MASK_CWEN);
		rsen = DINFOX_read_field(reg_value, UHFM_REG_STATUS_CONTROL_1_MASK_RSEN);
		// Read STRG bit.
		if (DINFOX_read_field(reg_value, UHFM_REG_STATUS_CONTROL_1_MASK_STRG) != 0) {
			// Send Sigfox message.
			status = _UHFM_strg_callback();
			if (status != NODE_SUCCESS) goto errors;
		}
		// Read TTRG bit.
		if (DINFOX_read_field(reg_value, UHFM_REG_STATUS_CONTROL_1_MASK_TTRG) != 0) {
			// Perform Sigfox test mode.
			status = _UHFM_ttrg_callback();
			if (status != NODE_SUCCESS) goto errors;
		}
		// Read CWEN bit.
		if (cwen != uhfm_flags.cwen) {
			// Start or stop CW.
			status = _UHFM_cwen_callback(cwen);
			if (status != NODE_SUCCESS) {
				// Clear request.
				DINFOX_write_field(&new_reg_value, &new_reg_mask, uhfm_flags.cwen, UHFM_REG_STATUS_CONTROL_1_MASK_CWEN);
				goto errors;
			}
			// Update local flag.
			uhfm_flags.cwen = cwen;
		}
		// Read RSEN bit.
		if (rsen != uhfm_flags.rsen) {
			// Start or stop RSSI measurement.
			status = _UHFM_rsen_callback(rsen);
			if (status != NODE_SUCCESS) {
				// Clear request.
				DINFOX_write_field(&new_reg_value, &new_reg_mask, uhfm_flags.rsen, UHFM_REG_STATUS_CONTROL_1_MASK_CWEN);
				goto errors;
			}
			// Update local flag.
			uhfm_flags.rsen = rsen;
		}
		break;
	default:
		// Nothing to do for other registers.
		break;
	}
errors:
	NODE_write_register(NODE_REQUEST_SOURCE_INTERNAL, reg_addr, new_reg_mask, new_reg_value);
	return status;
}
#endif

#ifdef UHFM
/*******************************************************************/
NODE_status_t UHFM_mtrg_callback(ADC_status_t* adc_status) {
	// Local variables.
	NODE_status_t status = NODE_SUCCESS;
	POWER_status_t power_status = POWER_SUCCESS;
	ADC_status_t adc1_status = ADC_SUCCESS;
	LPTIM_status_t lptim1_status = LPTIM_SUCCESS;
	uint32_t vrf_mv = 0;
	uint32_t radio_test_0_initial = 0;
	uint32_t radio_test_1_initial = 0;
	uint32_t radio_test_0;
	uint32_t radio_test_0_mask = 0;
	uint32_t radio_test_1;
	uint32_t radio_test_1_mask = 0;
	uint32_t analog_data_1 = 0;
	uint32_t analog_data_1_mask = 0;
	// Reset results.
	_UHFM_reset_analog_data();
	// Save radio test registers.
	NODE_read_register(NODE_REQUEST_SOURCE_INTERNAL, UHFM_REG_ADDR_RADIO_TEST_0, &radio_test_0_initial);
	NODE_read_register(NODE_REQUEST_SOURCE_INTERNAL, UHFM_REG_ADDR_RADIO_TEST_1, &radio_test_1_initial);
	// Configure frequency and TX power for measure.
	DINFOX_write_field(&radio_test_0, &radio_test_0_mask, UHFM_ADC_MEASUREMENTS_RF_FREQUENCY_HZ, UHFM_REG_RADIO_TEST_0_MASK_RF_FREQUENCY);
	DINFOX_write_field(&radio_test_1, &radio_test_1_mask, (uint32_t) DINFOX_convert_dbm(UHFM_ADC_MEASUREMENTS_TX_POWER_DBM), UHFM_REG_RADIO_TEST_1_MASK_TX_POWER);
	// Write registers.
	NODE_write_register(NODE_REQUEST_SOURCE_INTERNAL, UHFM_REG_ADDR_RADIO_TEST_0, radio_test_0, radio_test_0_mask);
	NODE_write_register(NODE_REQUEST_SOURCE_INTERNAL, UHFM_REG_ADDR_RADIO_TEST_1, radio_test_1, radio_test_1_mask);
	// Start CW.
	status = _UHFM_cwen_callback(1);
	if (status != NODE_SUCCESS) goto errors;
	lptim1_status = LPTIM1_delay_milliseconds(UHFM_ADC_RADIO_STABILIZATION_DELAY_MS, LPTIM_DELAY_MODE_SLEEP);
	LPTIM1_exit_error(NODE_ERROR_BASE_LPTIM1);
	// Perform analog measurements.
	power_status = POWER_enable(POWER_DOMAIN_ANALOG, LPTIM_DELAY_MODE_ACTIVE);
	POWER_exit_error(NODE_ERROR_BASE_POWER);
	adc1_status = ADC1_perform_measurements();
	ADC1_exit_error(NODE_ERROR_BASE_ADC1);
	power_status = POWER_disable(POWER_DOMAIN_ANALOG);
	POWER_exit_error(NODE_ERROR_BASE_POWER);
	// Stop CW.
	status = _UHFM_cwen_callback(0);
	if (status != NODE_SUCCESS) goto errors;
	// Check status.
	if (adc1_status == ADC_SUCCESS) {
		// VRF_TX.
		adc1_status = ADC1_get_data(ADC_DATA_INDEX_VRF_MV, &vrf_mv);
		ADC1_exit_error(NODE_ERROR_BASE_ADC1);
		if (adc1_status == ADC_SUCCESS) {
			DINFOX_write_field(&analog_data_1, &analog_data_1_mask, (uint32_t) DINFOX_convert_mv(vrf_mv), UHFM_REG_ANALOG_DATA_1_MASK_VRF_TX);
		}
	}
	// Start RX.
	status = _UHFM_rsen_callback(1);
	if (status != NODE_SUCCESS) goto errors;
	lptim1_status = LPTIM1_delay_milliseconds(UHFM_ADC_RADIO_STABILIZATION_DELAY_MS, LPTIM_DELAY_MODE_SLEEP);
	LPTIM1_exit_error(NODE_ERROR_BASE_LPTIM1);
	// Perform measurements in RX state.
	power_status = POWER_enable(POWER_DOMAIN_ANALOG, LPTIM_DELAY_MODE_ACTIVE);
	POWER_exit_error(NODE_ERROR_BASE_POWER);
	adc1_status = ADC1_perform_measurements();
	ADC1_exit_error(NODE_ERROR_BASE_ADC1);
	power_status = POWER_disable(POWER_DOMAIN_ANALOG);
	POWER_exit_error(NODE_ERROR_BASE_POWER);
	// Stop RX.
	status = _UHFM_rsen_callback(0);
	if (status != NODE_SUCCESS) goto errors;
	// Check status.
	if (adc1_status == ADC_SUCCESS) {
		// VRF_RX.
		adc1_status = ADC1_get_data(ADC_DATA_INDEX_VRF_MV, &vrf_mv);
		ADC1_exit_error(NODE_ERROR_BASE_ADC1);
		if (adc1_status == ADC_SUCCESS) {
			DINFOX_write_field(&analog_data_1, &analog_data_1_mask, (uint32_t) DINFOX_convert_mv(vrf_mv), UHFM_REG_ANALOG_DATA_1_MASK_VRF_RX);
		}
	}
	// Write register.
	NODE_write_register(NODE_REQUEST_SOURCE_INTERNAL, UHFM_REG_ADDR_ANALOG_DATA_1, analog_data_1_mask, analog_data_1);
	NODE_write_register(NODE_REQUEST_SOURCE_INTERNAL, UHFM_REG_ADDR_RADIO_TEST_0, DINFOX_REG_MASK_ALL, radio_test_0_initial);
	NODE_write_register(NODE_REQUEST_SOURCE_INTERNAL, UHFM_REG_ADDR_RADIO_TEST_1, DINFOX_REG_MASK_ALL, radio_test_1_initial);
errors:
	// Update ADC status.
	if (adc_status != NULL) {
		(*adc_status) = adc1_status;
	}
	return status;
}
#endif

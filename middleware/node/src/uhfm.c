/*
 * uhfm.c
 *
 *  Created on: 04 jun. 2023
 *      Author: ludo
 */

#include "uhfm.h"

#include "analog.h"
#include "aes.h"
#include "error.h"
#include "load.h"
#include "node.h"
#include "nvm.h"
#include "nvm_address.h"
#include "s2lp.h"
#include "swreg.h"
#include "una.h"
#include "manuf/mcu_api.h"
#include "manuf/rf_api.h"
#include "sigfox_ep_addon_rfp_api.h"
#include "sigfox_ep_api.h"
#include "sigfox_rc.h"
#include "sigfox_types.h"

#ifdef UHFM

/*** UHFM local macros ***/

#define UHFM_REGISTER_RADIO_TEST_0_DEFAULT_VALUE    0x33AD5EC0
#define UHFM_REGISTER_RADIO_TEST_1_DEFAULT_VALUE    0x000000BC

#define UHFM_ADC_MEASUREMENTS_RF_FREQUENCY_HZ       830000000
#define UHFM_ADC_RADIO_STABILIZATION_DELAY_MS       100

/*** UHFM local structures ***/

/*******************************************************************/
typedef union {
    struct {
        unsigned cwen :1;
        unsigned rsen :1;
    };
    uint8_t all;
} UHFM_flags_t;

/*** UHFM local global variables ***/

static UHFM_flags_t uhfm_flags = {
    .all = 0
};

/*** UHFM local functions ***/

/*******************************************************************/
#define _UHFM_sigfox_ep_addon_rfp_exit_error(void) { \
    if (sigfox_ep_addon_rfp_status != SIGFOX_EP_ADDON_RFP_API_SUCCESS) { \
        status = (NODE_ERROR_BASE_SIGFOX_EP_ADDON_RFP_API + sigfox_ep_addon_rfp_status); \
        goto errors; \
    } \
}

/*******************************************************************/
static void _UHFM_load_dynamic_configuration(void) {
    // Local variables.
    uint8_t reg_addr = 0;
    uint32_t reg_value = 0;
    uint32_t reg_mask = 0;
    // Load configuration registers from NVM.
    for (reg_addr = UHFM_REGISTER_ADDRESS_CONFIGURATION_0; reg_addr < UHFM_REGISTER_ADDRESS_STATUS_1; reg_addr++) {
        // Read NVM.
        NODE_read_nvm(reg_addr, &reg_value);
        // Write register.
        NODE_write_register(NODE_REQUEST_SOURCE_INTERNAL, reg_addr, reg_value, UNA_REGISTER_MASK_ALL);
    }
    // Override fields fixed by Sigfox library compilation flags.
    // TX power and RC.
    reg_value = 0;
    reg_mask = 0;
    SWREG_write_field(&reg_value, &reg_mask, UNA_convert_dbm(SIGFOX_EP_TX_POWER_DBM_EIRP), UHFM_REGISTER_CONFIGURATION_0_MASK_TX_POWER);
    SWREG_write_field(&reg_value, &reg_mask, 0b0000, UHFM_REGISTER_CONFIGURATION_0_MASK_RC);
    NODE_write_register(NODE_REQUEST_SOURCE_EXTERNAL, UHFM_REGISTER_ADDRESS_CONFIGURATION_0, reg_value, reg_mask);
    // TIFU and TCONF.
    reg_value = 0;
    reg_mask = 0;
    SWREG_write_field(&reg_value, &reg_mask, SIGFOX_EP_T_IFU_MS, UHFM_REGISTER_CONFIGURATION_1_MASK_TIFU);
    SWREG_write_field(&reg_value, &reg_mask, SIGFOX_EP_T_CONF_MS, UHFM_REGISTER_CONFIGURATION_1_MASK_TCONF);
    NODE_write_register(NODE_REQUEST_SOURCE_EXTERNAL, UHFM_REGISTER_ADDRESS_CONFIGURATION_1, reg_value, reg_mask);
}

/*******************************************************************/
static void _UHFM_reset_analog_data(void) {
    // Local variables.
    uint32_t reg_analog_data_1 = 0;
    uint32_t reg_analog_data_1_mask = 0;
    // Reset fields to error value.
    SWREG_write_field(&reg_analog_data_1, &reg_analog_data_1_mask, UNA_VOLTAGE_ERROR_VALUE, UHFM_REGISTER_ANALOG_DATA_1_MASK_VRF_TX);
    SWREG_write_field(&reg_analog_data_1, &reg_analog_data_1_mask, UNA_VOLTAGE_ERROR_VALUE, UHFM_REGISTER_ANALOG_DATA_1_MASK_VRF_RX);
    NODE_write_register(NODE_REQUEST_SOURCE_INTERNAL, UHFM_REGISTER_ADDRESS_ANALOG_DATA_1, reg_analog_data_1, reg_analog_data_1_mask);
}

/*******************************************************************/
static NODE_status_t _UHFM_is_radio_free(void) {
    // Local variables.
    NODE_status_t status = NODE_SUCCESS;
    // Compare state.
    if (POWER_get_state(POWER_DOMAIN_RADIO) != 0) {
        status = NODE_ERROR_RADIO_STATE;
        goto errors;
    }
errors:
    return status;
}

/*******************************************************************/
static NODE_status_t _UHFM_strg_callback(void) {
    // Local variables.
    NODE_status_t status = NODE_SUCCESS;
    SIGFOX_EP_API_status_t sigfox_ep_api_status = SIGFOX_EP_API_SUCCESS;
    MCU_API_status_t mcu_api_status = MCU_API_SUCCESS;
    SIGFOX_EP_API_config_t lib_config;
    SIGFOX_EP_API_application_message_t application_message;
#ifdef SIGFOX_EP_CONTROL_KEEP_ALIVE_MESSAGE
    SIGFOX_EP_API_control_message_t control_message;
#endif
    SIGFOX_EP_API_message_status_t message_status;
    uint32_t reg_status_1 = 0;
    uint32_t reg_status_1_mask = 0;
    uint32_t reg_config_0 = 0;
    uint32_t reg_control_1 = 0;
    sfx_bool bidirectional_flag = 0;
    sfx_u8 ul_payload[SIGFOX_UL_PAYLOAD_MAX_SIZE_BYTES];
    sfx_u8 ul_payload_size = 0;
    sfx_u8 dl_payload[SIGFOX_DL_PAYLOAD_SIZE_BYTES];
    sfx_s16 dl_rssi_dbm = 0;
    sfx_u8 nvm_data[SIGFOX_NVM_DATA_SIZE_BYTES];
    uint32_t message_counter = 0;
    // Reset status.
    message_status.all = 0;
    // Read configuration registers.
    NODE_read_register(NODE_REQUEST_SOURCE_INTERNAL, UHFM_REGISTER_ADDRESS_CONFIGURATION_0, &reg_config_0);
    NODE_read_register(NODE_REQUEST_SOURCE_INTERNAL, UHFM_REGISTER_ADDRESS_CONTROL_1, &reg_control_1);
    // Check radio state.
    status = _UHFM_is_radio_free();
    if (status != NODE_SUCCESS) goto errors;
    // Open library.
    lib_config.rc = &SIGFOX_RC1;
    sigfox_ep_api_status = SIGFOX_EP_API_open(&lib_config);
    SIGFOX_EP_API_check_status(NODE_ERROR_SIGFOX_EP_API);
#ifdef SIGFOX_EP_CONTROL_KEEP_ALIVE_MESSAGE
    // Check control message flag.
    if (SWREG_read_field(reg_control_1, UHFM_REGISTER_CONTROL_1_MASK_CMSG) == 0) {
#endif
        // Get payload size.
        ul_payload_size = (sfx_u8) SWREG_read_field(reg_control_1, UHFM_REGISTER_CONTROL_1_MASK_UL_PAYLOAD_SIZE);
        // Read UL payload.
        NODE_read_byte_array(NODE_REQUEST_SOURCE_INTERNAL, UHFM_REGISTER_ADDRESS_UL_PAYLOAD_0, (uint8_t*) ul_payload, ul_payload_size);
        // Update bidirectional flag.
        bidirectional_flag = (sfx_bool) SWREG_read_field(reg_control_1, UHFM_REGISTER_CONTROL_1_MASK_BF);
        // Read current message counter.
        if (bidirectional_flag == SIGFOX_TRUE) {
            // Read memory.
            mcu_api_status = MCU_API_get_nvm((sfx_u8*) nvm_data, SIGFOX_NVM_DATA_SIZE_BYTES);
            MCU_API_check_status(NODE_ERROR_SIGFOX_MCU_API);
            // Compute message counter.
            message_counter = (sfx_u32) (message_counter | ((((sfx_u32) nvm_data[SIGFOX_NVM_DATA_INDEX_MESSAGE_COUNTER_MSB]) << 8) & 0xFF00));
            message_counter = (sfx_u32) (message_counter | ((((sfx_u32) nvm_data[SIGFOX_NVM_DATA_INDEX_MESSAGE_COUNTER_LSB]) << 0) & 0x00FF));
        }
        // Build message structure.
        application_message.common_parameters.number_of_frames = (sfx_u8) SWREG_read_field(reg_config_0, UHFM_REGISTER_CONFIGURATION_0_MASK_NFR);
        application_message.common_parameters.ul_bit_rate = (SIGFOX_ul_bit_rate_t) SWREG_read_field(reg_config_0, UHFM_REGISTER_CONFIGURATION_0_MASK_BR);
#ifdef SIGFOX_EP_PUBLIC_KEY_CAPABLE
        application_message.common_parameters.ep_key_type = SIGFOX_EP_KEY_PRIVATE;
#endif
        application_message.type = (SIGFOX_application_message_type_t) SWREG_read_field(reg_control_1, UHFM_REGISTER_CONTROL_1_MASK_MSGT);
        application_message.bidirectional_flag = bidirectional_flag;
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
            NODE_write_byte_array(NODE_REQUEST_SOURCE_INTERNAL, UHFM_REGISTER_ADDRESS_DL_PAYLOAD_0, (uint8_t*) dl_payload, SIGFOX_DL_PAYLOAD_SIZE_BYTES);
            SWREG_write_field(&reg_status_1, &reg_status_1_mask, UNA_convert_dbm(dl_rssi_dbm), UHFM_REGISTER_STATUS_1_MASK_DL_RSSI);
        }
#ifdef SIGFOX_EP_CONTROL_KEEP_ALIVE_MESSAGE
    }
    else {
        control_message.common_parameters.number_of_frames = (sfx_u8) SWREG_read_field(reg_config_0, UHFM_REGISTER_CONFIGURATION_0_MASK_NFR);
        control_message.common_parameters.ul_bit_rate = (SIGFOX_ul_bit_rate_t) SWREG_read_field(reg_config_0, UHFM_REGISTER_CONFIGURATION_0_MASK_BR);
        control_message.common_parameters.ep_key_type = SIGFOX_EP_KEY_PRIVATE;
        control_message.type = SIGFOX_CONTROL_MESSAGE_TYPE_KEEP_ALIVE;
        // Send message.
        sigfox_ep_api_status = SIGFOX_EP_API_send_control_message(&control_message);
        SIGFOX_EP_API_check_status(NODE_ERROR_SIGFOX_EP_API);
        // Read message status.
        message_status = SIGFOX_EP_API_get_message_status();
    }
#endif
errors:
    // Close library.
    SIGFOX_EP_API_close();
    // Update message status.
    SWREG_write_field(&reg_status_1, &reg_status_1_mask, (uint32_t) (message_status.all), UHFM_REGISTER_STATUS_1_MASK_MESSAGE_STATUS);
    // Update bidirectional message counter.
    if ((bidirectional_flag == SIGFOX_TRUE) && (message_status.all != 0)) {
        SWREG_write_field(&reg_status_1, &reg_status_1_mask, (message_counter + 1), UHFM_REGISTER_STATUS_1_MASK_BIDIRECTIONAL_MC);
    }
    NODE_write_register(NODE_REQUEST_SOURCE_INTERNAL, UHFM_REGISTER_ADDRESS_STATUS_1, reg_status_1, reg_status_1_mask);
    // Return status.
    return status;
}

/*******************************************************************/
static NODE_status_t _UHFM_ttrg_callback(void) {
    // Local variables.
    NODE_status_t status = NODE_SUCCESS;
    SIGFOX_EP_ADDON_RFP_API_status_t sigfox_ep_addon_rfp_status = SIGFOX_EP_ADDON_RFP_API_SUCCESS;
    SIGFOX_EP_ADDON_RFP_API_config_t addon_config;
    SIGFOX_EP_ADDON_RFP_API_test_mode_t test_mode;
    uint32_t reg_config_0 = 0;
    uint32_t reg_control_1 = 0;
    // Read configuration registers.
    NODE_read_register(NODE_REQUEST_SOURCE_INTERNAL, UHFM_REGISTER_ADDRESS_CONFIGURATION_0, &reg_config_0);
    NODE_read_register(NODE_REQUEST_SOURCE_INTERNAL, UHFM_REGISTER_ADDRESS_CONTROL_1, &reg_control_1);
    // Check radio state.
    status = _UHFM_is_radio_free();
    if (status != NODE_SUCCESS) goto errors;
    // Open addon.
    addon_config.rc = &SIGFOX_RC1;
    sigfox_ep_addon_rfp_status = SIGFOX_EP_ADDON_RFP_API_open(&addon_config);
    _UHFM_sigfox_ep_addon_rfp_exit_error();
    // Call test mode function.
    test_mode.test_mode_reference = (SIGFOX_EP_ADDON_RFP_API_test_mode_reference_t) SWREG_read_field(reg_control_1, UHFM_REGISTER_CONTROL_1_MASK_RFP_TEST_MODE);
    test_mode.ul_bit_rate = (SIGFOX_ul_bit_rate_t) SWREG_read_field(reg_config_0, UHFM_REGISTER_CONFIGURATION_0_MASK_BR);
    sigfox_ep_addon_rfp_status = SIGFOX_EP_ADDON_RFP_API_test_mode(&test_mode);
    _UHFM_sigfox_ep_addon_rfp_exit_error();
errors:
    // Close addon.
    SIGFOX_EP_ADDON_RFP_API_close();
    // Return status.
    return status;
}

/*******************************************************************/
static NODE_status_t _UHFM_cwen_callback(uint8_t state) {
    // Local variables.
    NODE_status_t status = NODE_SUCCESS;
    RF_API_status_t rf_api_status = RF_API_SUCCESS;
    RF_API_radio_parameters_t radio_params;
    uint32_t reg_radio_test_0 = 0;
    uint32_t reg_radio_test_1 = 0;
    // Read RF frequency and CW power.
    NODE_read_register(NODE_REQUEST_SOURCE_INTERNAL, UHFM_REGISTER_ADDRESS_RADIO_TEST_0, &reg_radio_test_0);
    NODE_read_register(NODE_REQUEST_SOURCE_INTERNAL, UHFM_REGISTER_ADDRESS_RADIO_TEST_1, &reg_radio_test_1);
    // Radio configuration.
    radio_params.rf_mode = RF_API_MODE_TX;
    radio_params.frequency_hz = (sfx_u32) SWREG_read_field(reg_radio_test_0, UHFM_REGISTER_RADIO_TEST_0_MASK_RF_FREQUENCY);
    radio_params.modulation = RF_API_MODULATION_NONE;
    radio_params.bit_rate_bps = 0;
    radio_params.tx_power_dbm_eirp = (sfx_s8) UNA_get_dbm(SWREG_read_field(reg_radio_test_1, UHFM_REGISTER_RADIO_TEST_1_MASK_TX_POWER));
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
        // Init radio.
        rf_api_status = RF_API_wake_up();
        RF_API_check_status(NODE_ERROR_SIGFOX_RF_API);
        rf_api_status = RF_API_init(&radio_params);
        RF_API_check_status(NODE_ERROR_SIGFOX_RF_API);
        // Start CW.
        rf_api_status = RF_API_start_continuous_wave();
        RF_API_check_status(NODE_ERROR_SIGFOX_RF_API);
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

/*******************************************************************/
static NODE_status_t _UHFM_rsen_callback(uint8_t state) {
    // Local variables.
    NODE_status_t status = NODE_SUCCESS;
    RF_API_status_t rf_api_status = RF_API_SUCCESS;
    RF_API_radio_parameters_t radio_params;
    S2LP_status_t s2lp_status = S2LP_SUCCESS;
    uint32_t reg_radio_test_0 = 0;
    // Read RF frequency.
    NODE_read_register(NODE_REQUEST_SOURCE_INTERNAL, UHFM_REGISTER_ADDRESS_RADIO_TEST_0, &reg_radio_test_0);
    // Radio configuration.
    radio_params.rf_mode = RF_API_MODE_RX;
    radio_params.frequency_hz = (sfx_u32) SWREG_read_field(reg_radio_test_0, UHFM_REGISTER_RADIO_TEST_0_MASK_RF_FREQUENCY);
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
        // Init radio.
        rf_api_status = RF_API_wake_up();
        RF_API_check_status(NODE_ERROR_SIGFOX_RF_API);
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

/*** UHFM functions ***/

/*******************************************************************/
NODE_status_t UHFM_init_registers(void) {
    // Local variables.
    NODE_status_t status = NODE_SUCCESS;
    uint8_t idx = 0;
    uint8_t sigfox_ep_tab[SIGFOX_EP_KEY_SIZE_BYTES];
#ifdef XM_NVM_FACTORY_RESET
    uint32_t reg_value = 0;
    uint32_t reg_mask = 0;
    // TX power, NFR, bit rate and RC.
    SWREG_write_field(&reg_value, &reg_mask, UNA_convert_dbm(SIGFOX_EP_TX_POWER_DBM_EIRP), UHFM_REGISTER_CONFIGURATION_0_MASK_TX_POWER);
    SWREG_write_field(&reg_value, &reg_mask, 0b11, UHFM_REGISTER_CONFIGURATION_0_MASK_NFR);
    SWREG_write_field(&reg_value, &reg_mask, 0b01, UHFM_REGISTER_CONFIGURATION_0_MASK_BR);
    SWREG_write_field(&reg_value, &reg_mask, 0b0000, UHFM_REGISTER_CONFIGURATION_0_MASK_RC);
    NODE_write_register(NODE_REQUEST_SOURCE_EXTERNAL, UHFM_REGISTER_ADDRESS_CONFIGURATION_0, reg_value, reg_mask);
    // TCONF and TIFU.
    reg_value = 0;
    reg_mask = 0;
    SWREG_write_field(&reg_value, &reg_mask, SIGFOX_EP_T_IFU_MS, UHFM_REGISTER_CONFIGURATION_1_MASK_TIFU);
    SWREG_write_field(&reg_value, &reg_mask, SIGFOX_EP_T_CONF_MS, UHFM_REGISTER_CONFIGURATION_1_MASK_TCONF);
    NODE_write_register(NODE_REQUEST_SOURCE_EXTERNAL, UHFM_REGISTER_ADDRESS_CONFIGURATION_1, reg_value, reg_mask);
#endif
    // Init flags.
    uhfm_flags.all = 0;
    // Sigfox EP ID register.
    for (idx = 0; idx < SIGFOX_EP_ID_SIZE_BYTES; idx++) {
        NVM_read_byte((NVM_ADDRESS_SIGFOX_EP_ID + idx), &(sigfox_ep_tab[idx]));
    }
    NODE_write_byte_array(NODE_REQUEST_SOURCE_INTERNAL, UHFM_REGISTER_ADDRESS_EP_ID, (uint8_t*) sigfox_ep_tab, SIGFOX_EP_ID_SIZE_BYTES);
    // Sigfox EP key registers.
    for (idx = 0; idx < SIGFOX_EP_KEY_SIZE_BYTES; idx++) {
        NVM_read_byte((NVM_ADDRESS_SIGFOX_EP_KEY + idx), &(sigfox_ep_tab[idx]));
    }
    NODE_write_byte_array(NODE_REQUEST_SOURCE_INTERNAL, UHFM_REGISTER_ADDRESS_EP_KEY_0, (uint8_t*) sigfox_ep_tab, SIGFOX_EP_KEY_SIZE_BYTES);
    // Load default values.
    _UHFM_load_dynamic_configuration();
    _UHFM_reset_analog_data();
    NODE_write_register(NODE_REQUEST_SOURCE_INTERNAL, UHFM_REGISTER_ADDRESS_RADIO_TEST_0, UHFM_REGISTER_RADIO_TEST_0_DEFAULT_VALUE, UNA_REGISTER_MASK_ALL);
    NODE_write_register(NODE_REQUEST_SOURCE_INTERNAL, UHFM_REGISTER_ADDRESS_RADIO_TEST_1, UHFM_REGISTER_RADIO_TEST_1_DEFAULT_VALUE, UNA_REGISTER_MASK_ALL);
    return status;
}

/*******************************************************************/
NODE_status_t UHFM_update_register(uint8_t reg_addr) {
    // Local variables.
    NODE_status_t status = NODE_SUCCESS;
    S2LP_status_t s2lp_status = S2LP_SUCCESS;
    int16_t rssi_dbm = 0;
    uint32_t reg_value = 0;
    uint32_t reg_mask = 0;
    // Check address.
    switch (reg_addr) {
    case UHFM_REGISTER_ADDRESS_RADIO_TEST_1:
        // Check radio state.
        if ((POWER_get_state(POWER_DOMAIN_RADIO) != 0) && (uhfm_flags.rsen != 0)) {
            // Read RSSI.
            s2lp_status = S2LP_get_rssi(S2LP_RSSI_TYPE_RUN, &rssi_dbm);
            S2LP_exit_error(NODE_ERROR_BASE_S2LP);
            // Write RSSI.
            SWREG_write_field(&reg_value, &reg_mask, UNA_convert_dbm(rssi_dbm), UHFM_REGISTER_RADIO_TEST_1_MASK_RSSI);
        }
        break;
    default:
        // Nothing to do for other registers.
        break;
    }
errors:
    NODE_write_register(NODE_REQUEST_SOURCE_INTERNAL, reg_addr, reg_value, reg_mask);
    return status;
}

/*******************************************************************/
NODE_status_t UHFM_check_register(uint8_t reg_addr, uint32_t reg_mask) {
    // Local variables.
    NODE_status_t status = NODE_SUCCESS;
    uint32_t cwen = 0;
    uint32_t rsen = 0;
    uint32_t reg_value = 0;
    uint32_t new_reg_value = 0;
    uint32_t new_reg_mask = 0;
    // Read register.
    status = NODE_read_register(NODE_REQUEST_SOURCE_INTERNAL, reg_addr, &reg_value);
    if (status != NODE_SUCCESS) goto errors;
    // Check address.
    switch (reg_addr) {
    case UHFM_REGISTER_ADDRESS_CONFIGURATION_0:
    case UHFM_REGISTER_ADDRESS_CONFIGURATION_1:
        // Store new value in NVM.
        if (reg_mask != 0) {
            NODE_write_nvm(reg_addr, reg_value);
        }
        break;
    case UHFM_REGISTER_ADDRESS_CONTROL_1:
        // STRG.
        if ((reg_mask & UHFM_REGISTER_CONTROL_1_MASK_STRG) != 0) {
            // Read bit.
            if (SWREG_read_field(reg_value, UHFM_REGISTER_CONTROL_1_MASK_STRG) != 0) {
                // Clear request.
                NODE_write_register(NODE_REQUEST_SOURCE_INTERNAL, UHFM_REGISTER_ADDRESS_CONTROL_1, 0b0, UHFM_REGISTER_CONTROL_1_MASK_STRG);
                // Send Sigfox message.
                status = _UHFM_strg_callback();
                if (status != NODE_SUCCESS) goto errors;
            }
        }
        // TTRG.
        if ((reg_mask & UHFM_REGISTER_CONTROL_1_MASK_TTRG)) {
            // Read bit.
            if (SWREG_read_field(reg_value, UHFM_REGISTER_CONTROL_1_MASK_TTRG) != 0) {
                // Clear request.
                NODE_write_register(NODE_REQUEST_SOURCE_INTERNAL, UHFM_REGISTER_ADDRESS_CONTROL_1, 0b0, UHFM_REGISTER_CONTROL_1_MASK_TTRG);
                // Perform Sigfox test mode.
                status = _UHFM_ttrg_callback();
                if (status != NODE_SUCCESS) goto errors;
            }
        }
        // CWEN.
        if ((reg_mask & UHFM_REGISTER_CONTROL_1_MASK_CWEN) != 0) {
            // Read bit.
            cwen = SWREG_read_field(reg_value, UHFM_REGISTER_CONTROL_1_MASK_CWEN);
            // Compare to current state.
            if (cwen != uhfm_flags.cwen) {
                // Start or stop CW.
                status = _UHFM_cwen_callback(cwen);
                if (status != NODE_SUCCESS) {
                    // Clear request.
                    SWREG_write_field(&new_reg_value, &new_reg_mask, uhfm_flags.cwen, UHFM_REGISTER_CONTROL_1_MASK_CWEN);
                    goto errors;
                }
                // Update local flag.
                uhfm_flags.cwen = cwen;
            }
        }
        // RSEN.
        if ((reg_mask & UHFM_REGISTER_CONTROL_1_MASK_RSEN) != 0) {
            // Read bit.
            rsen = SWREG_read_field(reg_value, UHFM_REGISTER_CONTROL_1_MASK_RSEN);
            // Compare to current state.
            if (rsen != uhfm_flags.rsen) {
                // Start or stop RSSI measurement.
                status = _UHFM_rsen_callback(rsen);
                if (status != NODE_SUCCESS) {
                    // Clear request.
                    SWREG_write_field(&new_reg_value, &new_reg_mask, uhfm_flags.rsen, UHFM_REGISTER_CONTROL_1_MASK_CWEN);
                    goto errors;
                }
                // Update local flag.
                uhfm_flags.rsen = rsen;
            }
        }
        break;
    default:
        // Nothing to do for other registers.
        break;
    }
errors:
    NODE_write_register(NODE_REQUEST_SOURCE_INTERNAL, reg_addr, new_reg_value, new_reg_mask);
    return status;
}

/*******************************************************************/
NODE_status_t UHFM_mtrg_callback(void) {
    // Local variables.
    NODE_status_t status = NODE_SUCCESS;
    ANALOG_status_t analog_status = ANALOG_SUCCESS;
    LPTIM_status_t lptim_status = LPTIM_SUCCESS;
    int32_t vrf_mv = 0;
    uint32_t reg_radio_test_0_initial = 0;
    uint32_t reg_radio_test_1_initial = 0;
    uint32_t reg_radio_test_0;
    uint32_t reg_radio_test_0_mask = 0;
    uint32_t reg_radio_test_1;
    uint32_t reg_radio_test_1_mask = 0;
    uint32_t reg_analog_data_1 = 0;
    uint32_t reg_analog_data_1_mask = 0;
    // Reset results.
    _UHFM_reset_analog_data();
    // Save radio test registers.
    NODE_read_register(NODE_REQUEST_SOURCE_INTERNAL, UHFM_REGISTER_ADDRESS_RADIO_TEST_0, &reg_radio_test_0_initial);
    NODE_read_register(NODE_REQUEST_SOURCE_INTERNAL, UHFM_REGISTER_ADDRESS_RADIO_TEST_1, &reg_radio_test_1_initial);
    // Configure frequency and TX power for measure.
    SWREG_write_field(&reg_radio_test_0, &reg_radio_test_0_mask, UHFM_ADC_MEASUREMENTS_RF_FREQUENCY_HZ, UHFM_REGISTER_RADIO_TEST_0_MASK_RF_FREQUENCY);
    SWREG_write_field(&reg_radio_test_1, &reg_radio_test_1_mask, UNA_convert_dbm(SIGFOX_EP_TX_POWER_DBM_EIRP), UHFM_REGISTER_RADIO_TEST_1_MASK_TX_POWER);
    // Write registers.
    NODE_write_register(NODE_REQUEST_SOURCE_INTERNAL, UHFM_REGISTER_ADDRESS_RADIO_TEST_0, reg_radio_test_0_mask, reg_radio_test_0);
    NODE_write_register(NODE_REQUEST_SOURCE_INTERNAL, UHFM_REGISTER_ADDRESS_RADIO_TEST_1, reg_radio_test_1_mask, reg_radio_test_1);
    // Start CW.
    status = _UHFM_cwen_callback(1);
    if (status != NODE_SUCCESS) goto errors;
    lptim_status = LPTIM_delay_milliseconds(UHFM_ADC_RADIO_STABILIZATION_DELAY_MS, LPTIM_DELAY_MODE_SLEEP);
    LPTIM_exit_error(NODE_ERROR_BASE_LPTIM);
    // VRF_TX.
    analog_status = ANALOG_convert_channel(ANALOG_CHANNEL_VRF_MV, &vrf_mv);
    ANALOG_exit_error(NODE_ERROR_BASE_ANALOG);
    // Write field.
    SWREG_write_field(&reg_analog_data_1, &reg_analog_data_1_mask, UNA_convert_mv(vrf_mv), UHFM_REGISTER_ANALOG_DATA_1_MASK_VRF_TX);
    // Stop CW.
    status = _UHFM_cwen_callback(0);
    if (status != NODE_SUCCESS) goto errors;
    // Start RX.
    status = _UHFM_rsen_callback(1);
    if (status != NODE_SUCCESS) goto errors;
    lptim_status = LPTIM_delay_milliseconds(UHFM_ADC_RADIO_STABILIZATION_DELAY_MS, LPTIM_DELAY_MODE_SLEEP);
    LPTIM_exit_error(NODE_ERROR_BASE_LPTIM);
    // VRF_RX.
    analog_status = ANALOG_convert_channel(ANALOG_CHANNEL_VRF_MV, &vrf_mv);
    ANALOG_exit_error(NODE_ERROR_BASE_ANALOG);
    // Write field.
    SWREG_write_field(&reg_analog_data_1, &reg_analog_data_1_mask, UNA_convert_mv(vrf_mv), UHFM_REGISTER_ANALOG_DATA_1_MASK_VRF_RX);
    // Stop RX.
    status = _UHFM_rsen_callback(0);
    if (status != NODE_SUCCESS) goto errors;
    // Write registers.
    NODE_write_register(NODE_REQUEST_SOURCE_INTERNAL, UHFM_REGISTER_ADDRESS_ANALOG_DATA_1, reg_analog_data_1, reg_analog_data_1_mask);
    NODE_write_register(NODE_REQUEST_SOURCE_INTERNAL, UHFM_REGISTER_ADDRESS_RADIO_TEST_0, reg_radio_test_0_initial, UNA_REGISTER_MASK_ALL);
    NODE_write_register(NODE_REQUEST_SOURCE_INTERNAL, UHFM_REGISTER_ADDRESS_RADIO_TEST_1, reg_radio_test_1_initial, UNA_REGISTER_MASK_ALL);
errors:
    return status;
}

#endif /* UHFM */

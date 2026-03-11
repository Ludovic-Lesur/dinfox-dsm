/*
 * bcm.c
 *
 *  Created on: 27 mar. 2025
 *      Author: Ludo
 */

#include "bcm.h"

#ifdef BCM

#include "analog.h"
#include "bcm_registers.h"
#include "dsm_flags.h"
#include "error.h"
#include "load.h"
#include "node_register.h"
#include "node_status.h"
#include "swreg.h"
#include "rtc.h"
#include "types.h"
#include "una.h"

/*** BCM local macros ***/

#define BCM_CHARGE_SOURCE_VOLTAGE_TH_MV_MAX         60000
#define BCM_CHARGE_SOURCE_VOLTAGE_TH_MV_DEFAULT     16000

#define BCM_CHARGE_TOGGLE_PERIOD_SECONDS_MIN        60
#define BCM_CHARGE_TOGGLE_PERIOD_SECONDS_MAX        86400
#define BCM_CHARGE_TOGGLE_PERIOD_SECONDS_DEFAULT    3600
#define BCM_CHARGE_TOGGLE_DURATION_SECONDS          1

#define BCM_XVF_STORAGE_VOLTAGE_TH_MV_MAX           60000
#define BCM_XVF_UPDATE_PERIOD_SECONDS               5

#ifdef BCM_CHARGE_CONTROL_FORCED_HARDWARE
#define BCM_FLAG_CCFH                               0b1
#else
#define BCM_FLAG_CCFH                               0b0
#endif
#ifdef BCM_CHARGE_STATUS_FORCED_HARDWARE
#define BCM_FLAG_CSFH                               0b1
#else
#define BCM_FLAG_CSFH                               0b0
#endif
#ifdef BCM_CHARGE_LED_FORCED_HARDWARE
#define BCM_FLAG_CLFH                               0b1
#else
#define BCM_FLAG_CLFH                               0b0
#endif
#ifdef BCM_BACKUP_CONTROL_FORCED_HARDWARE
#define BCM_FLAG_BKFH                               0b1
#else
#define BCM_FLAG_BKFH                               0b0
#endif

/*** BCM local structures ***/

/*******************************************************************/
typedef struct {
    UNA_bit_representation_t charge_control_state;
    UNA_bit_representation_t backup_control_state;
    uint32_t xvf_update_next_time_seconds;
    int32_t charge_current_ua;
    int32_t charge_current_max_ua;
#ifndef BCM_CHARGE_CONTROL_FORCED_HARDWARE
    int32_t source_voltage_mv;
    uint32_t charge_toggle_previous_time_seconds;
    uint32_t charge_toggle_next_time_seconds;
#endif
} BCM_context_t;

/*** BCM local global variables ***/

static BCM_context_t bcm_ctx = {
    .charge_control_state = UNA_BIT_ERROR,
    .backup_control_state = UNA_BIT_ERROR,
    .charge_current_ua = 0,
    .charge_current_max_ua = 0,
    .xvf_update_next_time_seconds = 0,
#ifndef BCM_CHARGE_CONTROL_FORCED_HARDWARE
    .source_voltage_mv = 0,
    .charge_toggle_previous_time_seconds = 0,
    .charge_toggle_next_time_seconds = 0,
#endif
};

/*** BCM functions ***/

/*******************************************************************/
NODE_status_t BCM_init(void) {
    // Local variables.
    NODE_status_t status = NODE_SUCCESS;
    // Init context.
    bcm_ctx.charge_control_state = UNA_BIT_ERROR;
    bcm_ctx.backup_control_state = UNA_BIT_ERROR;
    bcm_ctx.charge_current_ua = 0;
    bcm_ctx.charge_current_max_ua = 0;
    bcm_ctx.xvf_update_next_time_seconds = 0;
#ifndef BCM_CHARGE_CONTROL_FORCED_HARDWARE
    bcm_ctx.source_voltage_mv = 0;
    bcm_ctx.charge_toggle_previous_time_seconds = 0;
    bcm_ctx.charge_toggle_next_time_seconds = 0;
#endif
    return status;
}

/*******************************************************************/
void BCM_init_register(uint8_t reg_addr, uint32_t* reg_value) {
    // Local variables.
    uint32_t unused_mask = 0;
    // Check address.
    switch (reg_addr) {
    case BCM_REGISTER_ADDRESS_FLAGS_1:
        SWREG_write_field(reg_value, &unused_mask, BCM_FLAG_CCFH, BCM_REGISTER_FLAGS_1_MASK_CCFH);
        SWREG_write_field(reg_value, &unused_mask, BCM_FLAG_CSFH, BCM_REGISTER_FLAGS_1_MASK_CSFH);
        SWREG_write_field(reg_value, &unused_mask, BCM_FLAG_CLFH, BCM_REGISTER_FLAGS_1_MASK_CLFH);
        SWREG_write_field(reg_value, &unused_mask, BCM_FLAG_BKFH, BCM_REGISTER_FLAGS_1_MASK_BKFH);
        break;
#ifdef DSM_NVM_FACTORY_RESET
    case BCM_REGISTER_ADDRESS_CONFIGURATION_0:
        SWREG_write_field(reg_value, &unused_mask, UNA_convert_seconds(BCM_CHARGE_TOGGLE_PERIOD_SECONDS), BCM_REGISTER_CONFIGURATION_0_MASK_CHARGE_TOGGLE_PERIOD);
        SWREG_write_field(reg_value, &unused_mask, UNA_convert_mv(BCM_CHARGE_SOURCE_VOLTAGE_TH_MV), BCM_REGISTER_CONFIGURATION_0_MASK_CHARGE_SOURCE_VOLTAGE_TH);
        break;
    case BCM_REGISTER_ADDRESS_CONFIGURATION_1:
        SWREG_write_field(reg_value, &unused_mask, UNA_convert_mv(BCM_LVF_STORAGE_VOLTAGE_THL_MV), BCM_REGISTER_CONFIGURATION_1_MASK_LVF_STORAGE_VOLTAGE_THL);
        SWREG_write_field(reg_value, &unused_mask, UNA_convert_mv(BCM_LVF_STORAGE_VOLTAGE_THH_MV), BCM_REGISTER_CONFIGURATION_1_MASK_LVF_STORAGE_VOLTAGE_THH);
        break;
    case BCM_REGISTER_ADDRESS_CONFIGURATION_2:
        SWREG_write_field(reg_value, &unused_mask, UNA_convert_mv(BCM_CVF_STORAGE_VOLTAGE_THL_MV), BCM_REGISTER_CONFIGURATION_2_MASK_CVF_STORAGE_VOLTAGE_THL);
        SWREG_write_field(reg_value, &unused_mask, UNA_convert_mv(BCM_CVF_STORAGE_VOLTAGE_THH_MV), BCM_REGISTER_CONFIGURATION_2_MASK_CVF_STORAGE_VOLTAGE_THH);
        break;
#endif
    default:
        break;
    }
}

/*******************************************************************/
void BCM_refresh_register(uint8_t reg_addr) {
    // Local variables.
    uint32_t* reg_ptr = &(NODE_RAM_REGISTER[reg_addr]);
    UNA_bit_representation_t chrgst0 = UNA_BIT_ERROR;
    UNA_bit_representation_t chrgst1 = UNA_BIT_ERROR;
    uint32_t unused_mask = 0;
    // Check address.
    switch (reg_addr) {
    case BCM_REGISTER_ADDRESS_STATUS_1:
        // Charge status.
#ifdef BCM_CHARGE_STATUS_FORCED_HARDWARE
        chrgst0 = UNA_BIT_FORCED_HARDWARE;
        chrgst1 = UNA_BIT_FORCED_HARDWARE;
#else
        // Check current.
        if (bcm_ctx.charge_current_ua > 0) {
            // Read pins.
            chrgst0 = ((LOAD_get_charge_status() >> 0) & 0x01);
            chrgst1 = ((LOAD_get_charge_status() >> 1) & 0x01);
        }
        else {
            // Force status to not charging or terminated.
            chrgst0 = UNA_BIT_0;
            chrgst1 = UNA_BIT_0;
        }
#endif
        // Charge state.
#ifdef BCM_CHARGE_CONTROL_FORCED_HARDWARE
        bcm_ctx.charge_control_state = UNA_BIT_FORCED_HARDWARE;
#else
        bcm_ctx.charge_control_state = LOAD_get_charge_state();
#endif
        // Backup_output state.
#ifdef BCM_BACKUP_CONTROL_FORCED_HARDWARE
        bcm_ctx.backup_control_state = UNA_BIT_FORCED_HARDWARE;
#else
        switch (LOAD_get_output_state()) {
        case 0:
            bcm_ctx.backup_control_state = UNA_BIT_0;
            break;
        case 1:
            bcm_ctx.backup_control_state = UNA_BIT_1;
            break;
        default:
            bcm_ctx.backup_control_state = UNA_BIT_ERROR;
            break;
        }
#endif
        SWREG_write_field(reg_ptr, &unused_mask, ((uint32_t) chrgst0), BCM_REGISTER_STATUS_1_MASK_CHST0);
        SWREG_write_field(reg_ptr, &unused_mask, ((uint32_t) chrgst1), BCM_REGISTER_STATUS_1_MASK_CHST1);
        SWREG_write_field(reg_ptr, &unused_mask, ((uint32_t) bcm_ctx.charge_control_state), BCM_REGISTER_STATUS_1_MASK_CHCS);
        SWREG_write_field(reg_ptr, &unused_mask, ((uint32_t) bcm_ctx.backup_control_state), BCM_REGISTER_STATUS_1_MASK_BKCS);
        break;
    default:
        break;
    }
}

/*******************************************************************/
NODE_status_t BCM_secure_register(uint8_t reg_addr, uint32_t new_reg_value, uint32_t* reg_mask, uint32_t* reg_value) {
    // Local variables.
    NODE_status_t status = NODE_SUCCESS;
    int32_t low_threshold_mv = 0;
    int32_t high_threshold_mv = 0;
    int32_t generic_s32 = 0;
    uint32_t generic_u32 = 0;
    // Check address.
    switch (reg_addr) {
    case BCM_REGISTER_ADDRESS_CONFIGURATION_0:
        SWREG_secure_field(
            BCM_REGISTER_CONFIGURATION_0_MASK_CHARGE_SOURCE_VOLTAGE_TH,
            UNA_get_mv,
            UNA_convert_mv,
            > BCM_CHARGE_SOURCE_VOLTAGE_TH_MV_MAX,
            > BCM_CHARGE_SOURCE_VOLTAGE_TH_MV_MAX,
            BCM_CHARGE_SOURCE_VOLTAGE_TH_MV_DEFAULT,
            status = NODE_ERROR_REGISTER_FIELD_VALUE
        );
        SWREG_secure_field(
            BCM_REGISTER_CONFIGURATION_0_MASK_CHARGE_TOGGLE_PERIOD,
            UNA_get_seconds,
            UNA_convert_seconds,
            < BCM_CHARGE_TOGGLE_PERIOD_SECONDS_MIN,
            > BCM_CHARGE_TOGGLE_PERIOD_SECONDS_MAX,
            BCM_CHARGE_TOGGLE_PERIOD_SECONDS_DEFAULT,
            status = NODE_ERROR_REGISTER_FIELD_VALUE
        );
        break;
    case BCM_REGISTER_ADDRESS_CONFIGURATION_1:
        low_threshold_mv = UNA_convert_mv(SWREG_read_field(new_reg_value, BCM_REGISTER_CONFIGURATION_1_MASK_LVF_STORAGE_VOLTAGE_THL));
        high_threshold_mv = UNA_convert_mv(SWREG_read_field(new_reg_value, BCM_REGISTER_CONFIGURATION_1_MASK_LVF_STORAGE_VOLTAGE_THH));
        SWREG_secure_field(
            BCM_REGISTER_CONFIGURATION_1_MASK_LVF_STORAGE_VOLTAGE_THL,
            UNA_get_mv,
            UNA_convert_mv,
            > BCM_XVF_STORAGE_VOLTAGE_TH_MV_MAX,
            > high_threshold_mv,
            0,
            status = NODE_ERROR_REGISTER_FIELD_VALUE
        );
        SWREG_secure_field(
            BCM_REGISTER_CONFIGURATION_1_MASK_LVF_STORAGE_VOLTAGE_THH,
            UNA_get_mv,
            UNA_convert_mv,
            > BCM_XVF_STORAGE_VOLTAGE_TH_MV_MAX,
            < low_threshold_mv,
            0,
            status = NODE_ERROR_REGISTER_FIELD_VALUE
        );
        break;
    case BCM_REGISTER_ADDRESS_CONFIGURATION_2:
        low_threshold_mv = UNA_convert_mv(SWREG_read_field(new_reg_value, BCM_REGISTER_CONFIGURATION_2_MASK_CVF_STORAGE_VOLTAGE_THL));
        high_threshold_mv = UNA_convert_mv(SWREG_read_field(new_reg_value, BCM_REGISTER_CONFIGURATION_2_MASK_CVF_STORAGE_VOLTAGE_THH));
        SWREG_secure_field(
            BCM_REGISTER_CONFIGURATION_2_MASK_CVF_STORAGE_VOLTAGE_THL,
            UNA_get_mv,
            UNA_convert_mv,
            > BCM_XVF_STORAGE_VOLTAGE_TH_MV_MAX,
            > high_threshold_mv,
            0,
            status = NODE_ERROR_REGISTER_FIELD_VALUE
        );
        SWREG_secure_field(
            BCM_REGISTER_CONFIGURATION_2_MASK_CVF_STORAGE_VOLTAGE_THH,
            UNA_get_mv,
            UNA_convert_mv,
            > BCM_XVF_STORAGE_VOLTAGE_TH_MV_MAX,
            < low_threshold_mv,
            0,
            status = NODE_ERROR_REGISTER_FIELD_VALUE
        );
        break;
    default:
        break;
    }
    return status;
}

/*******************************************************************/
NODE_status_t BCM_process_register(uint8_t reg_addr, uint32_t reg_mask) {
    // Local variables.
    NODE_status_t status = NODE_SUCCESS;
    uint32_t* reg_ptr = &(NODE_RAM_REGISTER[reg_addr]);
#ifndef BCM_BACKUP_CONTROL_FORCED_HARDWARE
    LOAD_status_t load_status = LOAD_SUCCESS;
    UNA_bit_representation_t bken = UNA_BIT_ERROR;
#endif
#ifndef BCM_CHARGE_CONTROL_FORCED_HARDWARE
    UNA_bit_representation_t chen = UNA_BIT_ERROR;
#endif
    // Check address.
    switch (reg_addr) {
    case BCM_REGISTER_ADDRESS_CONTROL_1:
        // CHEN.
        if ((reg_mask & BCM_REGISTER_CONTROL_1_MASK_CHEN) != 0) {
#ifdef BCM_CHARGE_CONTROL_FORCED_HARDWARE
            status = NODE_ERROR_FORCED_HARDWARE;
            goto errors;
#else
            // Check control mode.
            if (SWREG_read_field((*reg_ptr), BCM_REGISTER_CONTROL_1_MASK_CHMD) != 0) {
                // Read bit.
                chen = SWREG_read_field((*reg_ptr), BCM_REGISTER_CONTROL_1_MASK_CHEN);
                // Compare to current state.
                if (chen != bcm_ctx.charge_control_state) {
                    // Set charge state.
                    LOAD_set_charge_state(chen);
                }
            }
            else {
                status = NODE_ERROR_FORCED_SOFTWARE;
                goto errors;
            }
#endif
        }
        // BKEN.
        if ((reg_mask & BCM_REGISTER_CONTROL_1_MASK_BKEN) != 0) {
            // Check pin mode.
#ifdef BCM_BACKUP_CONTROL_FORCED_HARDWARE
            status = NODE_ERROR_FORCED_HARDWARE;
            goto errors;
#else
            // Read bit.
            bken = SWREG_read_field((*reg_ptr), BCM_REGISTER_CONTROL_1_MASK_BKEN);
            // Compare to current state.
            if (bken != bcm_ctx.backup_control_state) {
                // Set output state.
                load_status = LOAD_set_output_state(bken);
                LOAD_exit_error(NODE_ERROR_BASE_LOAD);
            }
#endif
        }
        break;
    default:
        UNUSED(reg_ptr);
        break;
    }
errors:
    BCM_refresh_register(BCM_REGISTER_ADDRESS_STATUS_1);
    return status;
}

/*******************************************************************/
NODE_status_t BCM_mtrg_callback(void) {
    // Local variables.
    NODE_status_t status = NODE_SUCCESS;
    ANALOG_status_t analog_status = ANALOG_SUCCESS;
    uint32_t* reg_analog_data_1_ptr = &(NODE_RAM_REGISTER[BCM_REGISTER_ADDRESS_ANALOG_DATA_1]);
    uint32_t* reg_analog_data_2_ptr = &(NODE_RAM_REGISTER[BCM_REGISTER_ADDRESS_ANALOG_DATA_2]);
    int32_t adc_data = 0;
    uint32_t unused_mask = 0;
    // Reset data.
    (*reg_analog_data_1_ptr) = NODE_REGISTER[BCM_REGISTER_ADDRESS_ANALOG_DATA_1].error_value;
    (*reg_analog_data_2_ptr) = NODE_REGISTER[BCM_REGISTER_ADDRESS_ANALOG_DATA_2].error_value;
    // Source voltage.
    analog_status = ANALOG_convert_channel(ANALOG_CHANNEL_SOURCE_VOLTAGE_MV, &adc_data);
    ANALOG_exit_error(NODE_ERROR_BASE_ANALOG);
    SWREG_write_field(reg_analog_data_1_ptr, &unused_mask, UNA_convert_mv(adc_data), BCM_REGISTER_ANALOG_DATA_1_MASK_SOURCE_VOLTAGE);
    // Storage element voltage.
    analog_status = ANALOG_convert_channel(ANALOG_CHANNEL_STORAGE_VOLTAGE_MV, &adc_data);
    ANALOG_exit_error(NODE_ERROR_BASE_ANALOG);
    SWREG_write_field(reg_analog_data_1_ptr, &unused_mask, UNA_convert_mv(adc_data), BCM_REGISTER_ANALOG_DATA_1_MASK_STORAGE_VOLTAGE);
    // Backup output voltage.
    analog_status = ANALOG_convert_channel(ANALOG_CHANNEL_BACKUP_VOLTAGE_MV, &adc_data);
    ANALOG_exit_error(NODE_ERROR_BASE_ANALOG);
    SWREG_write_field(reg_analog_data_2_ptr, &unused_mask, UNA_convert_mv(adc_data), BCM_REGISTER_ANALOG_DATA_2_MASK_BACKUP_VOLTAGE);
    // Battery charge current.
    analog_status = ANALOG_convert_channel(ANALOG_CHANNEL_CHARGE_CURRENT_UA, &adc_data);
    ANALOG_exit_error(NODE_ERROR_BASE_ANALOG);
    if (adc_data > bcm_ctx.charge_current_max_ua) {
        bcm_ctx.charge_current_max_ua = adc_data;
    }
    SWREG_write_field(reg_analog_data_2_ptr, &unused_mask, UNA_convert_ua(bcm_ctx.charge_current_max_ua), BCM_REGISTER_ANALOG_DATA_2_MASK_CHARGE_CURRENT);
    bcm_ctx.charge_current_max_ua = 0;
errors:
    return status;
}

/*******************************************************************/
NODE_status_t BCM_low_voltage_detector_process(void) {
    // Local variables.
    NODE_status_t status = NODE_SUCCESS;
    ANALOG_status_t analog_status = ANALOG_SUCCESS;
    uint32_t uptime_seconds = RTC_get_uptime_seconds();
    uint32_t* reg_config_1_ptr = &(NODE_RAM_REGISTER[BCM_REGISTER_ADDRESS_CONFIGURATION_1]);
    uint32_t* reg_config_2_ptr = &(NODE_RAM_REGISTER[BCM_REGISTER_ADDRESS_CONFIGURATION_2]);
    uint32_t* reg_status_1_ptr = &(NODE_RAM_REGISTER[BCM_REGISTER_ADDRESS_STATUS_1]);
    int32_t storage_voltage_mv = 0;
    uint32_t unused_mask = 0;
    // Check period.
    if (uptime_seconds >= bcm_ctx.xvf_update_next_time_seconds) {
        // Update next time.
        bcm_ctx.xvf_update_next_time_seconds = uptime_seconds + BCM_XVF_UPDATE_PERIOD_SECONDS;
        // Turn analog front-end on.
        POWER_enable(POWER_REQUESTER_ID_BCM, POWER_DOMAIN_ANALOG, LPTIM_DELAY_MODE_ACTIVE);
        // Read battery voltage.
        analog_status = ANALOG_convert_channel(ANALOG_CHANNEL_STORAGE_VOLTAGE_MV, &storage_voltage_mv);
        ANALOG_exit_error(NODE_ERROR_BASE_ANALOG);
        // Read charge current for status update.
        analog_status = ANALOG_convert_channel(ANALOG_CHANNEL_CHARGE_CURRENT_UA, &(bcm_ctx.charge_current_ua));
        ANALOG_exit_error(NODE_ERROR_BASE_ANALOG);
        // Update maximum value.
        if (bcm_ctx.charge_current_ua > bcm_ctx.charge_current_max_ua) {
            bcm_ctx.charge_current_max_ua = bcm_ctx.charge_current_ua;
        }
#ifndef BCM_CHARGE_CONTROL_FORCED_HARDWARE
        analog_status = ANALOG_convert_channel(ANALOG_CHANNEL_SOURCE_VOLTAGE_MV, &(bcm_ctx.source_voltage_mv));
        ANALOG_exit_error(NODE_ERROR_BASE_ANALOG);
#endif
        // Update LVF flag.
        if (storage_voltage_mv < UNA_get_mv(SWREG_read_field((*reg_config_1_ptr), BCM_REGISTER_CONFIGURATION_1_MASK_LVF_STORAGE_VOLTAGE_THL))) {
            SWREG_write_field(reg_status_1_ptr, &unused_mask, 0b1, BCM_REGISTER_STATUS_1_MASK_LVF);
        }
        if (storage_voltage_mv > UNA_get_mv(SWREG_read_field((*reg_config_1_ptr), BCM_REGISTER_CONFIGURATION_1_MASK_LVF_STORAGE_VOLTAGE_THH))) {
            SWREG_write_field(reg_status_1_ptr, &unused_mask, 0b0, BCM_REGISTER_STATUS_1_MASK_LVF);
        }
        // Update CVF flag.
        if (storage_voltage_mv < UNA_get_mv(SWREG_read_field((*reg_config_2_ptr), BCM_REGISTER_CONFIGURATION_2_MASK_CVF_STORAGE_VOLTAGE_THL))) {
            SWREG_write_field(reg_status_1_ptr, &unused_mask, 0b1, BCM_REGISTER_STATUS_1_MASK_CVF);
        }
        if (storage_voltage_mv > UNA_get_mv(SWREG_read_field((*reg_config_2_ptr), BCM_REGISTER_CONFIGURATION_2_MASK_CVF_STORAGE_VOLTAGE_THH))) {
            SWREG_write_field(reg_status_1_ptr, &unused_mask, 0b0, BCM_REGISTER_STATUS_1_MASK_CVF);
        }
    }
errors:
    POWER_disable(POWER_REQUESTER_ID_BCM, POWER_DOMAIN_ANALOG);
    return status;
}

#ifndef BCM_CHARGE_CONTROL_FORCED_HARDWARE
/*******************************************************************/
NODE_status_t BCM_charge_process(void) {
    // Local variables.
    NODE_status_t status = NODE_SUCCESS;
    uint32_t reg_control_1 = NODE_RAM_REGISTER[BCM_REGISTER_ADDRESS_CONTROL_1];
    uint32_t reg_config_0 = NODE_RAM_REGISTER[BCM_REGISTER_ADDRESS_CONFIGURATION_0];
    uint32_t uptime_seconds = RTC_get_uptime_seconds();
    // Check mode.
    if (SWREG_read_field(reg_control_1, BCM_REGISTER_CONTROL_1_MASK_CHMD) != 0) goto errors;
    // Check toggle period.
    if (uptime_seconds >= bcm_ctx.charge_toggle_next_time_seconds) {
        // Update times.
        bcm_ctx.charge_toggle_previous_time_seconds = uptime_seconds;
        bcm_ctx.charge_toggle_next_time_seconds = uptime_seconds + ((uint32_t) UNA_get_seconds(SWREG_read_field(reg_config_0, BCM_REGISTER_CONFIGURATION_0_MASK_CHARGE_TOGGLE_PERIOD)));
        // Disable charge.
        LOAD_set_charge_state(0);
    }
    if (uptime_seconds >= (bcm_ctx.charge_toggle_previous_time_seconds + BCM_CHARGE_TOGGLE_DURATION_SECONDS)) {
        // Check voltage.
        if (bcm_ctx.source_voltage_mv >= UNA_get_mv(SWREG_read_field(reg_config_0, BCM_REGISTER_CONFIGURATION_0_MASK_CHARGE_SOURCE_VOLTAGE_TH))) {
            // Enable charge.
            LOAD_set_charge_state(1);
        }
        else {
            // Disable charge.
            LOAD_set_charge_state(0);
        }
    }
errors:
    return status;
}
#endif

#endif /* BCM */

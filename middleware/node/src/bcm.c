/*
 * bcm.c
 *
 *  Created on: 27 mar. 2025
 *      Author: Ludo
 */

#include "bcm.h"

#include "analog.h"
#include "bcm_registers.h"
#include "error.h"
#include "load.h"
#include "node.h"
#include "swreg.h"
#include "rtc.h"
#include "una.h"
#include "xm_flags.h"

#ifdef BCM

/*** BCM local macros ***/

#define BCM_LVF_UPDATE_PERIOD_SECONDS       5
#define BCM_CHEN_TOGGLE_DURATION_SECONDS    1

/*** BCM local structures ***/

/*******************************************************************/
typedef struct {
    UNA_bit_representation_t chenst;
    UNA_bit_representation_t bkenst;
    uint32_t lvf_update_next_time_seconds;
#ifndef BCM_CHEN_FORCED_HARDWARE
    int32_t vsrc_mv;
    uint32_t chen_toggle_previous_time_seconds;
    uint32_t chen_toggle_next_time_seconds;
#endif
} BCM_context_t;

/*** BCM local global variables ***/

static BCM_context_t bcm_ctx = {
    .chenst = UNA_BIT_ERROR,
    .bkenst = UNA_BIT_ERROR,
    .lvf_update_next_time_seconds = 0,
#ifndef BCM_CHEN_FORCED_HARDWARE
    .vsrc_mv = 0,
    .chen_toggle_previous_time_seconds = 0,
    .chen_toggle_next_time_seconds = 0,
#endif
};

/*** BCM local functions ***/

/*******************************************************************/
static void _BCM_load_fixed_configuration(void) {
    // Local variables.
    uint32_t reg_value = 0;
    uint32_t reg_mask = 0;
    // Backup output control mode.
#ifdef BCM_BKEN_FORCED_HARDWARE
    SWREG_write_field(&reg_value, &reg_mask, 0b1, BCM_REGISTER_CONFIGURATION_0_MASK_BKFH);
#else
    SWREG_write_field(&reg_value, &reg_mask, 0b0, BCM_REGISTER_CONFIGURATION_0_MASK_BKFH);
#endif
    // Charge status mode.
#ifdef BCM_CHST_FORCED_HARDWARE
    SWREG_write_field(&reg_value, &reg_mask, 0b1, BCM_REGISTER_CONFIGURATION_0_MASK_CSFH);
#else
    SWREG_write_field(&reg_value, &reg_mask, 0b0, BCM_REGISTER_CONFIGURATION_0_MASK_CSFH);
#endif
    // Charge status LED control mode.
#ifdef BCM_CHLD_FORCED_HARDWARE
    SWREG_write_field(&reg_value, &reg_mask, 0b1, BCM_REGISTER_CONFIGURATION_0_MASK_CLFH);
#else
    SWREG_write_field(&reg_value, &reg_mask, 0b0, BCM_REGISTER_CONFIGURATION_0_MASK_CLFH);
#endif
    // Charge control mode.
#ifdef BCM_CHEN_FORCED_HARDWARE
    SWREG_write_field(&reg_value, &reg_mask, 0b1, BCM_REGISTER_CONFIGURATION_0_MASK_CEFH);
#else
    SWREG_write_field(&reg_value, &reg_mask, 0b0, BCM_REGISTER_CONFIGURATION_0_MASK_CEFH);
#endif
    NODE_write_register(NODE_REQUEST_SOURCE_INTERNAL, BCM_REGISTER_ADDRESS_CONFIGURATION_0, reg_value, reg_mask);
}

/*******************************************************************/
static void _BCM_load_dynamic_configuration(void) {
    // Local variables.
    uint8_t reg_addr = 0;
    uint32_t reg_value = 0;
    // Load configuration registers from NVM.
    for (reg_addr = BCM_REGISTER_ADDRESS_CONFIGURATION_1; reg_addr < BCM_REGISTER_ADDRESS_STATUS_1; reg_addr++) {
        // Read NVM.
        NODE_read_nvm(reg_addr, &reg_value);
        // Write register.
        NODE_write_register(NODE_REQUEST_SOURCE_INTERNAL, reg_addr, reg_value, UNA_REGISTER_MASK_ALL);
    }
}

/*******************************************************************/
static void _BCM_reset_analog_data(void) {
    // Local variables.
    uint32_t reg_analog_data_1 = 0;
    uint32_t reg_analog_data_1_mask = 0;
    uint32_t reg_analog_data_2 = 0;
    uint32_t reg_analog_data_2_mask = 0;
    // VSRC / VSTR.
    SWREG_write_field(&reg_analog_data_1, &reg_analog_data_1_mask, UNA_VOLTAGE_ERROR_VALUE, BCM_REGISTER_ANALOG_DATA_1_MASK_VSRC);
    SWREG_write_field(&reg_analog_data_1, &reg_analog_data_1_mask, UNA_VOLTAGE_ERROR_VALUE, BCM_REGISTER_ANALOG_DATA_1_MASK_VSTR);
    NODE_write_register(NODE_REQUEST_SOURCE_INTERNAL, BCM_REGISTER_ADDRESS_ANALOG_DATA_1, reg_analog_data_1, reg_analog_data_1_mask);
    // VBKP / ISTR.
    SWREG_write_field(&reg_analog_data_2, &reg_analog_data_2_mask, UNA_VOLTAGE_ERROR_VALUE, BCM_REGISTER_ANALOG_DATA_2_MASK_VBKP);
    SWREG_write_field(&reg_analog_data_2, &reg_analog_data_2_mask, UNA_VOLTAGE_ERROR_VALUE, BCM_REGISTER_ANALOG_DATA_2_MASK_ISTR);
    NODE_write_register(NODE_REQUEST_SOURCE_INTERNAL, BCM_REGISTER_ADDRESS_ANALOG_DATA_2, reg_analog_data_2, reg_analog_data_2_mask);
}

/*** BCM functions ***/

/*******************************************************************/
NODE_status_t BCM_init_registers(void) {
    // Local variables.
    NODE_status_t status = NODE_SUCCESS;
#ifdef XM_NVM_FACTORY_RESET
    uint32_t reg_value = 0;
    uint32_t reg_mask = 0;
#endif
    // Init context.
    bcm_ctx.chenst = UNA_BIT_ERROR;
    bcm_ctx.bkenst = UNA_BIT_ERROR;
    bcm_ctx.lvf_update_next_time_seconds = 0;
#ifndef BCM_CHEN_FORCED_HARDWARE
    bcm_ctx.vsrc_mv = 0;
    bcm_ctx.chen_toggle_previous_time_seconds = 0;
    bcm_ctx.chen_toggle_next_time_seconds = 0;
#endif
#ifdef XM_NVM_FACTORY_RESET
    // CHEN toggle threshold and period.
    SWREG_write_field(&reg_value, &reg_mask, UNA_convert_seconds(BCM_CHEN_TOGGLE_PERIOD_SECONDS), BCM_REGISTER_CONFIGURATION_1_MASK_CHEN_TOGGLE_PERIOD);
    SWREG_write_field(&reg_value, &reg_mask, UNA_convert_mv(BCM_CHEN_VSRC_THRESHOLD_MV), BCM_REGISTER_CONFIGURATION_1_MASK_CHEN_THRESHOLD);
    NODE_write_register(NODE_REQUEST_SOURCE_EXTERNAL, BCM_REGISTER_ADDRESS_CONFIGURATION_1, reg_value, reg_mask);
    // Low voltage detector thresholds.
    reg_value = 0;
    reg_mask = 0;
    SWREG_write_field(&reg_value, &reg_mask, UNA_convert_mv(BCM_LVF_LOW_THRESHOLD_MV), BCM_REGISTER_CONFIGURATION_2_MASK_LVF_LOW_THRESHOLD);
    SWREG_write_field(&reg_value, &reg_mask, UNA_convert_mv(BCM_LVF_HIGH_THRESHOLD_MV), BCM_REGISTER_CONFIGURATION_2_MASK_LVF_HIGH_THRESHOLD);
    NODE_write_register(NODE_REQUEST_SOURCE_EXTERNAL, BCM_REGISTER_ADDRESS_CONFIGURATION_2, reg_value, reg_mask);
#endif
    // Load default values.
    _BCM_load_fixed_configuration();
    _BCM_load_dynamic_configuration();
    _BCM_reset_analog_data();
    // Read init state.
    status = BCM_update_register(BCM_REGISTER_ADDRESS_STATUS_1);
    if (status != NODE_SUCCESS) goto errors;
errors:
    return status;
}

/*******************************************************************/
NODE_status_t BCM_update_register(uint8_t reg_addr) {
    // Local variables.
    NODE_status_t status = NODE_SUCCESS;
    uint32_t reg_value = 0;
    uint32_t reg_mask = 0;
    UNA_bit_representation_t chrgst0 = UNA_BIT_ERROR;
    UNA_bit_representation_t chrgst1 = UNA_BIT_ERROR;
    // Check address.
    switch (reg_addr) {
    case BCM_REGISTER_ADDRESS_STATUS_1:
        // Charge status.
#ifdef BCM_CHST_FORCED_HARDWARE
        chrgst0 = UNA_BIT_FORCED_HARDWARE;
        chrgst1 = UNA_BIT_FORCED_HARDWARE;
#else
        chrgst0 = ((LOAD_get_charge_status() >> 0) & 0x01);
        chrgst1 = ((LOAD_get_charge_status() >> 1) & 0x01);
#endif
        SWREG_write_field(&reg_value, &reg_mask, ((uint32_t) chrgst0), BCM_REGISTER_STATUS_1_MASK_CHRGST0);
        SWREG_write_field(&reg_value, &reg_mask, ((uint32_t) chrgst1), BCM_REGISTER_STATUS_1_MASK_CHRGST1);
        // Charge state.
#ifdef BCM_CHEN_FORCED_HARDWARE
        bcm_ctx.chenst = UNA_BIT_FORCED_HARDWARE;
#else
        bcm_ctx.chenst = LOAD_get_charge_state();
#endif
        SWREG_write_field(&reg_value, &reg_mask, ((uint32_t) bcm_ctx.chenst), BCM_REGISTER_STATUS_1_MASK_CHENST);
        // Backup_output state.
#ifdef BCM_BKEN_FORCED_HARDWARE
        bcm_ctx.bkenst = UNA_BIT_FORCED_HARDWARE;
#else
        bcm_ctx.bkenst = (LOAD_get_output_state() == 0) ? UNA_BIT_0 : UNA_BIT_1;
#endif
        SWREG_write_field(&reg_value, &reg_mask, ((uint32_t) bcm_ctx.bkenst), BCM_REGISTER_STATUS_1_MASK_BKENST);
        break;
    default:
        // Nothing to do for other registers.
        break;
    }
    NODE_write_register(NODE_REQUEST_SOURCE_INTERNAL, reg_addr, reg_value, reg_mask);
    return status;
}

/*******************************************************************/
NODE_status_t BCM_check_register(uint8_t reg_addr, uint32_t reg_mask) {
    // Local variables.
    NODE_status_t status = NODE_SUCCESS;
#ifndef BCM_BKEN_FORCED_HARDWARE
    LOAD_status_t load_status = LOAD_SUCCESS;
    UNA_bit_representation_t bken = UNA_BIT_ERROR;
#endif
#ifndef BCM_CHEN_FORCED_HARDWARE
    UNA_bit_representation_t chen = UNA_BIT_ERROR;
#endif
    uint32_t reg_value = 0;
    // Read register.
    status = NODE_read_register(NODE_REQUEST_SOURCE_INTERNAL, reg_addr, &reg_value);
    if (status != NODE_SUCCESS) goto errors;
    // Check address.
    switch (reg_addr) {
    case BCM_REGISTER_ADDRESS_CONFIGURATION_1:
    case BCM_REGISTER_ADDRESS_CONFIGURATION_2:
        // Store new value in NVM.
        if (reg_mask != 0) {
            status = NODE_write_nvm(reg_addr, reg_value);
            if (status != NODE_SUCCESS) goto errors;
        }
        break;
    case BCM_REGISTER_ADDRESS_CONTROL_1:
        // CHEN.
        if ((reg_mask & BCM_REGISTER_CONTROL_1_MASK_CHEN) != 0) {
#ifdef BCM_CHEN_FORCED_HARDWARE
            status = NODE_ERROR_FORCED_HARDWARE;
            goto errors;
#else
            // Check control mode.
            if (SWREG_read_field(reg_value, BCM_REGISTER_CONTROL_1_MASK_CHMD) != 0) {
                // Read bit.
                chen = SWREG_read_field(reg_value, BCM_REGISTER_CONTROL_1_MASK_CHEN);
                // Compare to current state.
                if (chen != bcm_ctx.chenst) {
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
#ifdef BCM_BKEN_FORCED_HARDWARE
            status = NODE_ERROR_FORCED_HARDWARE;
            goto errors;
#else
            // Read bit.
            bken = SWREG_read_field(reg_value, BCM_REGISTER_CONTROL_1_MASK_BKEN);
            // Compare to current state.
            if (bken != bcm_ctx.bkenst) {
                // Set output state.
                load_status = LOAD_set_output_state(bken);
                LOAD_exit_error(NODE_ERROR_BASE_LOAD);
            }
#endif
        }
        break;
    default:
        // Nothing to do for other registers.
        break;
    }
errors:
    // Update status register.
    BCM_update_register(BCM_REGISTER_ADDRESS_STATUS_1);
    return status;
}

/*******************************************************************/
NODE_status_t BCM_mtrg_callback(void) {
    // Local variables.
    NODE_status_t status = NODE_SUCCESS;
    ANALOG_status_t analog_status = ANALOG_SUCCESS;
    int32_t adc_data = 0;
    uint32_t reg_analog_data_1 = 0;
    uint32_t reg_analog_data_1_mask = 0;
    uint32_t reg_analog_data_2 = 0;
    uint32_t reg_analog_data_2_mask = 0;
    // Reset results.
    _BCM_reset_analog_data();
    // Source voltage.
    analog_status = ANALOG_convert_channel(ANALOG_CHANNEL_VSRC_MV, &adc_data);
    ANALOG_exit_error(NODE_ERROR_BASE_ANALOG);
    SWREG_write_field(&reg_analog_data_1, &reg_analog_data_1_mask, UNA_convert_mv(adc_data), BCM_REGISTER_ANALOG_DATA_1_MASK_VSRC);
    // Storage element voltage.
    analog_status = ANALOG_convert_channel(ANALOG_CHANNEL_VSTR_MV, &adc_data);
    ANALOG_exit_error(NODE_ERROR_BASE_ANALOG);
    SWREG_write_field(&reg_analog_data_1, &reg_analog_data_1_mask, UNA_convert_mv(adc_data), BCM_REGISTER_ANALOG_DATA_1_MASK_VSTR);
    // Backup output voltage.
    analog_status = ANALOG_convert_channel(ANALOG_CHANNEL_VBKP_MV, &adc_data);
    ANALOG_exit_error(NODE_ERROR_BASE_ANALOG);
    SWREG_write_field(&reg_analog_data_2, &reg_analog_data_2_mask, UNA_convert_mv(adc_data), BCM_REGISTER_ANALOG_DATA_2_MASK_VBKP);
    // Battery charge current.
    analog_status = ANALOG_convert_channel(ANALOG_CHANNEL_ISTR_UA, &adc_data);
    ANALOG_exit_error(NODE_ERROR_BASE_ANALOG);
    SWREG_write_field(&reg_analog_data_2, &reg_analog_data_2_mask, UNA_convert_ua(adc_data), BCM_REGISTER_ANALOG_DATA_2_MASK_ISTR);
    // Write registers.
    NODE_write_register(NODE_REQUEST_SOURCE_INTERNAL, BCM_REGISTER_ADDRESS_ANALOG_DATA_1, reg_analog_data_1, reg_analog_data_1_mask);
    NODE_write_register(NODE_REQUEST_SOURCE_INTERNAL, BCM_REGISTER_ADDRESS_ANALOG_DATA_2, reg_analog_data_2, reg_analog_data_2_mask);
errors:
    return status;
}

/*******************************************************************/
NODE_status_t BCM_low_voltage_detector_process(void) {
    // Local variables.
    NODE_status_t status = NODE_SUCCESS;
    ANALOG_status_t analog_status = ANALOG_SUCCESS;
    uint32_t reg_config_2 = 0;
    int32_t vstr_mv = 0;
    uint32_t uptime_seconds = RTC_get_uptime_seconds();
    // Check period.
    if (uptime_seconds >= bcm_ctx.lvf_update_next_time_seconds) {
        // Update next time.
        bcm_ctx.lvf_update_next_time_seconds = uptime_seconds + BCM_LVF_UPDATE_PERIOD_SECONDS;
        // Turn analog front-end on.
        POWER_enable(POWER_REQUESTER_ID_BCM, POWER_DOMAIN_ANALOG, LPTIM_DELAY_MODE_ACTIVE);
        // Read source voltage.
        analog_status = ANALOG_convert_channel(ANALOG_CHANNEL_VSTR_MV, &vstr_mv);
        ANALOG_exit_error(NODE_ERROR_BASE_ANALOG);
#ifndef BCM_CHEN_FORCED_HARDWARE
        analog_status = ANALOG_convert_channel(ANALOG_CHANNEL_VSRC_MV, &(bcm_ctx.vsrc_mv));
        ANALOG_exit_error(NODE_ERROR_BASE_ANALOG);
#endif
        // Read thresholds.
        NODE_read_register(NODE_REQUEST_SOURCE_INTERNAL, BCM_REGISTER_ADDRESS_CONFIGURATION_2, &reg_config_2);
        // Update LVF flag.
        if (vstr_mv < UNA_get_mv(SWREG_read_field(reg_config_2, BCM_REGISTER_CONFIGURATION_2_MASK_LVF_LOW_THRESHOLD))) {
            NODE_write_register(NODE_REQUEST_SOURCE_INTERNAL, BCM_REGISTER_ADDRESS_STATUS_1, BCM_REGISTER_STATUS_1_MASK_LVF, BCM_REGISTER_STATUS_1_MASK_LVF);
        }
        if (vstr_mv > UNA_get_mv(SWREG_read_field(reg_config_2, BCM_REGISTER_CONFIGURATION_2_MASK_LVF_HIGH_THRESHOLD))) {
            NODE_write_register(NODE_REQUEST_SOURCE_INTERNAL, BCM_REGISTER_ADDRESS_STATUS_1, 0b0, BCM_REGISTER_STATUS_1_MASK_LVF);
        }
    }
errors:
    POWER_disable(POWER_REQUESTER_ID_BCM, POWER_DOMAIN_ANALOG);
    return status;
}

#ifndef BCM_CHEN_FORCED_HARDWARE
/*******************************************************************/
NODE_status_t BCM_charge_process(void) {
    // Local variables.
    NODE_status_t status = NODE_SUCCESS;
    uint32_t reg_control_1 = 0;
    uint32_t reg_config_1 = 0;
    uint32_t uptime_seconds = RTC_get_uptime_seconds();
    // Read control mode, threshold and period.
    NODE_read_register(NODE_REQUEST_SOURCE_INTERNAL, BCM_REGISTER_ADDRESS_CONTROL_1, &reg_control_1);
    NODE_read_register(NODE_REQUEST_SOURCE_INTERNAL, BCM_REGISTER_ADDRESS_CONFIGURATION_1, &reg_config_1);
    // Check mode.
    if (SWREG_read_field(reg_control_1, BCM_REGISTER_CONTROL_1_MASK_CHMD) != 0) goto errors;
    // Check toggle period.
    if (uptime_seconds >= bcm_ctx.chen_toggle_next_time_seconds) {
        // Update times.
        bcm_ctx.chen_toggle_previous_time_seconds = uptime_seconds;
        bcm_ctx.chen_toggle_next_time_seconds = uptime_seconds + ((uint32_t) UNA_get_seconds(SWREG_read_field(reg_config_1, BCM_REGISTER_CONFIGURATION_1_MASK_CHEN_TOGGLE_PERIOD)));
        // Disable charge.
        LOAD_set_charge_state(0);
    }
    if (uptime_seconds >= (bcm_ctx.chen_toggle_previous_time_seconds + BCM_CHEN_TOGGLE_DURATION_SECONDS)) {
        // Check voltage.
        if (bcm_ctx.vsrc_mv >= UNA_get_mv(SWREG_read_field(reg_config_1, BCM_REGISTER_CONFIGURATION_1_MASK_CHEN_THRESHOLD))) {
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

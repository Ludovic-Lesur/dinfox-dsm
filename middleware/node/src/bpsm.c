/*
 * bpsm.c
 *
 *  Created on: 04 jun. 2023
 *      Author: Ludo
 */

#include "bpsm.h"

#ifdef BPSM

#include "analog.h"
#include "bpsm_registers.h"
#include "dsm_flags.h"
#include "error.h"
#include "load.h"
#include "node_register.h"
#include "node_status.h"
#include "swreg.h"
#include "rtc.h"
#include "types.h"
#include "una.h"

/*** BPSM local macros ***/

#define BPSM_CHEN_THRESHOLD_MV_MAX                  60000
#define BPSM_CHEN_THRESHOLD_MV_DEFAULT              16000

#define BPSM_CHEN_TOGGLE_PERIOD_SECONDS_MIN         60
#define BPSM_CHEN_TOGGLE_PERIOD_SECONDS_MAX         86400
#define BPSM_CHEN_TOGGLE_PERIOD_SECONDS_DEFAULT     3600
#define BPSM_CHEN_TOGGLE_DURATION_SECONDS           1

#define BPSM_XVF_THRESHOLD_MV_MAX                   60000
#define BPSM_XVF_UPDATE_PERIOD_SECONDS              5

#ifdef BPSM_BKEN_FORCED_HARDWARE
#define BPSM_FLAG_BKFH                              0b1
#else
#define BPSM_FLAG_BKFH                              0b0
#endif
#ifdef BPSM_CHST_FORCED_HARDWARE
#define BPSM_FLAG_CHST                              0b1
#else
#define BPSM_FLAG_CHST                              0b0
#endif
#ifdef BPSM_CHEN_FORCED_HARDWARE
#define BPSM_FLAG_CHEN                              0b1
#else
#define BPSM_FLAG_CHEN                              0b0
#endif

/*** BPSM local structures ***/

/*******************************************************************/
typedef struct {
    UNA_bit_representation_t chenst;
    UNA_bit_representation_t bkenst;
    uint32_t lvf_cvf_update_next_time_seconds;
#ifndef BPSM_CHEN_FORCED_HARDWARE
    int32_t vsrc_mv;
    uint32_t chen_toggle_previous_time_seconds;
    uint32_t chen_toggle_next_time_seconds;
#endif
} BPSM_context_t;

/*** BPSM local global variables ***/

static BPSM_context_t bpsm_ctx = {
    .chenst = UNA_BIT_ERROR,
    .bkenst = UNA_BIT_ERROR,
    .lvf_cvf_update_next_time_seconds = 0,
#ifndef BPSM_CHEN_FORCED_HARDWARE
    .vsrc_mv = 0,
    .chen_toggle_previous_time_seconds = 0,
    .chen_toggle_next_time_seconds = 0,
#endif
};

/*** BPSM functions ***/

/*******************************************************************/
NODE_status_t BPSM_init(void) {
    // Local variables.
    NODE_status_t status = NODE_SUCCESS;
    // Init context.
    bpsm_ctx.chenst = UNA_BIT_ERROR;
    bpsm_ctx.bkenst = UNA_BIT_ERROR;
    bpsm_ctx.lvf_cvf_update_next_time_seconds = 0;
#ifndef BPSM_CHEN_FORCED_HARDWARE
    bpsm_ctx.vsrc_mv = 0;
    bpsm_ctx.chen_toggle_previous_time_seconds = 0;
    bpsm_ctx.chen_toggle_next_time_seconds = 0;
#endif
    return status;
}

/*******************************************************************/
void BPSM_init_register(uint8_t reg_addr, uint32_t* reg_value) {
    // Local variables.
    uint32_t unused_mask = 0;
    // Check address.
    switch (reg_addr) {
    case BPSM_REGISTER_ADDRESS_FLAGS_1:
        SWREG_write_field(reg_value, &unused_mask, BPSM_FLAG_BKFH, BPSM_REGISTER_FLAGS_1_MASK_BKFH);
        SWREG_write_field(reg_value, &unused_mask, BPSM_FLAG_CHST, BPSM_REGISTER_FLAGS_1_MASK_CSFH);
        SWREG_write_field(reg_value, &unused_mask, BPSM_FLAG_CHEN, BPSM_REGISTER_FLAGS_1_MASK_CEFH);
        break;
#ifdef DSM_NVM_FACTORY_RESET
    case BPSM_REGISTER_ADDRESS_CONFIGURATION_0:
        SWREG_write_field(reg_value, &unused_mask, UNA_convert_seconds(BPSM_CHEN_TOGGLE_PERIOD_SECONDS), BPSM_REGISTER_CONFIGURATION_0_MASK_CHEN_TOGGLE_PERIOD);
        SWREG_write_field(reg_value, &unused_mask, UNA_convert_mv(BPSM_CHEN_VSRC_THRESHOLD_MV), BPSM_REGISTER_CONFIGURATION_0_MASK_CHEN_THRESHOLD);
        break;
    case BPSM_REGISTER_ADDRESS_CONFIGURATION_1:
        SWREG_write_field(reg_value, &unused_mask, UNA_convert_mv(BPSM_LVF_LOW_THRESHOLD_MV), BPSM_REGISTER_CONFIGURATION_1_MASK_LVF_LOW_THRESHOLD);
        SWREG_write_field(reg_value, &unused_mask, UNA_convert_mv(BPSM_LVF_HIGH_THRESHOLD_MV), BPSM_REGISTER_CONFIGURATION_1_MASK_LVF_HIGH_THRESHOLD);
        break;
    case BPSM_REGISTER_ADDRESS_CONFIGURATION_2:
        SWREG_write_field(reg_value, &unused_mask, UNA_convert_mv(BPSM_CVF_LOW_THRESHOLD_MV), BPSM_REGISTER_CONFIGURATION_2_MASK_CVF_LOW_THRESHOLD);
        SWREG_write_field(reg_value, &unused_mask, UNA_convert_mv(BPSM_CVF_HIGH_THRESHOLD_MV), BPSM_REGISTER_CONFIGURATION_2_MASK_CVF_HIGH_THRESHOLD);
        break;
#endif
    default:
        break;
    }
}

/*******************************************************************/
void BPSM_refresh_register(uint8_t reg_addr) {
    // Local variables.
    uint32_t* reg_ptr = &(NODE_RAM_REGISTER[reg_addr]);
    UNA_bit_representation_t chrgst = UNA_BIT_ERROR;
    uint32_t unused_mask = 0;
    // Check address.
    switch (reg_addr) {
    case BPSM_REGISTER_ADDRESS_STATUS_1:
        // Charge status.
#ifdef BPSM_CHST_FORCED_HARDWARE
        chrgst = UNA_BIT_FORCED_HARDWARE;
#else
        chrgst = LOAD_get_charge_status();
#endif
        // Charge state.
#ifdef BPSM_CHEN_FORCED_HARDWARE
        bpsm_ctx.chenst = UNA_BIT_FORCED_HARDWARE;
#else
        bpsm_ctx.chenst = LOAD_get_charge_state();
#endif
        // Backup_output state.
#ifdef BPSM_BKEN_FORCED_HARDWARE
        bpsm_ctx.bkenst = UNA_BIT_FORCED_HARDWARE;
#else
        switch (LOAD_get_output_state()) {
        case 0:
            bpsm_ctx.bkenst = UNA_BIT_0;
            break;
        case 1:
            bpsm_ctx.bkenst = UNA_BIT_1;
            break;
        default:
            bpsm_ctx.bkenst = UNA_BIT_ERROR;
            break;
        }
#endif
        SWREG_write_field(reg_ptr, &unused_mask, ((uint32_t) chrgst), BPSM_REGISTER_STATUS_1_MASK_CHRGST);
        SWREG_write_field(reg_ptr, &unused_mask, ((uint32_t) bpsm_ctx.chenst), BPSM_REGISTER_STATUS_1_MASK_CHENST);
        SWREG_write_field(reg_ptr, &unused_mask, ((uint32_t) bpsm_ctx.bkenst), BPSM_REGISTER_STATUS_1_MASK_BKENST);
        break;
    default:
        // Nothing to do for other registers.
        break;
    }
}

/*******************************************************************/
NODE_status_t BPSM_secure_register(uint8_t reg_addr, uint32_t new_reg_value, uint32_t* reg_mask, uint32_t* reg_value) {
    // Local variables.
    NODE_status_t status = NODE_SUCCESS;
    uint32_t low_threshold_mv = 0;
    uint32_t high_threshold_mv = 0;
    uint32_t generic_u32 = 0;
    // Check address.
    switch (reg_addr) {
    case BPSM_REGISTER_ADDRESS_CONFIGURATION_0:
        SWREG_secure_field(
            BPSM_REGISTER_CONFIGURATION_0_MASK_CHEN_THRESHOLD,
            UNA_get_mv,
            UNA_convert_mv,
            > BPSM_CHEN_THRESHOLD_MV_MAX,
            > BPSM_CHEN_THRESHOLD_MV_MAX,
            BPSM_CHEN_THRESHOLD_MV_DEFAULT,
            status = NODE_ERROR_REGISTER_FIELD_VALUE
        );
        SWREG_secure_field(
            BPSM_REGISTER_CONFIGURATION_0_MASK_CHEN_TOGGLE_PERIOD,
            UNA_get_seconds,
            UNA_convert_seconds,
            < BPSM_CHEN_TOGGLE_PERIOD_SECONDS_MIN,
            > BPSM_CHEN_TOGGLE_PERIOD_SECONDS_MAX,
            BPSM_CHEN_TOGGLE_PERIOD_SECONDS_DEFAULT,
            status = NODE_ERROR_REGISTER_FIELD_VALUE
        );
        break;
    case BPSM_REGISTER_ADDRESS_CONFIGURATION_1:
        low_threshold_mv = UNA_get_mv(SWREG_read_field(new_reg_value, BPSM_REGISTER_CONFIGURATION_1_MASK_LVF_LOW_THRESHOLD));
        high_threshold_mv = UNA_get_mv(SWREG_read_field(new_reg_value, BPSM_REGISTER_CONFIGURATION_1_MASK_LVF_HIGH_THRESHOLD));
        SWREG_secure_field(
            BPSM_REGISTER_CONFIGURATION_1_MASK_LVF_LOW_THRESHOLD,
            UNA_get_mv,
            UNA_convert_mv,
            > BPSM_XVF_THRESHOLD_MV_MAX,
            > high_threshold_mv,
            0,
            status = NODE_ERROR_REGISTER_FIELD_VALUE
        );
        SWREG_secure_field(
            BPSM_REGISTER_CONFIGURATION_1_MASK_LVF_HIGH_THRESHOLD,
            UNA_get_mv,
            UNA_convert_mv,
            > BPSM_XVF_THRESHOLD_MV_MAX,
            < low_threshold_mv,
            0,
            status = NODE_ERROR_REGISTER_FIELD_VALUE
        );
        break;
    case BPSM_REGISTER_ADDRESS_CONFIGURATION_2:
        low_threshold_mv = UNA_get_mv(SWREG_read_field(new_reg_value, BPSM_REGISTER_CONFIGURATION_2_MASK_CVF_LOW_THRESHOLD));
        high_threshold_mv = UNA_get_mv(SWREG_read_field(new_reg_value, BPSM_REGISTER_CONFIGURATION_2_MASK_CVF_HIGH_THRESHOLD));
        SWREG_secure_field(
            BPSM_REGISTER_CONFIGURATION_2_MASK_CVF_LOW_THRESHOLD,
            UNA_get_mv,
            UNA_convert_mv,
            > BPSM_XVF_THRESHOLD_MV_MAX,
            > high_threshold_mv,
            0,
            status = NODE_ERROR_REGISTER_FIELD_VALUE
        );
        SWREG_secure_field(
            BPSM_REGISTER_CONFIGURATION_2_MASK_CVF_HIGH_THRESHOLD,
            UNA_get_mv,
            UNA_convert_mv,
            > BPSM_XVF_THRESHOLD_MV_MAX,
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
NODE_status_t BPSM_process_register(uint8_t reg_addr, uint32_t reg_mask) {
    // Local variables.
    NODE_status_t status = NODE_SUCCESS;
    uint32_t* reg_ptr = &(NODE_RAM_REGISTER[reg_addr]);
#ifndef BPSM_BKEN_FORCED_HARDWARE
    LOAD_status_t load_status = LOAD_SUCCESS;
    UNA_bit_representation_t bken = UNA_BIT_ERROR;
#endif
#ifndef BPSM_CHEN_FORCED_HARDWARE
    UNA_bit_representation_t chen = UNA_BIT_ERROR;
#endif
    // Check address.
    switch (reg_addr) {
    case BPSM_REGISTER_ADDRESS_CONTROL_1:
        // CHEN.
        if ((reg_mask & BPSM_REGISTER_CONTROL_1_MASK_CHEN) != 0) {
#ifdef BPSM_CHEN_FORCED_HARDWARE
            status = NODE_ERROR_FORCED_HARDWARE;
            goto errors;
#else
            // Check control mode.
            if (SWREG_read_field((*reg_ptr), BPSM_REGISTER_CONTROL_1_MASK_CHMD) != 0) {
                // Read bit.
                chen = SWREG_read_field((*reg_ptr), BPSM_REGISTER_CONTROL_1_MASK_CHEN);
                // Compare to current state.
                if (chen != bpsm_ctx.chenst) {
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
        if ((reg_mask & BPSM_REGISTER_CONTROL_1_MASK_BKEN) != 0) {
            // Check pin mode.
#ifdef BPSM_BKEN_FORCED_HARDWARE
            status = NODE_ERROR_FORCED_HARDWARE;
            goto errors;
#else
            // Read bit.
            bken = SWREG_read_field((*reg_ptr), BPSM_REGISTER_CONTROL_1_MASK_BKEN);
            // Compare to current state.
            if (bken != bpsm_ctx.bkenst) {
                // Set output state.
                load_status = LOAD_set_output_state(bken);
                LOAD_exit_error(NODE_ERROR_BASE_LOAD);
            }
#endif
        }
        break;
    default:
        break;
    }
errors:
    BPSM_refresh_register(BPSM_REGISTER_ADDRESS_STATUS_1);
    return status;
}

/*******************************************************************/
NODE_status_t BPSM_mtrg_callback(void) {
    // Local variables.
    NODE_status_t status = NODE_SUCCESS;
    ANALOG_status_t analog_status = ANALOG_SUCCESS;
    uint32_t* reg_analog_data_1_ptr = &(NODE_RAM_REGISTER[BPSM_REGISTER_ADDRESS_ANALOG_DATA_1]);
    uint32_t* reg_analog_data_2_ptr = &(NODE_RAM_REGISTER[BPSM_REGISTER_ADDRESS_ANALOG_DATA_2]);
    int32_t adc_data = 0;
    uint32_t unused_mask = 0;
    // Reset data.
    (*reg_analog_data_1_ptr) = NODE_REGISTER[BPSM_REGISTER_ADDRESS_ANALOG_DATA_1].error_value;
    (*reg_analog_data_2_ptr) = NODE_REGISTER[BPSM_REGISTER_ADDRESS_ANALOG_DATA_2].error_value;
    // Source voltage.
    analog_status = ANALOG_convert_channel(ANALOG_CHANNEL_VSRC_MV, &adc_data);
    ANALOG_exit_error(NODE_ERROR_BASE_ANALOG);
    SWREG_write_field(reg_analog_data_1_ptr, &unused_mask, UNA_convert_mv(adc_data), BPSM_REGISTER_ANALOG_DATA_1_MASK_VSRC);
    // Storage element voltage.
    analog_status = ANALOG_convert_channel(ANALOG_CHANNEL_VSTR_MV, &adc_data);
    ANALOG_exit_error(NODE_ERROR_BASE_ANALOG);
    SWREG_write_field(reg_analog_data_1_ptr, &unused_mask, UNA_convert_mv(adc_data), BPSM_REGISTER_ANALOG_DATA_1_MASK_VSTR);
    // Backup output voltage.
    analog_status = ANALOG_convert_channel(ANALOG_CHANNEL_VBKP_MV, &adc_data);
    ANALOG_exit_error(NODE_ERROR_BASE_ANALOG);
    SWREG_write_field(reg_analog_data_2_ptr, &unused_mask, UNA_convert_mv(adc_data), BPSM_REGISTER_ANALOG_DATA_2_MASK_VBKP);
errors:
    return status;
}

/*******************************************************************/
NODE_status_t BPSM_low_voltage_detector_process(void) {
    // Local variables.
    NODE_status_t status = NODE_SUCCESS;
    ANALOG_status_t analog_status = ANALOG_SUCCESS;
    uint32_t uptime_seconds = RTC_get_uptime_seconds();
    uint32_t* reg_config_1_ptr = &(NODE_RAM_REGISTER[BPSM_REGISTER_ADDRESS_ANALOG_DATA_1]);
    uint32_t* reg_config_2_ptr = &(NODE_RAM_REGISTER[BPSM_REGISTER_ADDRESS_ANALOG_DATA_2]);
    uint32_t* reg_status_1_ptr = &(NODE_RAM_REGISTER[BPSM_REGISTER_ADDRESS_STATUS_1]);
    int32_t vstr_mv = 0;
    uint32_t unused_mask = 0;
    // Check period.
    if (uptime_seconds >= bpsm_ctx.lvf_cvf_update_next_time_seconds) {
        // Update next time.
        bpsm_ctx.lvf_cvf_update_next_time_seconds = uptime_seconds + BPSM_XVF_UPDATE_PERIOD_SECONDS;
        // Turn analog front-end on.
        POWER_enable(POWER_REQUESTER_ID_BPSM, POWER_DOMAIN_ANALOG, LPTIM_DELAY_MODE_ACTIVE);
        // Read source voltage.
        analog_status = ANALOG_convert_channel(ANALOG_CHANNEL_VSTR_MV, &vstr_mv);
        ANALOG_exit_error(NODE_ERROR_BASE_ANALOG);
#ifndef BPSM_CHEN_FORCED_HARDWARE
        analog_status = ANALOG_convert_channel(ANALOG_CHANNEL_VSRC_MV, &(bpsm_ctx.vsrc_mv));
        ANALOG_exit_error(NODE_ERROR_BASE_ANALOG);
#endif
        // Update LVF flag.
        if (vstr_mv < UNA_get_mv(SWREG_read_field((*reg_config_1_ptr), BPSM_REGISTER_CONFIGURATION_1_MASK_LVF_LOW_THRESHOLD))) {
            SWREG_write_field(reg_status_1_ptr, &unused_mask, 0b1, BPSM_REGISTER_STATUS_1_MASK_LVF);
        }
        if (vstr_mv > UNA_get_mv(SWREG_read_field((*reg_config_1_ptr), BPSM_REGISTER_CONFIGURATION_1_MASK_LVF_HIGH_THRESHOLD))) {
            SWREG_write_field(reg_status_1_ptr, &unused_mask, 0b0, BPSM_REGISTER_STATUS_1_MASK_LVF);
        }
        // Update CVF flag.
        if (vstr_mv < UNA_get_mv(SWREG_read_field((*reg_config_2_ptr), BPSM_REGISTER_CONFIGURATION_2_MASK_CVF_LOW_THRESHOLD))) {
            SWREG_write_field(reg_status_1_ptr, &unused_mask, 0b1, BPSM_REGISTER_STATUS_1_MASK_CVF);
        }
        if (vstr_mv > UNA_get_mv(SWREG_read_field((*reg_config_2_ptr), BPSM_REGISTER_CONFIGURATION_2_MASK_CVF_HIGH_THRESHOLD))) {
            SWREG_write_field(reg_status_1_ptr, &unused_mask, 0b0, BPSM_REGISTER_STATUS_1_MASK_CVF);
        }
    }
errors:
    POWER_disable(POWER_REQUESTER_ID_BPSM, POWER_DOMAIN_ANALOG);
    return status;
}

#ifndef BPSM_CHEN_FORCED_HARDWARE
/*******************************************************************/
NODE_status_t BPSM_charge_process(void) {
    // Local variables.
    NODE_status_t status = NODE_SUCCESS;
    uint32_t reg_control_1 = NODE_RAM_REGISTER[BPSM_REGISTER_ADDRESS_CONTROL_1];
    uint32_t reg_config_0 = NODE_RAM_REGISTER[BPSM_REGISTER_ADDRESS_CONFIGURATION_0];
    uint32_t uptime_seconds = RTC_get_uptime_seconds();
    // Check mode.
    if (SWREG_read_field(reg_control_1, BPSM_REGISTER_CONTROL_1_MASK_CHMD) != 0) goto errors;
    // Check toggle period.
    if (uptime_seconds >= bpsm_ctx.chen_toggle_next_time_seconds) {
        // Update times.
        bpsm_ctx.chen_toggle_previous_time_seconds = uptime_seconds;
        bpsm_ctx.chen_toggle_next_time_seconds = uptime_seconds + ((uint32_t) UNA_get_seconds(SWREG_read_field(reg_config_0, BPSM_REGISTER_CONFIGURATION_0_MASK_CHEN_TOGGLE_PERIOD)));
        // Disable charge.
        LOAD_set_charge_state(0);
    }
    if (uptime_seconds >= (bpsm_ctx.chen_toggle_previous_time_seconds + BPSM_CHEN_TOGGLE_DURATION_SECONDS)) {
        // Check voltage.
        if (bpsm_ctx.vsrc_mv >= UNA_get_mv(SWREG_read_field(reg_config_0, BPSM_REGISTER_CONFIGURATION_0_MASK_CHEN_THRESHOLD))) {
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

#endif /* BPSM */

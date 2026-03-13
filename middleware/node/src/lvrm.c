/*
 * lvrm.c
 *
 *  Created on: 04 jun. 2023
 *      Author: Ludo
 */

#include "lvrm.h"

#ifdef LVRM

#include "adc.h"
#include "dsm_flags.h"
#include "dsm_flags_slave.h"
#include "error.h"
#include "load.h"
#include "lvrm_registers.h"
#include "node_register.h"
#include "node_status.h"
#include "rtc.h"
#include "swreg.h"
#include "types.h"
#include "una.h"

/*** LVRM local macros ***/

// Note: output current measurement uses LT6106, OPA187 and optionally TMUX7219 chips whose minimum operating voltage is 4.5V.
#define LVRM_OUTPUT_CURRENT_MEASUREMENT_POWER_TH_MV     4500

#define LVRM_OUTPUT_CURRENT_OFFSET_UA_MAX               100000
#define LVRM_OUTPUT_CURRENT_OFFSET_UA_DEFAULT           0

#ifdef LVRM_MODE_BMS
#define LVRM_FLAG_BMSF                                  0b1
#define LVRM_BMS_INPUT_VOLTAGE_THX_MV_MAX               60000
#define LVRM_BMS_PROCESS_PERIOD_SECONDS                 60
#else
#define LVRM_FLAG_BMSF                                  0b0
#endif
#ifdef LVRM_RELAY_CONTROL_FORCED_HARDWARE
#define LVRM_FLAG_RCFH                                  0b1
#else
#define LVRM_FLAG_RCFH                                  0b0
#endif

/*** LVRM local structures ***/

/*******************************************************************/
typedef struct {
    UNA_bit_representation_t regulator_control_state;
#ifdef LVRM_MODE_BMS
    uint32_t bms_process_next_time_seconds;
#endif
} LVRM_context_t;

/*** LVRM local global variables ***/

static LVRM_context_t lvrm_ctx = {
    .regulator_control_state = UNA_BIT_ERROR,
#ifdef LVRM_MODE_BMS
    .bms_process_next_time_seconds = 0,
#endif
};

/*** LVRM functions ***/

/*******************************************************************/
NODE_status_t LVRM_init(void) {
    // Local variables.
    NODE_status_t status = NODE_SUCCESS;
    // Init context.
    lvrm_ctx.regulator_control_state = UNA_BIT_ERROR;
#ifdef LVRM_MODE_BMS
    lvrm_ctx.bms_process_next_time_seconds = 0;
#endif
    return status;
}

/*******************************************************************/
void LVRM_init_register(uint8_t reg_addr, uint32_t* reg_value) {
    // Local variables.
    uint32_t unused_mask = 0;
    // Check address.
    switch (reg_addr) {
    case LVRM_REGISTER_ADDRESS_FLAGS_1:
        SWREG_write_field(reg_value, &unused_mask, LVRM_FLAG_BMSF, LVRM_REGISTER_FLAGS_1_MASK_BMSF);
        SWREG_write_field(reg_value, &unused_mask, LVRM_FLAG_RCFH, LVRM_REGISTER_FLAGS_1_MASK_RCFH);
        break;
#ifdef DSM_NVM_FACTORY_RESET
    case LVRM_REGISTER_ADDRESS_CONFIGURATION_0:
        SWREG_write_field(reg_value, &unused_mask, UNA_convert_mv(LVRM_BMS_INPUT_VOLTAGE_THL_MV), LVRM_REGISTER_CONFIGURATION_0_MASK_BMS_INPUT_VOLTAGE_THL);
        SWREG_write_field(reg_value, &unused_mask, UNA_convert_mv(LVRM_BMS_INPUT_VOLTAGE_THH_MV), LVRM_REGISTER_CONFIGURATION_0_MASK_BMS_INPUT_VOLTAGE_THH);
        break;
    case LVRM_REGISTER_ADDRESS_CONFIGURATION_1:
        SWREG_write_field(reg_value, &unused_mask, UNA_convert_ua(LVRM_OUTPUT_CURRENT_OFFSET_UA_DEFAULT), LVRM_REGISTER_CONFIGURATION_1_MASK_OUTPUT_CURRENT_OFFSET);
        break;
#endif
    default:
        break;
    }
}

/*******************************************************************/
void LVRM_refresh_register(uint8_t reg_addr) {
    // Local variables.
    uint32_t* reg_ptr = &(NODE_RAM_REGISTER[reg_addr]);
    uint32_t unused_mask = 0;
    // Check address.
    switch (reg_addr) {
    case LVRM_REGISTER_ADDRESS_STATUS_1:
        // Relay state.
#ifdef LVRM_RELAY_CONTROL_FORCED_HARDWARE
        lvrm_ctx.regulator_control_state = UNA_BIT_FORCED_HARDWARE;
#else
        switch (LOAD_get_output_state()) {
        case 0:
            lvrm_ctx.regulator_control_state = UNA_BIT_0;
            break;
        case 1:
            lvrm_ctx.regulator_control_state = UNA_BIT_1;
            break;
        default:
            lvrm_ctx.regulator_control_state = UNA_BIT_ERROR;
            break;
        }
#endif
        SWREG_write_field(reg_ptr, &unused_mask, ((uint32_t) lvrm_ctx.regulator_control_state), LVRM_REGISTER_STATUS_1_MASK_RCS);
        break;
    default:
        break;
    }
}

/*******************************************************************/
NODE_status_t LVRM_secure_register(uint8_t reg_addr, uint32_t new_reg_value, uint32_t* reg_mask, uint32_t* reg_value) {
    // Local variables.
    NODE_status_t status = NODE_SUCCESS;
    int32_t generic_s32 = 0;
    uint32_t generic_u32 = 0;
#ifdef LVRM_MODE_BMS
    int32_t low_threshold_mv = 0;
    int32_t high_threshold_mv = 0;
#endif
    // Check address.
    switch (reg_addr) {
#ifdef LVRM_MODE_BMS
    case LVRM_REGISTER_ADDRESS_CONFIGURATION_0:
        low_threshold_mv = UNA_convert_mv(SWREG_read_field(new_reg_value, LVRM_REGISTER_CONFIGURATION_0_MASK_BMS_INPUT_VOLTAGE_THL));
        high_threshold_mv = UNA_convert_mv(SWREG_read_field(new_reg_value, LVRM_REGISTER_CONFIGURATION_0_MASK_BMS_INPUT_VOLTAGE_THH));
        SWREG_secure_field(
            LVRM_REGISTER_CONFIGURATION_0_MASK_BMS_INPUT_VOLTAGE_THL,
            UNA_get_mv,
            UNA_convert_mv,
            > LVRM_BMS_INPUT_VOLTAGE_THX_MV_MAX,
            > high_threshold_mv,
            0,
            status = NODE_ERROR_REGISTER_FIELD_VALUE
        );
        SWREG_secure_field(
            LVRM_REGISTER_CONFIGURATION_0_MASK_BMS_INPUT_VOLTAGE_THH,
            UNA_get_mv,
            UNA_convert_mv,
            > LVRM_BMS_INPUT_VOLTAGE_THX_MV_MAX,
            < low_threshold_mv,
            0,
            status = NODE_ERROR_REGISTER_FIELD_VALUE
        );
        break;
#endif
    case LVRM_REGISTER_ADDRESS_CONFIGURATION_1:
        SWREG_secure_field(
            LVRM_REGISTER_CONFIGURATION_1_MASK_OUTPUT_CURRENT_OFFSET,
            UNA_get_ua,
            UNA_convert_ua,
            > LVRM_OUTPUT_CURRENT_OFFSET_UA_MAX,
            > LVRM_OUTPUT_CURRENT_OFFSET_UA_MAX,
            LVRM_OUTPUT_CURRENT_OFFSET_UA_DEFAULT,
            status = NODE_ERROR_REGISTER_FIELD_VALUE
        );
        break;
    default:
        break;
    }
    return status;
}

/*******************************************************************/
NODE_status_t LVRM_process_register(uint8_t reg_addr, uint32_t reg_mask) {
    // Local variables.
    NODE_status_t status = NODE_SUCCESS;
#if !(defined LVRM_RELAY_CONTROL_FORCED_HARDWARE) && !(defined LVRM_MODE_BMS)
    uint32_t* reg_ptr = &(NODE_RAM_REGISTER[reg_addr]);
    LOAD_status_t load_status = LOAD_SUCCESS;
    UNA_bit_representation_t rlst = UNA_BIT_ERROR;
#endif
    // Check address.
    switch (reg_addr) {
    case LVRM_REGISTER_ADDRESS_CONTROL_1:
        // RLST.
        if ((reg_mask & LVRM_REGISTER_CONTROL_1_MASK_RC) != 0) {
            // Check pin mode.
#ifdef LVRM_RELAY_CONTROL_FORCED_HARDWARE
            status = NODE_ERROR_FORCED_HARDWARE;
            goto errors;
#else
#ifdef LVRM_MODE_BMS
            status = NODE_ERROR_FORCED_SOFTWARE;
            goto errors;
#else
            // Read bit.
            rlst = SWREG_read_field((*reg_ptr), LVRM_REGISTER_CONTROL_1_MASK_RC);
            // Compare to current state.
            if (rlst != lvrm_ctx.regulator_control_state) {
                // Set relay state.
                load_status = LOAD_set_output_state(rlst);
                LOAD_exit_error(NODE_ERROR_BASE_LOAD);
            }
#endif
#endif
        }
        break;
    default:
        break;
    }
errors:
    LVRM_refresh_register(LVRM_REGISTER_ADDRESS_STATUS_1);
    return status;
}

/*******************************************************************/
NODE_status_t LVRM_mtrg_callback(void) {
    // Local variables.
    NODE_status_t status = NODE_SUCCESS;
    ANALOG_status_t analog_status = ANALOG_SUCCESS;
    uint32_t* reg_analog_data_1_ptr = &(NODE_RAM_REGISTER[LVRM_REGISTER_ADDRESS_ANALOG_DATA_1]);
    uint32_t* reg_analog_data_2_ptr = &(NODE_RAM_REGISTER[LVRM_REGISTER_ADDRESS_ANALOG_DATA_2]);
    int32_t input_voltage_mv = 0;
    int32_t output_voltage_mv = 0;
    int32_t output_current_ua = 0;
    uint32_t unused_mask = 0;
    // Reset data.
    (*reg_analog_data_1_ptr) = NODE_REGISTER[LVRM_REGISTER_ADDRESS_ANALOG_DATA_1].error_value;
    (*reg_analog_data_2_ptr) = NODE_REGISTER[LVRM_REGISTER_ADDRESS_ANALOG_DATA_2].error_value;
    // Relay common voltage.
    analog_status = ANALOG_convert_channel(ANALOG_CHANNEL_INPUT_VOLTAGE_MV, &input_voltage_mv);
    ANALOG_exit_error(NODE_ERROR_BASE_ANALOG);
    SWREG_write_field(reg_analog_data_1_ptr, &unused_mask, UNA_convert_mv(input_voltage_mv), LVRM_REGISTER_ANALOG_DATA_1_MASK_INPUT_VOLTAGE);
    // Relay output voltage.
    analog_status = ANALOG_convert_channel(ANALOG_CHANNEL_OUTPUT_VOLTAGE_MV, &output_voltage_mv);
    ANALOG_exit_error(NODE_ERROR_BASE_ANALOG);
    SWREG_write_field(reg_analog_data_1_ptr, &unused_mask, UNA_convert_mv(output_voltage_mv), LVRM_REGISTER_ANALOG_DATA_1_MASK_OUTPUT_VOLTAGE);
    // Check output current measurement validity.
    if (input_voltage_mv >= LVRM_OUTPUT_CURRENT_MEASUREMENT_POWER_TH_MV) {
        // Relay output current.
        analog_status = ANALOG_convert_channel(ANALOG_CHANNEL_OUTPUT_CURRENT_UA, &output_current_ua);
        ANALOG_exit_error(NODE_ERROR_BASE_ANALOG);
    }
    SWREG_write_field(reg_analog_data_2_ptr, &unused_mask, UNA_convert_ua(output_current_ua), LVRM_REGISTER_ANALOG_DATA_2_MASK_OUTPUT_CURRENT);
errors:
    return status;
}

#ifdef LVRM_MODE_BMS
/*******************************************************************/
NODE_status_t LVRM_bms_process(void) {
    // Local variables.
    NODE_status_t status = NODE_SUCCESS;
    ANALOG_status_t analog_status = ANALOG_SUCCESS;
    LOAD_status_t load_status = LOAD_SUCCESS;
    uint32_t reg_config_0 = NODE_RAM_REGISTER[LVRM_REGISTER_ADDRESS_CONFIGURATION_0];
    int32_t vbatt_mv = 0;
    uint32_t uptime_seconds = RTC_get_uptime_seconds();
    // Check period.
    if (uptime_seconds >= lvrm_ctx.bms_process_next_time_seconds) {
        // Update next time.
        lvrm_ctx.bms_process_next_time_seconds = uptime_seconds + LVRM_BMS_PROCESS_PERIOD_SECONDS;
        // Turn analog front-end on.
        POWER_enable(POWER_REQUESTER_ID_LVRM, POWER_DOMAIN_ANALOG, LPTIM_DELAY_MODE_ACTIVE);
        // Check battery voltage.
        analog_status = ANALOG_convert_channel(ANALOG_CHANNEL_INPUT_VOLTAGE_MV, &vbatt_mv);
        ANALOG_exit_error(NODE_ERROR_BASE_ANALOG);
        // Check battery voltage.
        if (vbatt_mv < UNA_get_mv(SWREG_read_field(reg_config_0, LVRM_REGISTER_CONFIGURATION_0_MASK_BMS_INPUT_VOLTAGE_THL))) {
            // Open relay.
            load_status = LOAD_set_output_state(0);
            LOAD_exit_error(NODE_ERROR_BASE_LOAD);
        }
        if (vbatt_mv > UNA_get_mv(SWREG_read_field(reg_config_0, LVRM_REGISTER_CONFIGURATION_0_MASK_BMS_INPUT_VOLTAGE_THH))) {
            // Close relay.
            load_status = LOAD_set_output_state(1);
            LOAD_exit_error(NODE_ERROR_BASE_LOAD);
        }
    }
errors:
    POWER_disable(POWER_REQUESTER_ID_LVRM, POWER_DOMAIN_ANALOG);
    return status;
}
#endif

#endif /* LVRM */

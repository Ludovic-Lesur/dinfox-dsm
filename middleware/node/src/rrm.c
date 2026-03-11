/*
 * rrm.c
 *
 *  Created on: 10 jun. 2023
 *      Author: Ludo
 */

#include "rrm.h"

#ifdef RRM

#include "analog.h"
#include "dsm_flags.h"
#include "error.h"
#include "load.h"
#include "rrm_registers.h"
#include "node_register.h"
#include "node_status.h"
#include "swreg.h"
#include "types.h"
#include "una.h"

/*** RRM local macros ***/

// Note: output current measurement uses LT6106 and OPA187 chips whose minimum operating voltage is 4.5V.
#define RRM_OUTPUT_CURRENT_MEASUREMENT_POWER_TH_MV  4500

#define RRM_OUTPUT_CURRENT_OFFSET_UA_MAX            100000
#define RRM_OUTPUT_CURRENT_OFFSET_UA_DEFAULT        0

#ifdef RRM_REGULATOR_CONTROL_FORCED_HARDWARE
#define RRM_FLAG_RCFH                               0b1
#else
#define RRM_FLAG_RCFH                               0b0
#endif

/*** RRM local structures ***/

/*******************************************************************/
typedef struct {
    UNA_bit_representation_t regulator_control_state;
} RRM_context_t;

/*** RRM local global variables ***/

static RRM_context_t rrm_ctx = {
    .regulator_control_state = UNA_BIT_ERROR
};

/*** RRM functions ***/

/*******************************************************************/
NODE_status_t RRM_init(void) {
    // Local variables.
    NODE_status_t status = NODE_SUCCESS;
    // Init context.
    rrm_ctx.regulator_control_state = UNA_BIT_ERROR;
    return status;
}

/*******************************************************************/
void RRM_init_register(uint8_t reg_addr, uint32_t* reg_value) {
    // Local variables.
    uint32_t unused_mask = 0;
    // Check address.
    switch (reg_addr) {
    case RRM_REGISTER_ADDRESS_FLAGS_1:
        SWREG_write_field(reg_value, &unused_mask, RRM_FLAG_RCFH, RRM_REGISTER_FLAGS_1_MASK_RCFH);
        break;
#ifdef DSM_NVM_FACTORY_RESET
    case RRM_REGISTER_ADDRESS_CONFIGURATION_0:
        SWREG_write_field(reg_value, &unused_mask, UNA_convert_ua(RRM_OUTPUT_CURRENT_OFFSET_UA_DEFAULT), RRM_REGISTER_CONFIGURATION_0_MASK_OUTPUT_CURRENT_OFFSET);
        break;
#endif
    default:
        break;
    }
}

/*******************************************************************/
void RRM_refresh_register(uint8_t reg_addr) {
    // Local variables.
    uint32_t* reg_ptr = &(NODE_RAM_REGISTER[reg_addr]);
    uint32_t unused_mask = 0;
    // Check address.
    switch (reg_addr) {
    case RRM_REGISTER_ADDRESS_STATUS_1:
        // Regulator state.
#ifdef RRM_REGULATOR_CONTROL_FORCED_HARDWARE
        rrm_ctx.regulator_control_state = UNA_BIT_FORCED_HARDWARE;
#else
        switch (LOAD_get_output_state()) {
        case 0:
            rrm_ctx.regulator_control_state = UNA_BIT_0;
            break;
        case 1:
            rrm_ctx.regulator_control_state = UNA_BIT_1;
            break;
        default:
            rrm_ctx.regulator_control_state = UNA_BIT_ERROR;
            break;
        }
#endif
        SWREG_write_field(reg_ptr, &unused_mask, ((uint32_t) rrm_ctx.regulator_control_state), RRM_REGISTER_STATUS_1_MASK_RCS);
        break;
    default:
        break;
    }
}

/*******************************************************************/
NODE_status_t RRM_secure_register(uint8_t reg_addr, uint32_t new_reg_value, uint32_t* reg_mask, uint32_t* reg_value) {
    // Local variables.
    NODE_status_t status = NODE_SUCCESS;
    int32_t generic_s32 = 0;
    uint32_t generic_u32 = 0;
    // Check address.
    switch (reg_addr) {
    case RRM_REGISTER_ADDRESS_CONFIGURATION_0:
        SWREG_secure_field(
            RRM_REGISTER_CONFIGURATION_0_MASK_OUTPUT_CURRENT_OFFSET,
            UNA_get_ua,
            UNA_convert_ua,
            > RRM_OUTPUT_CURRENT_OFFSET_UA_MAX,
            > RRM_OUTPUT_CURRENT_OFFSET_UA_MAX,
            RRM_OUTPUT_CURRENT_OFFSET_UA_DEFAULT,
            status = NODE_ERROR_REGISTER_FIELD_VALUE
        );
        break;
    default:
        break;
    }
    return status;
}

/*******************************************************************/
NODE_status_t RRM_process_register(uint8_t reg_addr, uint32_t reg_mask) {
    // Local variables.
    NODE_status_t status = NODE_SUCCESS;
    uint32_t* reg_ptr = &(NODE_RAM_REGISTER[reg_addr]);
#ifndef RRM_REGULATOR_CONTROL_FORCED_HARDWARE
    LOAD_status_t load_status = LOAD_SUCCESS;
    UNA_bit_representation_t ren = UNA_BIT_ERROR;
#endif
    // Check address.
    switch (reg_addr) {
    case RRM_REGISTER_ADDRESS_CONTROL_1:
        // REN.
        if ((reg_mask & RRM_REGISTER_CONTROL_1_MASK_REN) != 0) {
            // Check pin mode.
#ifdef RRM_REGULATOR_CONTROL_FORCED_HARDWARE
            status = NODE_ERROR_FORCED_HARDWARE;
            goto errors;
#else
            // Read bit.
            ren = SWREG_read_field((*reg_ptr), RRM_REGISTER_CONTROL_1_MASK_REN);
            // Compare to current state.
            if (ren != rrm_ctx.regulator_control_state) {
                // Set regulator state.
                load_status = LOAD_set_output_state(ren);
                LOAD_exit_error(NODE_ERROR_BASE_LOAD);
            }
#endif
        }
        break;
    default:
        break;
    }
errors:
    RRM_refresh_register(RRM_REGISTER_ADDRESS_STATUS_1);
    return status;
}

/*******************************************************************/
NODE_status_t RRM_mtrg_callback(void) {
    // Local variables.
    NODE_status_t status = NODE_SUCCESS;
    ANALOG_status_t analog_status = ANALOG_SUCCESS;
    uint32_t* reg_analog_data_1_ptr = &(NODE_RAM_REGISTER[RRM_REGISTER_ADDRESS_ANALOG_DATA_1]);
    uint32_t* reg_analog_data_2_ptr = &(NODE_RAM_REGISTER[RRM_REGISTER_ADDRESS_ANALOG_DATA_2]);
    int32_t input_voltage_mv = 0;
    int32_t output_voltage_mv = 0;
    int32_t output_current_ua = 0;
    uint32_t unused_mask = 0;
    // Reset data.
    (*reg_analog_data_1_ptr) = NODE_REGISTER[RRM_REGISTER_ADDRESS_ANALOG_DATA_1].error_value;
    (*reg_analog_data_2_ptr) = NODE_REGISTER[RRM_REGISTER_ADDRESS_ANALOG_DATA_2].error_value;
    // Regulator input voltage.
    analog_status = ANALOG_convert_channel(ANALOG_CHANNEL_INPUT_VOLTAGE_MV, &input_voltage_mv);
    ANALOG_exit_error(NODE_ERROR_BASE_ANALOG);
    SWREG_write_field(reg_analog_data_1_ptr, &unused_mask, UNA_convert_mv(input_voltage_mv), RRM_REGISTER_ANALOG_DATA_1_MASK_INPUT_VOLTAGE);
    // Regulator output voltage.
    analog_status = ANALOG_convert_channel(ANALOG_CHANNEL_OUTPUT_VOLTAGE_MV, &output_voltage_mv);
    ANALOG_exit_error(NODE_ERROR_BASE_ANALOG);
    SWREG_write_field(reg_analog_data_1_ptr, &unused_mask, UNA_convert_mv(output_voltage_mv), RRM_REGISTER_ANALOG_DATA_1_MASK_OUTPUT_VOLTAGE);
    // Check output current measurement validity.
    if (output_voltage_mv >= RRM_OUTPUT_CURRENT_MEASUREMENT_POWER_TH_MV) {
        // Regulator output current.
        analog_status = ANALOG_convert_channel(ANALOG_CHANNEL_OUTPUT_CURRENT_UA, &output_current_ua);
        ANALOG_exit_error(NODE_ERROR_BASE_ANALOG);
    }
    SWREG_write_field(reg_analog_data_2_ptr, &unused_mask, UNA_convert_ua(output_current_ua), RRM_REGISTER_ANALOG_DATA_2_MASK_OUTPUT_CURRENT);
errors:
    return status;
}

#endif /* RRM */

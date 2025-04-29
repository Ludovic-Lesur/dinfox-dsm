/*
 * rrm.c
 *
 *  Created on: 10 jun. 2023
 *      Author: Ludo
 */

#include "rrm.h"

#include "analog.h"
#include "dsm_flags.h"
#include "error.h"
#include "load.h"
#include "rrm_registers.h"
#include "node.h"
#include "swreg.h"
#include "una.h"

#ifdef RRM

/*** RRM local macros ***/

// Note: IOUT measurement uses LT6106 and OPA187 chips whose minimum operating voltage is 4.5V.
#define RRM_IOUT_MEASUREMENT_VSH_MIN_MV     4500

/*** RRM local structures ***/

/*******************************************************************/
typedef union {
    UNA_bit_representation_t renst;
} RRM_context_t;

/*** RRM local global variables ***/

static RRM_context_t rrm_ctx = {
    .renst = UNA_BIT_ERROR
};

/*** RRM local functions ***/

/*******************************************************************/
static void _RRM_load_fixed_configuration(void) {
    // Local variables.
    uint32_t reg_value = 0;
    uint32_t reg_mask = 0;
    // Regulator control mode.
#ifdef RRM_REN_FORCED_HARDWARE
    SWREG_write_field(&reg_value, &reg_mask, 0b1, RRM_REGISTER_CONFIGURATION_0_MASK_RFH);
#else
    SWREG_write_field(&reg_value, &reg_mask, 0b0, RRM_REGISTER_CONFIGURATION_0_MASK_RFH);
#endif
    NODE_write_register(NODE_REQUEST_SOURCE_INTERNAL, RRM_REGISTER_ADDRESS_CONFIGURATION_0, reg_value, reg_mask);
}

/*******************************************************************/
static void _RRM_load_dynamic_configuration(void) {
    // Local variables.
    uint8_t reg_addr = 0;
    uint32_t reg_value = 0;
    // Load configuration registers from NVM.
    for (reg_addr = RRM_REGISTER_ADDRESS_CONFIGURATION_1; reg_addr < RRM_REGISTER_ADDRESS_STATUS_1; reg_addr++) {
        // Read NVM.
        NODE_read_nvm(reg_addr, &reg_value);
        // Write register.
        NODE_write_register(NODE_REQUEST_SOURCE_INTERNAL, reg_addr, reg_value, UNA_REGISTER_MASK_ALL);
    }
}

/*******************************************************************/
static void _RRM_reset_analog_data(void) {
    // Reset analog registers.
    NODE_write_register(NODE_REQUEST_SOURCE_INTERNAL, RRM_REGISTER_ADDRESS_ANALOG_DATA_1, RRM_REGISTER_ERROR_VALUE[RRM_REGISTER_ADDRESS_ANALOG_DATA_1], UNA_REGISTER_MASK_ALL);
    NODE_write_register(NODE_REQUEST_SOURCE_INTERNAL, RRM_REGISTER_ADDRESS_ANALOG_DATA_2, RRM_REGISTER_ERROR_VALUE[RRM_REGISTER_ADDRESS_ANALOG_DATA_2], UNA_REGISTER_MASK_ALL);
}

/*** RRM functions ***/

/*******************************************************************/
NODE_status_t RRM_init_registers(void) {
    // Local variables.
    NODE_status_t status = NODE_SUCCESS;
#ifdef DSM_NVM_FACTORY_RESET
    uint32_t reg_value = 0;
    uint32_t reg_mask = 0;
    // IOUT offset.
    SWREG_write_field(&reg_value, &reg_mask, 0, RRM_REGISTER_CONFIGURATION_1_MASK_IOUT_OFFSET);
    NODE_write_register(NODE_REQUEST_SOURCE_EXTERNAL, RRM_REGISTER_ADDRESS_CONFIGURATION_1, reg_value, reg_mask);
#endif
    // Load default values.
    _RRM_load_fixed_configuration();
    _RRM_load_dynamic_configuration();
    _RRM_reset_analog_data();
    // Read init state.
    status = RRM_update_register(RRM_REGISTER_ADDRESS_STATUS_1);
    if (status != NODE_SUCCESS) goto errors;
errors:
    return status;
}

/*******************************************************************/
NODE_status_t RRM_update_register(uint8_t reg_addr) {
    // Local variables.
    NODE_status_t status = NODE_SUCCESS;
    uint32_t reg_value = 0;
    uint32_t reg_mask = 0;
    // Check address.
    switch (reg_addr) {
    case RRM_REGISTER_ADDRESS_STATUS_1:
        // Regulator state.
#ifdef RRM_REN_FORCED_HARDWARE
        rrm_ctx.renst = UNA_BIT_FORCED_HARDWARE;
#else
        rrm_ctx.renst = (LOAD_get_output_state() == 0) ? UNA_BIT_0 : UNA_BIT_1;
#endif
        SWREG_write_field(&reg_value, &reg_mask, ((uint32_t) rrm_ctx.renst), RRM_REGISTER_STATUS_1_MASK_RENST);
        break;
    default:
        // Nothing to do for other registers.
        break;
    }
    // Write register.
    NODE_write_register(NODE_REQUEST_SOURCE_INTERNAL, reg_addr, reg_value, reg_mask);
    return status;
}

/*******************************************************************/
NODE_status_t RRM_check_register(uint8_t reg_addr, uint32_t reg_mask) {
    // Local variables.
    NODE_status_t status = NODE_SUCCESS;
    uint32_t reg_value = 0;
#ifndef RRM_REN_FORCED_HARDWARE
    LOAD_status_t load_status = LOAD_SUCCESS;
    UNA_bit_representation_t ren = UNA_BIT_ERROR;
#endif
    // Read register.
    status = NODE_read_register(NODE_REQUEST_SOURCE_INTERNAL, reg_addr, &reg_value);
    if (status != NODE_SUCCESS) goto errors;
    // Check address.
    switch (reg_addr) {
    case RRM_REGISTER_ADDRESS_CONTROL_1:
        // REN.
        if ((reg_mask & RRM_REGISTER_CONTROL_1_MASK_REN) != 0) {
            // Check pin mode.
#ifdef RRM_REN_FORCED_HARDWARE
            status = NODE_ERROR_FORCED_HARDWARE;
            goto errors;
#else
            // Read bit.
            ren = SWREG_read_field(reg_value, RRM_REGISTER_CONTROL_1_MASK_REN);
            // Compare to current state.
            if (ren != rrm_ctx.renst) {
                // Set regulator state.
                load_status = LOAD_set_output_state(ren);
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
    POWER_disable(POWER_REQUESTER_ID_RRM, POWER_DOMAIN_ANALOG);
    RRM_update_register(RRM_REGISTER_ADDRESS_STATUS_1);
    return status;
}

/*******************************************************************/
NODE_status_t RRM_mtrg_callback(void) {
    // Local variables.
    NODE_status_t status = NODE_SUCCESS;
    ANALOG_status_t analog_status = ANALOG_SUCCESS;
    int32_t adc_data = 0;
    int32_t vsh_mv = 0;
    uint32_t reg_analog_data_1 = 0;
    uint32_t reg_analog_data_1_mask = 0;
    uint32_t reg_analog_data_2 = 0;
    uint32_t reg_analog_data_2_mask = 0;
    // Reset result.
    _RRM_reset_analog_data();
    // Regulator input voltage.
    analog_status = ANALOG_convert_channel(ANALOG_CHANNEL_VIN_MV, &adc_data);
    ANALOG_exit_error(NODE_ERROR_BASE_ANALOG);
    SWREG_write_field(&reg_analog_data_1, &reg_analog_data_1_mask, UNA_convert_mv(adc_data), RRM_REGISTER_ANALOG_DATA_1_MASK_VIN);
    // Regulator output voltage.
    analog_status = ANALOG_convert_channel(ANALOG_CHANNEL_VOUT_MV, &adc_data);
    ANALOG_exit_error(NODE_ERROR_BASE_ANALOG);
    SWREG_write_field(&reg_analog_data_1, &reg_analog_data_1_mask, UNA_convert_mv(adc_data), RRM_REGISTER_ANALOG_DATA_1_MASK_VOUT);
    vsh_mv = adc_data;
    // Check IOUT measurement validity.
    if (vsh_mv >= RRM_IOUT_MEASUREMENT_VSH_MIN_MV) {
        // Regulator output current.
        analog_status = ANALOG_convert_channel(ANALOG_CHANNEL_IOUT_UA, &adc_data);
        ANALOG_exit_error(NODE_ERROR_BASE_ANALOG);
        SWREG_write_field(&reg_analog_data_2, &reg_analog_data_2_mask, UNA_convert_ua(adc_data), RRM_REGISTER_ANALOG_DATA_2_MASK_IOUT);
    }
    // Write registers.
    NODE_write_register(NODE_REQUEST_SOURCE_INTERNAL, RRM_REGISTER_ADDRESS_ANALOG_DATA_1, reg_analog_data_1, reg_analog_data_1_mask);
    NODE_write_register(NODE_REQUEST_SOURCE_INTERNAL, RRM_REGISTER_ADDRESS_ANALOG_DATA_2, reg_analog_data_2, reg_analog_data_2_mask);
errors:
    return status;
}

#endif /* RRM */

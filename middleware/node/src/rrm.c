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

// Note: IOUT measurement uses LT6106 and OPA187 chips whose minimum operating voltage is 4.5V.
#define RRM_IOUT_MEASUREMENT_VSH_MIN_MV     4500

#define RRM_IOUT_OFFSET_UA_MAX              100000
#define RRM_IOUT_OFFSET_UA_DEFAULT          0

#ifdef RRM_REN_FORCED_HARDWARE
#define RRM_FLAG_RFH                        0b1
#else
#define RRM_FLAG_RFH                        0b0
#endif

/*** RRM local structures ***/

/*******************************************************************/
typedef struct {
    UNA_bit_representation_t renst;
} RRM_context_t;

/*** RRM local global variables ***/

static RRM_context_t rrm_ctx = {
    .renst = UNA_BIT_ERROR
};

/*** RRM functions ***/

/*******************************************************************/
NODE_status_t RRM_init(void) {
    // Local variables.
    NODE_status_t status = NODE_SUCCESS;
    // Init context.
    rrm_ctx.renst = UNA_BIT_ERROR;
    return status;
}

/*******************************************************************/
void RRM_init_register(uint8_t reg_addr, uint32_t* reg_value) {
    // Local variables.
    uint32_t unused_mask = 0;
    // Check address.
    switch (reg_addr) {
    case RRM_REGISTER_ADDRESS_FLAGS_1:
        SWREG_write_field(reg_value, &unused_mask, RRM_FLAG_RFH, RRM_REGISTER_FLAGS_1_MASK_RFH);
        break;
#ifdef DSM_NVM_FACTORY_RESET
    case RRM_REGISTER_ADDRESS_CONFIGURATION_0:
        SWREG_write_field(reg_value, &unused_mask, RRM_IOUT_OFFSET_UA_DEFAULT, RRM_REGISTER_CONFIGURATION_0_MASK_IOUT_OFFSET);
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
#ifdef RRM_REN_FORCED_HARDWARE
        rrm_ctx.renst = UNA_BIT_FORCED_HARDWARE;
#else
        switch (LOAD_get_output_state()) {
        case 0:
            rrm_ctx.renst = UNA_BIT_0;
            break;
        case 1:
            rrm_ctx.renst = UNA_BIT_1;
            break;
        default:
            rrm_ctx.renst = UNA_BIT_ERROR;
            break;
        }
#endif
        SWREG_write_field(reg_ptr, &unused_mask, ((uint32_t) rrm_ctx.renst), RRM_REGISTER_STATUS_1_MASK_RENST);
        break;
    default:
        break;
    }
}

/*******************************************************************/
NODE_status_t RRM_secure_register(uint8_t reg_addr, uint32_t new_reg_value, uint32_t* reg_mask, uint32_t* reg_value) {
    // Local variables.
    NODE_status_t status = NODE_SUCCESS;
    uint32_t generic_u32 = 0;
    // Check address.
    switch (reg_addr) {
    case RRM_REGISTER_ADDRESS_CONFIGURATION_0:
        SWREG_secure_field(
            RRM_REGISTER_CONFIGURATION_0_MASK_IOUT_OFFSET,
            UNA_get_ua,
            UNA_convert_ua,
            > RRM_IOUT_OFFSET_UA_MAX,
            > RRM_IOUT_OFFSET_UA_MAX,
            RRM_IOUT_OFFSET_UA_DEFAULT,
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
#ifndef RRM_REN_FORCED_HARDWARE
    LOAD_status_t load_status = LOAD_SUCCESS;
    UNA_bit_representation_t ren = UNA_BIT_ERROR;
#endif
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
            ren = SWREG_read_field((*reg_ptr), RRM_REGISTER_CONTROL_1_MASK_REN);
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
    int32_t vin_mv = 0;
    int32_t vout_mv = 0;
    int32_t iout_ua = 0;
    uint32_t unused_mask = 0;
    // Reset data.
    (*reg_analog_data_1_ptr) = NODE_REGISTER[RRM_REGISTER_ADDRESS_ANALOG_DATA_1].error_value;
    (*reg_analog_data_2_ptr) = NODE_REGISTER[RRM_REGISTER_ADDRESS_ANALOG_DATA_2].error_value;
    // Regulator input voltage.
    analog_status = ANALOG_convert_channel(ANALOG_CHANNEL_VIN_MV, &vin_mv);
    ANALOG_exit_error(NODE_ERROR_BASE_ANALOG);
    SWREG_write_field(reg_analog_data_1_ptr, &unused_mask, UNA_convert_mv(vin_mv), RRM_REGISTER_ANALOG_DATA_1_MASK_VIN);
    // Regulator output voltage.
    analog_status = ANALOG_convert_channel(ANALOG_CHANNEL_VOUT_MV, &vout_mv);
    ANALOG_exit_error(NODE_ERROR_BASE_ANALOG);
    SWREG_write_field(reg_analog_data_1_ptr, &unused_mask, UNA_convert_mv(vout_mv), RRM_REGISTER_ANALOG_DATA_1_MASK_VOUT);
    // Check IOUT measurement validity.
    if (vout_mv >= RRM_IOUT_MEASUREMENT_VSH_MIN_MV) {
        // Regulator output current.
        analog_status = ANALOG_convert_channel(ANALOG_CHANNEL_IOUT_UA, &iout_ua);
        ANALOG_exit_error(NODE_ERROR_BASE_ANALOG);
    }
    SWREG_write_field(reg_analog_data_2_ptr, &unused_mask, UNA_convert_ua(iout_ua), RRM_REGISTER_ANALOG_DATA_2_MASK_IOUT);
errors:
    return status;
}

#endif /* RRM */

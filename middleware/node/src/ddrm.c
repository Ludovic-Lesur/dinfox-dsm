/*
 * ddrm.c
 *
 *  Created on: 04 jun. 2023
 *      Author: Ludo
 */

#include "ddrm.h"

#ifdef DDRM

#include "adc.h"
#include "dsm_flags.h"
#include "error.h"
#include "load.h"
#include "ddrm_registers.h"
#include "node_register.h"
#include "node_status.h"
#include "swreg.h"
#include "types.h"
#include "una.h"

/*** DDRM local macros ***/

// Note: IOUT measurement uses LT6106 and OPA187 chips whose minimum operating voltage is 4.5V.
#define DDRM_IOUT_MEASUREMENT_VSH_MIN_MV    4500

#define DDRM_IOUT_OFFSET_UA_MAX             100000
#define DDRM_IOUT_OFFSET_UA_DEFAULT         0

#ifdef DDRM_DDEN_FORCED_HARDWARE
#define DDRM_FLAG_DDFH                      0b1
#else
#define DDRM_FLAG_DDFH                      0b0
#endif

/*** DDRM local structures ***/

/*******************************************************************/
typedef struct {
    UNA_bit_representation_t ddenst;
} DDRM_context_t;

/*** DDRM local global variables ***/

static DDRM_context_t ddrm_ctx = {
    .ddenst = UNA_BIT_ERROR
};

/*** DDRM functions ***/

/*******************************************************************/
NODE_status_t DDRM_init(void) {
    // Local variables.
    NODE_status_t status = NODE_SUCCESS;
    // Init context.
    ddrm_ctx.ddenst = UNA_BIT_ERROR;
    return status;
}

/*******************************************************************/
void DDRM_init_register(uint8_t reg_addr, uint32_t* reg_value) {
    // Local variables.
    uint32_t unused_mask = 0;
    // Check address.
    switch (reg_addr) {
    case DDRM_REGISTER_ADDRESS_FLAGS_1:
        SWREG_write_field(reg_value, &unused_mask, DDRM_FLAG_DDFH, DDRM_REGISTER_FLAGS_1_MASK_DDFH);
        break;
#ifdef DSM_NVM_FACTORY_RESET
    case DDRM_REGISTER_ADDRESS_CONFIGURATION_0:
        SWREG_write_field(reg_value, &unused_mask, DDRM_IOUT_OFFSET_UA_DEFAULT, DDRM_REGISTER_CONFIGURATION_0_MASK_IOUT_OFFSET);
        break;
#endif
    default:
        break;
    }
}

/*******************************************************************/
void DDRM_refresh_register(uint8_t reg_addr) {
    // Local variables.
    uint32_t* reg_ptr = &(NODE_RAM_REGISTER[reg_addr]);
    uint32_t unused_mask = 0;
    // Check address.
    switch (reg_addr) {
    case DDRM_REGISTER_ADDRESS_STATUS_1:
        // DC-DC state.
#ifdef DDRM_DDEN_FORCED_HARDWARE
        ddrm_ctx.ddenst = UNA_BIT_FORCED_HARDWARE;
#else
        switch (LOAD_get_output_state()) {
        case 0:
            ddrm_ctx.ddenst = UNA_BIT_0;
            break;
        case 1:
            ddrm_ctx.ddenst = UNA_BIT_1;
            break;
        default:
            ddrm_ctx.ddenst = UNA_BIT_ERROR;
            break;
        }
#endif
        SWREG_write_field(reg_ptr, &unused_mask, ((uint32_t) ddrm_ctx.ddenst), DDRM_REGISTER_STATUS_1_MASK_DDENST);
        break;
    default:
        break;
    }
}

/*******************************************************************/
NODE_status_t DDRM_secure_register(uint8_t reg_addr, uint32_t new_reg_value, uint32_t* reg_mask, uint32_t* reg_value) {
    // Local variables.
    NODE_status_t status = NODE_SUCCESS;
    uint32_t generic_u32 = 0;
    // Check address.
    switch (reg_addr) {
    case DDRM_REGISTER_ADDRESS_CONFIGURATION_0:
        SWREG_secure_field(
            DDRM_REGISTER_CONFIGURATION_0_MASK_IOUT_OFFSET,
            UNA_get_ua,
            UNA_convert_ua,
            > DDRM_IOUT_OFFSET_UA_MAX,
            > DDRM_IOUT_OFFSET_UA_MAX,
            DDRM_IOUT_OFFSET_UA_DEFAULT,
            status = NODE_ERROR_REGISTER_FIELD_VALUE
        );
        break;
    default:
        break;
    }
    return status;
}

/*******************************************************************/
NODE_status_t DDRM_process_register(uint8_t reg_addr, uint32_t reg_mask) {
    // Local variables.
    NODE_status_t status = NODE_SUCCESS;
    uint32_t* reg_ptr = &(NODE_RAM_REGISTER[reg_addr]);
#ifndef DDRM_DDEN_FORCED_HARDWARE
    LOAD_status_t load_status = LOAD_SUCCESS;
    UNA_bit_representation_t dden = UNA_BIT_ERROR;
#endif
    // Check address.
    switch (reg_addr) {
    case DDRM_REGISTER_ADDRESS_CONTROL_1:
        // DDEN.
        if ((reg_mask & DDRM_REGISTER_CONTROL_1_MASK_DDEN) != 0) {
            // Check pin mode.
#ifdef DDRM_DDEN_FORCED_HARDWARE
            status = NODE_ERROR_FORCED_HARDWARE;
            goto errors;
#else
            // Read bit.
            dden = SWREG_read_field((*reg_ptr), DDRM_REGISTER_CONTROL_1_MASK_DDEN);
            // Compare to current state.
            if (dden != ddrm_ctx.ddenst) {
                // Set DC-DC state.
                load_status = LOAD_set_output_state(dden);
                LOAD_exit_error(NODE_ERROR_BASE_LOAD);
            }
#endif
        }
        break;
    }
errors:
    DDRM_refresh_register(DDRM_REGISTER_ADDRESS_STATUS_1);
    return status;
}

/*******************************************************************/
NODE_status_t DDRM_mtrg_callback(void) {
    // Local variables.
    NODE_status_t status = NODE_SUCCESS;
    ANALOG_status_t analog_status = ANALOG_SUCCESS;
    uint32_t* reg_analog_data_1_ptr = &(NODE_RAM_REGISTER[DDRM_REGISTER_ADDRESS_ANALOG_DATA_1]);
    uint32_t* reg_analog_data_2_ptr = &(NODE_RAM_REGISTER[DDRM_REGISTER_ADDRESS_ANALOG_DATA_2]);
    int32_t vin_mv = 0;
    int32_t vout_mv = 0;
    int32_t iout_ua = 0;
    uint32_t unused_mask = 0;
    // Reset data.
    (*reg_analog_data_1_ptr) = NODE_REGISTER[DDRM_REGISTER_ADDRESS_ANALOG_DATA_1].error_value;
    (*reg_analog_data_2_ptr) = NODE_REGISTER[DDRM_REGISTER_ADDRESS_ANALOG_DATA_2].error_value;
    // DC-DC input voltage.
    analog_status = ANALOG_convert_channel(ANALOG_CHANNEL_VIN_MV, &vin_mv);
    ANALOG_exit_error(NODE_ERROR_BASE_ANALOG);
    SWREG_write_field(reg_analog_data_1_ptr, &unused_mask, UNA_convert_mv(vin_mv), DDRM_REGISTER_ANALOG_DATA_1_MASK_VIN);
    // DC-DC output voltage.
    analog_status = ANALOG_convert_channel(ANALOG_CHANNEL_VOUT_MV, &vout_mv);
    ANALOG_exit_error(NODE_ERROR_BASE_ANALOG);
    SWREG_write_field(reg_analog_data_1_ptr, &unused_mask, UNA_convert_mv(vout_mv), DDRM_REGISTER_ANALOG_DATA_1_MASK_VOUT);
    // Check IOUT measurement validity.
    if (vout_mv >= DDRM_IOUT_MEASUREMENT_VSH_MIN_MV) {
        // DC-DC output current.
        analog_status = ANALOG_convert_channel(ANALOG_CHANNEL_IOUT_UA, &iout_ua);
        ANALOG_exit_error(NODE_ERROR_BASE_ANALOG);
    }
    SWREG_write_field(reg_analog_data_2_ptr, &unused_mask, UNA_convert_ua(iout_ua), DDRM_REGISTER_ANALOG_DATA_2_MASK_IOUT);
errors:
    return status;
}

#endif /* DDRM */

/*
 * lvrm.c
 *
 *  Created on: 04 jun. 2023
 *      Author: Ludo
 */

#include "lvrm.h"

#include "adc.h"
#include "error.h"
#include "load.h"
#include "lvrm_registers.h"
#include "node.h"
#include "swreg.h"
#include "una.h"

#ifdef LVRM

/*** LVRM local macros ***/

// Note: IOUT measurement uses LT6106, OPA187 and optionally TMUX7219 chips whose minimum operating voltage is 4.5V.
#define LVRM_IOUT_MEASUREMENT_VCOM_MIN_MV   4500

/*** LVRM local structures ***/

/*******************************************************************/
typedef struct {
    UNA_bit_representation_t rlstst;
} LVRM_context_t;

/*** LVRM local global variables ***/

static LVRM_context_t lvrm_ctx;

/*** LVRM local functions ***/

/*******************************************************************/
static void _LVRM_load_fixed_configuration(void) {
    // Local variables.
    uint32_t reg_value = 0;
    uint32_t reg_mask = 0;
    // BMS flag.
#ifdef LVRM_MODE_BMS
    SWREG_write_field(&reg_value, &reg_mask, 0b1, LVRM_REGISTER_CONFIGURATION_0_MASK_BMSF);
#else
    SWREG_write_field(&reg_value, &reg_mask, 0b0, LVRM_REGISTER_CONFIGURATION_0_MASK_BMSF);
#endif
    // Relay control mode.
#ifdef LVRM_RLST_FORCED_HARDWARE
    SWREG_write_field(&reg_value, &reg_mask, 0b1, LVRM_REGISTER_CONFIGURATION_0_MASK_RLFH);
#else
    SWREG_write_field(&reg_value, &reg_mask, 0b0, LVRM_REGISTER_CONFIGURATION_0_MASK_RLFH);
#endif
    NODE_write_register(NODE_REQUEST_SOURCE_INTERNAL, LVRM_REGISTER_ADDRESS_CONFIGURATION_0, reg_value, reg_mask);
}

/*******************************************************************/
static void _LVRM_load_dynamic_configuration(void) {
    // Local variables.
    uint8_t reg_addr = 0;
    uint32_t reg_value = 0;
    // Load configuration registers from NVM.
    for (reg_addr = LVRM_REGISTER_ADDRESS_CONFIGURATION_1; reg_addr < LVRM_REGISTER_ADDRESS_STATUS_1; reg_addr++) {
        // Read NVM.
        NODE_read_nvm(reg_addr, &reg_value);
        // Write register.
        NODE_write_register(NODE_REQUEST_SOURCE_INTERNAL, reg_addr, reg_value, UNA_REGISTER_MASK_ALL);
    }
}

/*******************************************************************/
static void _LVRM_reset_analog_data(void) {
    // Local variables.
    uint32_t reg_analog_data_1 = 0;
    uint32_t reg_analog_data_1_mask = 0;
    uint32_t reg_analog_data_2 = 0;
    uint32_t reg_analog_data_2_mask = 0;
    // VIN / VOUT.
    SWREG_write_field(&reg_analog_data_1, &reg_analog_data_1_mask, UNA_VOLTAGE_ERROR_VALUE, LVRM_REGISTER_ANALOG_DATA_1_MASK_VCOM);
    SWREG_write_field(&reg_analog_data_1, &reg_analog_data_1_mask, UNA_VOLTAGE_ERROR_VALUE, LVRM_REGISTER_ANALOG_DATA_1_MASK_VOUT);
    NODE_write_register(NODE_REQUEST_SOURCE_INTERNAL, LVRM_REGISTER_ADDRESS_ANALOG_DATA_1, reg_analog_data_1, reg_analog_data_1_mask);
    // IOUT.
    SWREG_write_field(&reg_analog_data_2, &reg_analog_data_2_mask, UNA_VOLTAGE_ERROR_VALUE, LVRM_REGISTER_ANALOG_DATA_2_MASK_IOUT);
    NODE_write_register(NODE_REQUEST_SOURCE_INTERNAL, LVRM_REGISTER_ADDRESS_ANALOG_DATA_2, reg_analog_data_2, reg_analog_data_2_mask);
}

/*** LVRM functions ***/

/*******************************************************************/
NODE_status_t LVRM_init_registers(void) {
    // Local variables.
    NODE_status_t status = NODE_SUCCESS;
#ifdef XM_NVM_FACTORY_RESET
    // Local variables.
    uint32_t reg_value = 0;
    uint32_t reg_mask = 0;
    // VBATT thresholds in BMS mode.
    SWREG_write_field(&reg_value, &reg_mask, LVRM_BMS_VBATT_LOW_THRESHOLD_MV, LVRM_REGISTER_CONFIGURATION_1_MASK_VBATT_LOW_THRESHOLD);
    SWREG_write_field(&reg_value, &reg_mask, LVRM_BMS_VBATT_HIGH_THRESHOLD_MV, LVRM_REGISTER_CONFIGURATION_1_MASK_VBATT_HIGH_THRESHOLD);
    NODE_write_register(NODE_REQUEST_SOURCE_EXTERNAL, LVRM_REGISTER_ADDRESS_CONFIGURATION_1, reg_value, reg_mask);
    // IOUT offset.
    reg_value = 0;
    reg_mask = 0;
    SWREG_write_field(&reg_value, &reg_mask, 0, LVRM_REGISTER_CONFIGURATION_2_MASK_IOUT_OFFSET);
    NODE_write_register(NODE_REQUEST_SOURCE_EXTERNAL, LVRM_REGISTER_ADDRESS_CONFIGURATION_2, reg_value, reg_mask);
#endif
    // Load defaults values.
    _LVRM_load_fixed_configuration();
    _LVRM_load_dynamic_configuration();
    _LVRM_reset_analog_data();
    // Read init state.
    status = LVRM_update_register(LVRM_REGISTER_ADDRESS_STATUS_1);
    if (status != NODE_SUCCESS) goto errors;
errors:
    return status;
}

/*******************************************************************/
NODE_status_t LVRM_update_register(uint8_t reg_addr) {
    // Local variables.
    NODE_status_t status = NODE_SUCCESS;
    uint32_t reg_value = 0;
    uint32_t reg_mask = 0;
    // Check address.
    switch (reg_addr) {
    case LVRM_REGISTER_ADDRESS_STATUS_1:
        // Relay state.
#ifdef LVRM_RLST_FORCED_HARDWARE
        lvrm_ctx.rlstst = UNA_BIT_FORCED_HARDWARE;
#else
        lvrm_ctx.rlstst = (LOAD_get_output_state() == 0) ? UNA_BIT_0 : UNA_BIT_1;
#endif
        SWREG_write_field(&reg_value, &reg_mask, ((uint32_t) lvrm_ctx.rlstst), LVRM_REGISTER_STATUS_1_MASK_RLSTST);
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
NODE_status_t LVRM_check_register(uint8_t reg_addr, uint32_t reg_mask) {
    // Local variables.
    NODE_status_t status = NODE_SUCCESS;
    uint32_t reg_value = 0;
#if !(defined LVRM_RLST_FORCED_HARDWARE) && !(defined LVRM_MODE_BMS)
    LOAD_status_t load_status = LOAD_SUCCESS;
    UNA_bit_representation_t rlst = UNA_BIT_ERROR;
#endif
    // Read register.
    status = NODE_read_register(NODE_REQUEST_SOURCE_INTERNAL, reg_addr, &reg_value);
    if (status != NODE_SUCCESS) goto errors;
    // Check address.
    switch (reg_addr) {
    case LVRM_REGISTER_ADDRESS_CONFIGURATION_1:
    case LVRM_REGISTER_ADDRESS_CONFIGURATION_2:
        // Store new value in NVM.
        if (reg_mask != 0) {
            NODE_write_nvm(reg_addr, reg_value);
        }
        break;
    case LVRM_REGISTER_ADDRESS_CONTROL_1:
        // RLST.
        if ((reg_mask & LVRM_REGISTER_CONTROL_1_MASK_RLST) != 0) {
            // Check pin mode.
#ifdef LVRM_RLST_FORCED_HARDWARE
            status = NODE_ERROR_FORCED_HARDWARE;
            goto errors;
#else
#ifdef LVRM_MODE_BMS
            status = NODE_ERROR_FORCED_SOFTWARE;
            goto errors;
#else
            // Read bit.
            rlst = SWREG_read_field(reg_value, LVRM_REGISTER_CONTROL_1_MASK_RLST);
            // Compare to current state.
            if (rlst != lvrm_ctx.rlstst) {
                // Set relay state.
                load_status = LOAD_set_output_state(rlst);
                LOAD_exit_error(NODE_ERROR_BASE_LOAD);
            }
#endif
#endif
        }
        break;
    default:
        // Nothing to do for other registers.
        break;
    }
errors:
    POWER_disable(POWER_REQUESTER_ID_LVRM, POWER_DOMAIN_ANALOG);
    LVRM_update_register(LVRM_REGISTER_ADDRESS_STATUS_1);
    return status;
}

/*******************************************************************/
NODE_status_t LVRM_mtrg_callback(void) {
    // Local variables.
    NODE_status_t status = NODE_SUCCESS;
    ANALOG_status_t analog_status = ANALOG_SUCCESS;
    int32_t adc_data = 0;
    int32_t vcom_mv = 0;
    uint32_t reg_analog_data_1 = 0;
    uint32_t reg_analog_data_1_mask = 0;
    uint32_t reg_analog_data_2 = 0;
    uint32_t reg_analog_data_2_mask = 0;
    // Reset results.
    _LVRM_reset_analog_data();
    // Relay common voltage.
    analog_status = ANALOG_convert_channel(ANALOG_CHANNEL_VIN_MV, &adc_data);
    ANALOG_exit_error(NODE_ERROR_BASE_ANALOG);
    SWREG_write_field(&reg_analog_data_1, &reg_analog_data_1_mask, UNA_convert_mv(adc_data), LVRM_REGISTER_ANALOG_DATA_1_MASK_VCOM);
    vcom_mv = adc_data;
    // Relay output voltage.
    analog_status = ANALOG_convert_channel(ANALOG_CHANNEL_VOUT_MV, &adc_data);
    ANALOG_exit_error(NODE_ERROR_BASE_ANALOG);
    SWREG_write_field(&reg_analog_data_1, &reg_analog_data_1_mask, UNA_convert_mv(adc_data), LVRM_REGISTER_ANALOG_DATA_1_MASK_VOUT);
    // Check IOUT measurement validity.
    if (vcom_mv >= LVRM_IOUT_MEASUREMENT_VCOM_MIN_MV) {
        // Relay output current.
        analog_status = ANALOG_convert_channel(ANALOG_CHANNEL_IOUT_UA, &adc_data);
        ANALOG_exit_error(NODE_ERROR_BASE_ANALOG);
        SWREG_write_field(&reg_analog_data_2, &reg_analog_data_2_mask, UNA_convert_ua(adc_data), LVRM_REGISTER_ANALOG_DATA_2_MASK_IOUT);
    }
    // Write registers.
    NODE_write_register(NODE_REQUEST_SOURCE_INTERNAL, LVRM_REGISTER_ADDRESS_ANALOG_DATA_1, reg_analog_data_1, reg_analog_data_1_mask);
    NODE_write_register(NODE_REQUEST_SOURCE_INTERNAL, LVRM_REGISTER_ADDRESS_ANALOG_DATA_2, reg_analog_data_2, reg_analog_data_2_mask);
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
    uint32_t reg_config_1 = 0;
    int32_t vbatt_mv = 0;
    // Turn analog front-end on.
    POWER_enable(POWER_REQUESTER_ID_LVRM, POWER_DOMAIN_ANALOG, LPTIM_DELAY_MODE_ACTIVE);
    // Check battery voltage.
    analog_status = ANALOG_convert_channel(ANALOG_CHANNEL_VIN_MV, &vbatt_mv);
    ANALOG_exit_error(NODE_ERROR_BASE_ANALOG);
    // Read thresholds in registers.
    NODE_read_register(NODE_REQUEST_SOURCE_INTERNAL, LVRM_REGISTER_ADDRESS_CONFIGURATION_1, &reg_config_1);
    // Check battery voltage.
    if (vbatt_mv < UNA_get_mv(SWREG_read_field(reg_config_1, LVRM_REGISTER_CONFIGURATION_1_MASK_VBATT_LOW_THRESHOLD))) {
        // Open relay.
        load_status = LOAD_set_output_state(0);
        LOAD_exit_error(NODE_ERROR_BASE_LOAD);
    }
    if (vbatt_mv > UNA_get_mv(SWREG_read_field(reg_config_1, LVRM_REGISTER_CONFIGURATION_1_MASK_VBATT_HIGH_THRESHOLD))) {
        // Close relay.
        load_status = LOAD_set_output_state(1);
        LOAD_exit_error(NODE_ERROR_BASE_LOAD);
    }
errors:
    POWER_disable(POWER_REQUESTER_ID_LVRM, POWER_DOMAIN_ANALOG);
    return status;
}
#endif

#endif /* LVRM */

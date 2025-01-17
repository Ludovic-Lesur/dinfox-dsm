/*
 * bpsm.c
 *
 *  Created on: 04 jun. 2023
 *      Author: Ludo
 */

#include "bpsm.h"

#include "analog.h"
#include "bpsm_registers.h"
#include "error.h"
#include "load.h"
#include "mode.h"
#include "node.h"
#include "swreg.h"
#include "rtc.h"
#include "una.h"

#ifdef BPSM

/*** BPSM local macros ***/

#define BPSM_CHEN_TOGGLE_DURATION_SECONDS   1

/*** BPSM local structures ***/

/*******************************************************************/
typedef struct {
	UNA_bit_representation_t chenst;
	UNA_bit_representation_t bkenst;
#ifndef BPSM_CHEN_FORCED_HARDWARE
	uint32_t chen_toggle_previous_time_seconds;
	uint32_t chen_toggle_next_time_seconds;
#endif
} BPSM_context_t;

/*** BPSM local global variables ***/

static BPSM_context_t bpsm_ctx;

/*** BPSM local functions ***/

/*******************************************************************/
static void _BPSM_load_fixed_configuration(void) {
	// Local variables.
	uint32_t reg_value = 0;
	uint32_t reg_mask = 0;
	// Voltage divider ratio.
	SWREG_write_field(&reg_value, &reg_mask, BPSM_DIVIDER_RATIO_VSTR, BPSM_REGISTER_CONFIGURATION_0_MASK_VSTR_RATIO);
	// Backup output control mode.
#ifdef BPSM_BKEN_FORCED_HARDWARE
	SWREG_write_field(&reg_value, &reg_mask, 0b1, BPSM_REGISTER_CONFIGURATION_0_MASK_BKFH);
#else
	SWREG_write_field(&reg_value, &reg_mask, 0b0, BPSM_REGISTER_CONFIGURATION_0_MASK_BKFH);
#endif
	// Charge status mode.
#ifdef BPSM_CHST_FORCED_HARDWARE
	SWREG_write_field(&reg_value, &reg_mask, 0b1, BPSM_REGISTER_CONFIGURATION_0_MASK_CSFH);
#else
	SWREG_write_field(&reg_value, &reg_mask, 0b0, BPSM_REGISTER_CONFIGURATION_0_MASK_CSFH);
#endif
	// Charge control mode.
#ifdef BPSM_CHEN_FORCED_HARDWARE
	SWREG_write_field(&reg_value, &reg_mask, 0b1, BPSM_REGISTER_CONFIGURATION_0_MASK_CEFH);
#else
	SWREG_write_field(&reg_value, &reg_mask, 0b0, BPSM_REGISTER_CONFIGURATION_0_MASK_CEFH);
#endif
	NODE_write_register(NODE_REQUEST_SOURCE_INTERNAL, BPSM_REGISTER_ADDRESS_CONFIGURATION_0, reg_value, reg_mask);
}

/*******************************************************************/
static void _BPSM_load_dynamic_configuration(void) {
	// Local variables.
	uint8_t reg_addr = 0;
	uint32_t reg_value = 0;
	// Load configuration registers from NVM.
	for (reg_addr=BPSM_REGISTER_ADDRESS_CONFIGURATION_1 ; reg_addr<BPSM_REGISTER_ADDRESS_STATUS_1 ; reg_addr++) {
		// Read NVM.
		NODE_read_nvm(reg_addr, &reg_value);
		// Write register.
		NODE_write_register(NODE_REQUEST_SOURCE_INTERNAL, reg_addr, reg_value, UNA_REGISTER_MASK_ALL);
	}
}

/*******************************************************************/
static void _BPSM_reset_analog_data(void) {
	// Local variables.
	uint32_t reg_analog_data_1 = 0;
	uint32_t reg_analog_data_1_mask = 0;
	uint32_t reg_analog_data_2 = 0;
	uint32_t reg_analog_data_2_mask = 0;
	// VSRC / VSTR.
	SWREG_write_field(&reg_analog_data_1, &reg_analog_data_1_mask, UNA_VOLTAGE_ERROR_VALUE, BPSM_REGISTER_ANALOG_DATA_1_MASK_VSRC);
	SWREG_write_field(&reg_analog_data_1, &reg_analog_data_1_mask, UNA_VOLTAGE_ERROR_VALUE, BPSM_REGISTER_ANALOG_DATA_1_MASK_VSTR);
	NODE_write_register(NODE_REQUEST_SOURCE_INTERNAL, BPSM_REGISTER_ADDRESS_ANALOG_DATA_1, reg_analog_data_1, reg_analog_data_1_mask);
	// VBKP.
	SWREG_write_field(&reg_analog_data_2, &reg_analog_data_2_mask, UNA_VOLTAGE_ERROR_VALUE, BPSM_REGISTER_ANALOG_DATA_2_MASK_VBKP);
	NODE_write_register(NODE_REQUEST_SOURCE_INTERNAL, BPSM_REGISTER_ADDRESS_ANALOG_DATA_2, reg_analog_data_2, reg_analog_data_2_mask);
}

/*** BPSM functions ***/

/*******************************************************************/
NODE_status_t BPSM_init_registers(void) {
    // Local variables.
    NODE_status_t status = NODE_SUCCESS;
#ifdef XM_NVM_FACTORY_RESET
    uint32_t reg_value = 0;
    uint32_t reg_mask = 0;
#endif
    // Init context.
    bpsm_ctx.chenst = UNA_BIT_ERROR;
    bpsm_ctx.bkenst = UNA_BIT_ERROR;
#ifndef BPSM_CHEN_FORCED_HARDWARE
    bpsm_ctx.chen_toggle_previous_time_seconds = 0;
    bpsm_ctx.chen_toggle_next_time_seconds = 0;
#endif
#ifdef XM_NVM_FACTORY_RESET
	// CHEN toggle threshold and period.
	SWREG_write_field(&reg_value, &reg_mask, UNA_convert_seconds(BPSM_CHEN_TOGGLE_PERIOD_SECONDS),  BPSM_REGISTER_CONFIGURATION_1_MASK_CHEN_TOGGLE_PERIOD);
	SWREG_write_field(&reg_value, &reg_mask, UNA_convert_mv(BPSM_CHEN_VSRC_THRESHOLD_MV), BPSM_REGISTER_CONFIGURATION_1_MASK_CHEN_THRESHOLD);
	NODE_write_register(NODE_REQUEST_SOURCE_EXTERNAL, BPSM_REGISTER_ADDRESS_CONFIGURATION_1, reg_value, reg_mask);
#endif
	// Load default values.
	_BPSM_load_fixed_configuration();
	_BPSM_load_dynamic_configuration();
	_BPSM_reset_analog_data();
	// Read init state.
    status = BPSM_update_register(BPSM_REGISTER_ADDRESS_STATUS_1);
	if (status != NODE_SUCCESS) goto errors;
errors:
    return status;
}

/*******************************************************************/
NODE_status_t BPSM_update_register(uint8_t reg_addr) {
	// Local variables.
	NODE_status_t status = NODE_SUCCESS;
	uint32_t reg_value = 0;
	uint32_t reg_mask = 0;
	UNA_bit_representation_t chrgst = UNA_BIT_ERROR;
	// Check address.
	switch (reg_addr) {
	case BPSM_REGISTER_ADDRESS_STATUS_1:
		// Charge status.
#ifdef BPSM_CHST_FORCED_HARDWARE
		chrgst = UNA_BIT_FORCED_HARDWARE;
#else
		chrgst = LOAD_get_charge_status();
#endif
		SWREG_write_field(&reg_value, &reg_mask, ((uint32_t) chrgst), BPSM_REGISTER_STATUS_1_MASK_CHRGST);
		// Charge state.
#ifdef BPSM_CHEN_FORCED_HARDWARE
		bpsm_ctx.chenst = UNA_BIT_FORCED_HARDWARE;
#else
		bpsm_ctx.chenst = LOAD_get_charge_state();
#endif
		SWREG_write_field(&reg_value, &reg_mask, ((uint32_t) bpsm_ctx.chenst), BPSM_REGISTER_STATUS_1_MASK_CHENST);
		// Backup_output state.
#ifdef BPSM_BKEN_FORCED_HARDWARE
		bpsm_ctx.bkenst = UNA_BIT_FORCED_HARDWARE;
#else
		bpsm_ctx.bkenst = (LOAD_get_output_state() == 0) ? UNA_BIT_0 : UNA_BIT_1;
#endif
		SWREG_write_field(&reg_value, &reg_mask, ((uint32_t) bpsm_ctx.bkenst), BPSM_REGISTER_STATUS_1_MASK_BKENST);
		break;
	default:
		// Nothing to do for other registers.
		break;
	}
	NODE_write_register(NODE_REQUEST_SOURCE_INTERNAL, reg_addr, reg_value, reg_mask);
	return status;
}

/*******************************************************************/
NODE_status_t BPSM_check_register(uint8_t reg_addr, uint32_t reg_mask) {
	// Local variables.
	NODE_status_t status = NODE_SUCCESS;
#ifndef BPSM_BKEN_FORCED_HARDWARE
	LOAD_status_t load_status = LOAD_SUCCESS;
	UNA_bit_representation_t bken = UNA_BIT_ERROR;
#endif
#ifndef BPSM_CHEN_FORCED_HARDWARE
	UNA_bit_representation_t chen = UNA_BIT_ERROR;
#endif
	uint32_t reg_value = 0;
	// Read register.
	status = NODE_read_register(NODE_REQUEST_SOURCE_INTERNAL, reg_addr, &reg_value);
	if (status != NODE_SUCCESS) goto errors;
	// Check address.
	switch (reg_addr) {
	case BPSM_REGISTER_ADDRESS_CONFIGURATION_1:
		// Store new value in NVM.
		if (reg_mask != 0) {
			status = NODE_write_nvm(reg_addr, reg_value);
			if (status != NODE_SUCCESS) goto errors;
		}
		break;
	case BPSM_REGISTER_ADDRESS_CONTROL_1:
		// CHEN.
		if ((reg_mask & BPSM_REGISTER_CONTROL_1_MASK_CHEN) != 0) {
#ifdef BPSM_CHEN_FORCED_HARDWARE
			status = NODE_ERROR_FORCED_HARDWARE;
			goto errors;
#else
			// Check control mode.
			if (SWREG_read_field(reg_value, BPSM_REGISTER_CONTROL_1_MASK_CHMD) != 0) {
				// Read bit.
				chen = SWREG_read_field(reg_value, BPSM_REGISTER_CONTROL_1_MASK_CHEN);
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
			bken = SWREG_read_field(reg_value, BPSM_REGISTER_CONTROL_1_MASK_BKEN);
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
		// Nothing to do for other registers.
		break;
	}
errors:
	// Update status register.
	BPSM_update_register(BPSM_REGISTER_ADDRESS_STATUS_1);
	return status;
}

/*******************************************************************/
NODE_status_t BPSM_mtrg_callback(void) {
	// Local variables.
	NODE_status_t status = NODE_SUCCESS;
	ANALOG_status_t analog_status = ANALOG_SUCCESS;
	int32_t adc_data = 0;
	uint32_t reg_analog_data_1 = 0;
	uint32_t reg_analog_data_1_mask = 0;
	uint32_t reg_analog_data_2 = 0;
	uint32_t reg_analog_data_2_mask = 0;
	// Reset results.
	_BPSM_reset_analog_data();
    // Relay common voltage.
    analog_status = ANALOG_convert_channel(ANALOG_CHANNEL_VSRC_MV, &adc_data);
    ANALOG_exit_error(NODE_ERROR_BASE_ANALOG);
    SWREG_write_field(&reg_analog_data_1, &reg_analog_data_1_mask, (uint32_t) UNA_convert_mv(adc_data), BPSM_REGISTER_ANALOG_DATA_1_MASK_VSRC);
    // Relay output voltage.
    analog_status = ANALOG_convert_channel(ANALOG_CHANNEL_VSTR_MV, &adc_data);
    ANALOG_exit_error(NODE_ERROR_BASE_ANALOG);
    SWREG_write_field(&reg_analog_data_1, &reg_analog_data_1_mask, (uint32_t) UNA_convert_mv(adc_data), BPSM_REGISTER_ANALOG_DATA_1_MASK_VSTR);
    // Relay output current.
    analog_status = ANALOG_convert_channel(ANALOG_CHANNEL_VBKP_MV, &adc_data);
    ANALOG_exit_error(NODE_ERROR_BASE_ANALOG);
    SWREG_write_field(&reg_analog_data_2, &reg_analog_data_2_mask, (uint32_t) UNA_convert_mv(adc_data), BPSM_REGISTER_ANALOG_DATA_2_MASK_VBKP);
    // Write registers.
    NODE_write_register(NODE_REQUEST_SOURCE_INTERNAL, BPSM_REGISTER_ADDRESS_ANALOG_DATA_1, reg_analog_data_1, reg_analog_data_1_mask);
    NODE_write_register(NODE_REQUEST_SOURCE_INTERNAL, BPSM_REGISTER_ADDRESS_ANALOG_DATA_2, reg_analog_data_2, reg_analog_data_2_mask);
errors:
	return status;
}

#ifndef BPSM_CHEN_FORCED_HARDWARE
/*******************************************************************/
NODE_status_t BPSM_charge_process(void) {
	// Local variables.
	NODE_status_t status = NODE_SUCCESS;
	ANALOG_status_t analog_status = ANALOG_SUCCESS;
	uint32_t reg_control_1 = 0;
	uint32_t reg_config_1 = 0;
	int32_t vsrc_mv = 0;
	// Read control register.
	NODE_read_register(NODE_REQUEST_SOURCE_INTERNAL, BPSM_REGISTER_ADDRESS_CONTROL_1, &reg_control_1);
	// Check mode.
	if (SWREG_read_field(reg_control_1, BPSM_REGISTER_CONTROL_1_MASK_CHMD) == 0) {
	    // Turn analog front-end on.
        POWER_enable(POWER_REQUESTER_ID_BPSM, POWER_DOMAIN_ANALOG, LPTIM_DELAY_MODE_ACTIVE);
		// Check source voltage.
		analog_status = ANALOG_convert_channel(ANALOG_CHANNEL_VSRC_MV, &vsrc_mv);
		ANALOG_exit_error(NODE_ERROR_BASE_ANALOG);
		// Read threshold and period.
		NODE_read_register(NODE_REQUEST_SOURCE_INTERNAL, BPSM_REGISTER_ADDRESS_CONFIGURATION_1, &reg_config_1);
		// Check voltage.
        if (vsrc_mv >= UNA_get_mv(SWREG_read_field(reg_config_1, BPSM_REGISTER_CONFIGURATION_1_MASK_CHEN_THRESHOLD))) {
            // Check toggle period.
            if (RTC_get_uptime_seconds() >= bpsm_ctx.chen_toggle_next_time_seconds) {
                // Update times.
                bpsm_ctx.chen_toggle_previous_time_seconds = RTC_get_uptime_seconds();
                bpsm_ctx.chen_toggle_next_time_seconds = RTC_get_uptime_seconds() + UNA_get_seconds(SWREG_read_field(reg_config_1, BPSM_REGISTER_CONFIGURATION_1_MASK_CHEN_TOGGLE_PERIOD));
                // Disable charge.
                LOAD_set_charge_state(0);
            }
            else {
                if (RTC_get_uptime_seconds() >= (bpsm_ctx.chen_toggle_previous_time_seconds + BPSM_CHEN_TOGGLE_DURATION_SECONDS)) {
                    // Enable charge.
                    LOAD_set_charge_state(1);
                }
            }
        }
        else {
            // Disable charge.
            LOAD_set_charge_state(0);
        }
	}
errors:
    POWER_disable(POWER_REQUESTER_ID_BPSM, POWER_DOMAIN_ANALOG);
	return status;
}
#endif

#endif /* BPSM */

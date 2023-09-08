/*
 * rrm.c
 *
 *  Created on: 10 jun. 2023
 *      Author: Ludo
 */

#include "rrm.h"

#include "adc.h"
#include "dinfox.h"
#include "error.h"
#include "load.h"
#include "rrm_reg.h"
#include "node.h"

/*** RRM local structures ***/

typedef union {
	struct {
		unsigned ren : 1;
	};
	uint8_t all;
} RRM_flags_t;

/*** RRM local global variables ***/

#ifdef RRM
static RRM_flags_t rrm_flags;
#endif

/*** RRM local functions ***/

#ifdef RRM
/*******************************************************************/
static void _RRM_reset_analog_data(void) {
	// Local variables.
	uint32_t analog_data_1 = 0;
	uint32_t analog_data_1_mask = 0;
	uint32_t analog_data_2 = 0;
	uint32_t analog_data_2_mask = 0;
	// Vin / Vout.
	DINFOX_write_field(&analog_data_1, &analog_data_1_mask, DINFOX_VOLTAGE_ERROR_VALUE, RRM_REG_ANALOG_DATA_1_MASK_VIN);
	DINFOX_write_field(&analog_data_1, &analog_data_1_mask, DINFOX_VOLTAGE_ERROR_VALUE, RRM_REG_ANALOG_DATA_1_MASK_VOUT);
	NODE_write_register(NODE_REQUEST_SOURCE_INTERNAL, RRM_REG_ADDR_ANALOG_DATA_1, analog_data_1_mask, analog_data_1);
	// Iout.
	DINFOX_write_field(&analog_data_2, &analog_data_2_mask, DINFOX_VOLTAGE_ERROR_VALUE, RRM_REG_ANALOG_DATA_2_MASK_IOUT);
	NODE_write_register(NODE_REQUEST_SOURCE_INTERNAL, RRM_REG_ADDR_ANALOG_DATA_2, analog_data_2_mask, analog_data_2);
}
#endif

/*** RRM functions ***/

#ifdef RRM
/*******************************************************************/
void RRM_init_registers(void) {
	// Status and control register 1.
	RRM_update_register(RRM_REG_ADDR_STATUS_CONTROL_1);
	// Load default values.
	_RRM_reset_analog_data();
}
#endif

#ifdef RRM
/*******************************************************************/
NODE_status_t RRM_update_register(uint8_t reg_addr) {
	// Local variables.
	NODE_status_t status = NODE_SUCCESS;
	LOAD_status_t load_status = LOAD_SUCCESS;
	uint8_t state = 0;
	uint32_t reg_value = 0;
	uint32_t reg_mask = 0;
	// Check address.
	switch (reg_addr) {
	case RRM_REG_ADDR_STATUS_CONTROL_1:
		// Relay state.
		load_status = LOAD_get_output_state(&state);
		LOAD_exit_error(NODE_ERROR_BASE_LOAD);
		rrm_flags.ren = (state == 0) ? 0b0 : 0b1;
		DINFOX_write_field(&reg_value, &reg_mask, rrm_flags.ren, RRM_REG_STATUS_CONTROL_1_MASK_REN);
		break;
	default:
		// Nothing to do for other registers.
		break;
	}
errors:
	NODE_write_register(NODE_REQUEST_SOURCE_INTERNAL, reg_addr, reg_mask, reg_value);
	return status;
}
#endif

#ifdef RRM
/*******************************************************************/
NODE_status_t RRM_check_register(uint8_t reg_addr) {
	// Local variables.
	NODE_status_t status = NODE_SUCCESS;
	LOAD_status_t load_status = LOAD_SUCCESS;
	uint32_t reg_value = 0;
	// Read register.
	NODE_read_register(NODE_REQUEST_SOURCE_INTERNAL, reg_addr, &reg_value);
	// Check address.
	switch (reg_addr) {
	case RRM_REG_ADDR_STATUS_CONTROL_1:
		// Read REN bit.
		if (DINFOX_read_field(reg_value, RRM_REG_STATUS_CONTROL_1_MASK_REN) != rrm_flags.ren) {
			// Update local flag.
			rrm_flags.ren = DINFOX_read_field(reg_value, RRM_REG_STATUS_CONTROL_1_MASK_REN);
			// Set DC-DC state.
			load_status = LOAD_set_output_state((uint8_t) rrm_flags.ren);
			LOAD_exit_error(NODE_ERROR_BASE_LOAD);
		}
		break;
	default:
		// Nothing to do for other registers.
		break;
	}
errors:
	return status;
}
#endif

#ifdef RRM
/*******************************************************************/
NODE_status_t RRM_mtrg_callback(ADC_status_t* adc_status) {
	// Local variables.
	NODE_status_t status = NODE_SUCCESS;
	POWER_status_t power_status = POWER_SUCCESS;
	ADC_status_t adc1_status = ADC_SUCCESS;
	uint32_t adc_data = 0;
	uint32_t analog_data_1 = 0;
	uint32_t analog_data_1_mask = 0;
	uint32_t analog_data_2 = 0;
	uint32_t analog_data_2_mask = 0;
	// Reset result.
	_RRM_reset_analog_data();
	// Perform analog measurements.
	power_status = POWER_enable(POWER_DOMAIN_ANALOG, LPTIM_DELAY_MODE_ACTIVE);
	POWER_exit_error(NODE_ERROR_BASE_POWER);
	adc1_status = ADC1_perform_measurements();
	ADC1_exit_error(NODE_ERROR_BASE_ADC1);
	power_status = POWER_disable(POWER_DOMAIN_ANALOG);
	POWER_exit_error(NODE_ERROR_BASE_POWER);
	// Check status.
	if (adc1_status == ADC_SUCCESS) {
		// Relay common voltage.
		adc1_status = ADC1_get_data(ADC_DATA_INDEX_VIN_MV, &adc_data);
		ADC1_exit_error(NODE_ERROR_BASE_ADC1);
		if (adc1_status == ADC_SUCCESS) {
			DINFOX_write_field(&analog_data_1, &analog_data_1_mask, (uint32_t) DINFOX_convert_mv(adc_data), RRM_REG_ANALOG_DATA_1_MASK_VIN);
		}
		// Relay output voltage.
		adc1_status = ADC1_get_data(ADC_DATA_INDEX_VOUT_MV, &adc_data);
		ADC1_exit_error(NODE_ERROR_BASE_ADC1);
		if (adc1_status == ADC_SUCCESS) {
			DINFOX_write_field(&analog_data_1, &analog_data_1_mask, (uint32_t) DINFOX_convert_mv(adc_data), RRM_REG_ANALOG_DATA_1_MASK_VOUT);
		}
		// Relay output current.
		adc1_status = ADC1_get_data(ADC_DATA_INDEX_IOUT_UA, &adc_data);
		ADC1_exit_error(NODE_ERROR_BASE_ADC1);
		if (adc1_status == ADC_SUCCESS) {
			DINFOX_write_field(&analog_data_2, &analog_data_2_mask, (uint32_t) DINFOX_convert_ua(adc_data), RRM_REG_ANALOG_DATA_2_MASK_IOUT);
		}
		// Write registers.
		NODE_write_register(NODE_REQUEST_SOURCE_INTERNAL, RRM_REG_ADDR_ANALOG_DATA_1, analog_data_1_mask, analog_data_1);
		NODE_write_register(NODE_REQUEST_SOURCE_INTERNAL, RRM_REG_ADDR_ANALOG_DATA_2, analog_data_2_mask, analog_data_2);
	}
	// Update ADC status.
	if (adc_status != NULL) {
		(*adc_status) = adc1_status;
	}
	return status;
errors:
	POWER_disable(POWER_DOMAIN_ANALOG);
	// Update ADC status.
	if (adc_status != NULL) {
		(*adc_status) = adc1_status;
	}
	return status;
}
#endif

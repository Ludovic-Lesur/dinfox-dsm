/*
 * lvrm.c
 *
 *  Created on: 4 jun. 2023
 *      Author: Ludo
 */

#include "lvrm.h"

#include "adc.h"
#include "dinfox.h"
#include "error.h"
#include "load.h"
#include "lvrm_reg.h"
#include "node.h"

/*** LVRM local structures ***/

typedef union {
	struct {
		unsigned rlst : 1;
	};
	uint8_t all;
} LVRM_flags_t;

/*** LVRM local global variables ***/

#ifdef LVRM
static LVRM_flags_t lvrm_flags;
#endif

/*** LVRM local functions ***/

#ifdef LVRM
/*******************************************************************/
static void _LVRM_reset_analog_data(void) {
	// Local variables.
	NODE_status_t node_status = NODE_SUCCESS;
	uint32_t analog_data_1 = 0;
	uint32_t analog_data_1_mask = 0;
	uint32_t analog_data_2 = 0;
	uint32_t analog_data_2_mask = 0;
	// Vin / Vout.
	DINFOX_write_field(&analog_data_1, &analog_data_1_mask, DINFOX_VOLTAGE_ERROR_VALUE, LVRM_REG_ANALOG_DATA_1_MASK_VCOM);
	DINFOX_write_field(&analog_data_1, &analog_data_1_mask, DINFOX_VOLTAGE_ERROR_VALUE, LVRM_REG_ANALOG_DATA_1_MASK_VOUT);
	node_status = NODE_write_register(NODE_REQUEST_SOURCE_INTERNAL, LVRM_REG_ADDR_ANALOG_DATA_1, analog_data_1_mask, analog_data_1);
	NODE_stack_error();
	// Iout.
	DINFOX_write_field(&analog_data_2, &analog_data_2_mask, DINFOX_VOLTAGE_ERROR_VALUE, LVRM_REG_ANALOG_DATA_2_MASK_IOUT);
	node_status = NODE_write_register(NODE_REQUEST_SOURCE_INTERNAL, LVRM_REG_ADDR_ANALOG_DATA_2, analog_data_2_mask, analog_data_2);
	NODE_stack_error();
}
#endif

/*** LVRM functions ***/

#ifdef LVRM
/*******************************************************************/
void LVRM_init_registers(void) {
	// Local variables.
	NODE_status_t node_status = NODE_SUCCESS;
	// Status and control register 1.
	node_status = LVRM_update_register(LVRM_REG_ADDR_STATUS_CONTROL_1);
	NODE_stack_error();
	// Load defaults values.
	_LVRM_reset_analog_data();
}
#endif

#ifdef LVRM
/*******************************************************************/
NODE_status_t LVRM_update_register(uint8_t reg_addr) {
	// Local variables.
	NODE_status_t status = NODE_SUCCESS;
	NODE_status_t node_status = NODE_SUCCESS;
	LOAD_status_t load_status = LOAD_SUCCESS;
	uint8_t state = 0;
	uint32_t reg_value = 0;
	uint32_t reg_mask = 0;
	// Check address.
	switch (reg_addr) {
	case LVRM_REG_ADDR_STATUS_CONTROL_1:
		// Relay state.
		load_status = LOAD_get_output_state(&state);
		LOAD_stack_error();
		lvrm_flags.rlst = (state == 0) ? 0b0 : 0b1;
		DINFOX_write_field(&reg_value, &reg_mask, lvrm_flags.rlst, LVRM_REG_STATUS_CONTROL_1_MASK_RLST);
		break;
	default:
		// Nothing to do for other registers.
		break;
	}
	// Write register.
	node_status = NODE_write_register(NODE_REQUEST_SOURCE_INTERNAL, reg_addr, reg_mask, reg_value);
	NODE_stack_error();

	return status;
}
#endif

#ifdef LVRM
/*******************************************************************/
NODE_status_t LVRM_check_register(uint8_t reg_addr) {
	// Local variables.
	NODE_status_t status = NODE_SUCCESS;
	NODE_status_t node_status = NODE_SUCCESS;
	LOAD_status_t load_status = LOAD_SUCCESS;
	uint32_t reg_value = 0;
	// Read register.
	node_status = NODE_read_register(NODE_REQUEST_SOURCE_INTERNAL, reg_addr, &reg_value);
	NODE_stack_error();
	// Check address.
	switch (reg_addr) {
	case LVRM_REG_ADDR_STATUS_CONTROL_1:
		// Read DDEN bit.
		if (DINFOX_read_field(reg_value, LVRM_REG_STATUS_CONTROL_1_MASK_RLST) != lvrm_flags.rlst) {
			// Update local flag.
			lvrm_flags.rlst = DINFOX_read_field(reg_value, LVRM_REG_STATUS_CONTROL_1_MASK_RLST);
			// Set DC-DC state.
			load_status = LOAD_set_output_state((uint8_t) lvrm_flags.rlst);
			LOAD_stack_error();
		}
		break;
	default:
		// Nothing to do for other registers.
		break;
	}
	return status;
}
#endif

#ifdef LVRM
/*******************************************************************/
NODE_status_t LVRM_mtrg_callback(ADC_status_t* adc_status) {
	// Local variables.
	NODE_status_t status = NODE_SUCCESS;
	NODE_status_t node_status = NODE_SUCCESS;
	POWER_status_t power_status = POWER_SUCCESS;
	ADC_status_t adc1_status = ADC_SUCCESS;
	uint32_t adc_data = 0;
	uint32_t analog_data_1 = 0;
	uint32_t analog_data_1_mask = 0;
	uint32_t analog_data_2 = 0;
	uint32_t analog_data_2_mask = 0;
	// Reset results.
	_LVRM_reset_analog_data();
	// Perform analog measurements.
	power_status = POWER_enable(POWER_DOMAIN_ANALOG, LPTIM_DELAY_MODE_ACTIVE);
	POWER_stack_error();
	adc1_status = ADC1_perform_measurements();
	ADC1_stack_error();
	power_status = POWER_disable(POWER_DOMAIN_ANALOG);
	POWER_stack_error();
	// Update parameter.
	if (adc_status != NULL) {
		(*adc_status) = adc1_status;
	}
	// Check status.
	if (adc1_status == ADC_SUCCESS) {
		// Relay common voltage.
		adc1_status = ADC1_get_data(ADC_DATA_INDEX_VCOM_MV, &adc_data);
		ADC1_stack_error();
		if (adc1_status == ADC_SUCCESS) {
			DINFOX_write_field(&analog_data_1, &analog_data_1_mask, (uint32_t) DINFOX_convert_mv(adc_data), LVRM_REG_ANALOG_DATA_1_MASK_VCOM);
		}
		// Relay output voltage.
		adc1_status = ADC1_get_data(ADC_DATA_INDEX_VOUT_MV, &adc_data);
		ADC1_stack_error();
		if (adc1_status == ADC_SUCCESS) {
			DINFOX_write_field(&analog_data_1, &analog_data_1_mask, (uint32_t) DINFOX_convert_mv(adc_data), LVRM_REG_ANALOG_DATA_1_MASK_VOUT);
		}
		// Relay output current.
		adc1_status = ADC1_get_data(ADC_DATA_INDEX_IOUT_UA, &adc_data);
		ADC1_stack_error();
		if (adc1_status == ADC_SUCCESS) {
			DINFOX_write_field(&analog_data_2, &analog_data_2_mask, (uint32_t) DINFOX_convert_mv(adc_data), LVRM_REG_ANALOG_DATA_2_MASK_IOUT);
		}
		// Write registers.
		node_status = NODE_write_register(NODE_REQUEST_SOURCE_INTERNAL, LVRM_REG_ADDR_ANALOG_DATA_1, analog_data_1_mask, analog_data_1);
		NODE_stack_error();
		node_status = NODE_write_register(NODE_REQUEST_SOURCE_INTERNAL, LVRM_REG_ADDR_ANALOG_DATA_2, analog_data_2_mask, analog_data_2);
		NODE_stack_error();
	}
	return status;
}
#endif

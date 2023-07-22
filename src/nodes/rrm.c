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
	NODE_status_t node_status = NODE_SUCCESS;
	// Reset fields to error value.
	node_status = NODE_write_field(NODE_REQUEST_SOURCE_INTERNAL, RRM_REG_ADDR_ANALOG_DATA_1, RRM_REG_ANALOG_DATA_1_MASK_VIN, DINFOX_VOLTAGE_ERROR_VALUE);
	NODE_stack_error();
	node_status = NODE_write_field(NODE_REQUEST_SOURCE_INTERNAL, RRM_REG_ADDR_ANALOG_DATA_1, RRM_REG_ANALOG_DATA_1_MASK_VOUT, DINFOX_VOLTAGE_ERROR_VALUE);
	NODE_stack_error();
	node_status = NODE_write_field(NODE_REQUEST_SOURCE_INTERNAL, RRM_REG_ADDR_ANALOG_DATA_2, RRM_REG_ANALOG_DATA_2_MASK_IOUT, DINFOX_CURRENT_ERROR_VALUE);
	NODE_stack_error();
}
#endif

/*** RRM functions ***/

#ifdef RRM
/*******************************************************************/
void RRM_init_registers(void) {
	// Local variables.
	NODE_status_t node_status = NODE_SUCCESS;
	LOAD_status_t load_status = LOAD_SUCCESS;
	uint8_t dc_dc_state = 0;
	// Read init state.
	load_status = LOAD_get_output_state(&dc_dc_state);
	LOAD_stack_error();
	// Init context.
	rrm_flags.ren = (dc_dc_state == 0) ? 0 : 1;
	// Status and control register 1.
	node_status = NODE_write_field(NODE_REQUEST_SOURCE_INTERNAL, RRM_REG_ADDR_STATUS_CONTROL_1, RRM_REG_STATUS_CONTROL_1_MASK_REN, (uint32_t) dc_dc_state);
	NODE_stack_error();
	// Load default values.
	_RRM_reset_analog_data();
}
#endif

#ifdef RRM
/*******************************************************************/
NODE_status_t RRM_update_register(uint8_t reg_addr) {
	// Local variables.
	NODE_status_t status = NODE_SUCCESS;
	NODE_status_t node_status = NODE_SUCCESS;
	LOAD_status_t load_status = LOAD_SUCCESS;
	uint8_t dc_dc_state = 0;
	// Check address.
	switch (reg_addr) {
	case RRM_REG_ADDR_STATUS_CONTROL_1:
		// Relay state.
		load_status = LOAD_get_output_state(&dc_dc_state);
		LOAD_stack_error();
		node_status = NODE_write_field(NODE_REQUEST_SOURCE_INTERNAL, RRM_REG_ADDR_STATUS_CONTROL_1, RRM_REG_STATUS_CONTROL_1_MASK_REN, (uint32_t) dc_dc_state);
		NODE_stack_error();
		break;
	default:
		// Nothing to do for other registers.
		break;
	}
	return status;
}
#endif

#ifdef RRM
/*******************************************************************/
NODE_status_t RRM_check_register(uint8_t reg_addr) {
	// Local variables.
	NODE_status_t status = NODE_SUCCESS;
	NODE_status_t node_status = NODE_SUCCESS;
	LOAD_status_t load_status = LOAD_SUCCESS;
	uint32_t regulator_state = 0;
	// Check address.
	switch (reg_addr) {
	case RRM_REG_ADDR_STATUS_CONTROL_1:
		// Check relay control bit.
		node_status = NODE_read_field(NODE_REQUEST_SOURCE_INTERNAL, RRM_REG_ADDR_STATUS_CONTROL_1, RRM_REG_STATUS_CONTROL_1_MASK_REN, &regulator_state);
		NODE_stack_error();
		// Check bit change.
		if (rrm_flags.ren !=regulator_state) {
			// Set relay state.
			load_status = LOAD_set_output_state((uint8_t) regulator_state);
			LOAD_stack_error();
			// Update local flag.
			rrm_flags.ren = (regulator_state == 0) ? 0 : 1;
		}
		break;
	default:
		// Nothing to do for other registers.
		break;
	}
	return status;
}
#endif

#ifdef RRM
/*******************************************************************/
NODE_status_t RRM_mtrg_callback(ADC_status_t* adc_status) {
	// Local variables.
	NODE_status_t status = NODE_SUCCESS;
	NODE_status_t node_status = NODE_SUCCESS;
	ADC_status_t adc1_status = ADC_SUCCESS;
	uint32_t adc_data = 0;
	// Reset result.
	_RRM_reset_analog_data();
	// Perform measurements.
	adc1_status = ADC1_perform_measurements();
	ADC1_stack_error();
	// Update parameter.
	if (adc_status != NULL) {
		(*adc_status) = adc1_status;
	}
	// Check status.
	if (adc1_status == ADC_SUCCESS) {
		// Relay common voltage.
		adc1_status = ADC1_get_data(ADC_DATA_INDEX_VIN_MV, &adc_data);
		ADC1_stack_error();
		if (adc1_status == ADC_SUCCESS) {
			node_status = NODE_write_field(NODE_REQUEST_SOURCE_INTERNAL, RRM_REG_ADDR_ANALOG_DATA_1, RRM_REG_ANALOG_DATA_1_MASK_VIN, (uint32_t) DINFOX_convert_mv(adc_data));
			NODE_stack_error();
		}
		// Relay output voltage.
		adc1_status = ADC1_get_data(ADC_DATA_INDEX_VOUT_MV, &adc_data);
		ADC1_stack_error();
		if (adc1_status == ADC_SUCCESS) {
			node_status = NODE_write_field(NODE_REQUEST_SOURCE_INTERNAL, RRM_REG_ADDR_ANALOG_DATA_1, RRM_REG_ANALOG_DATA_1_MASK_VOUT, (uint32_t) DINFOX_convert_mv(adc_data));
			NODE_stack_error();
		}
		// Relay output current.
		adc1_status = ADC1_get_data(ADC_DATA_INDEX_IOUT_UA, &adc_data);
		ADC1_stack_error();
		if (adc1_status == ADC_SUCCESS) {
			node_status = NODE_write_field(NODE_REQUEST_SOURCE_INTERNAL, RRM_REG_ADDR_ANALOG_DATA_2, RRM_REG_ANALOG_DATA_2_MASK_IOUT, (uint32_t) DINFOX_convert_ua(adc_data));
			NODE_stack_error();
		}
	}
	return status;
}
#endif

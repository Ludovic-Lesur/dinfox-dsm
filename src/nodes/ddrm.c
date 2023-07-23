/*
 * ddrm.c
 *
 *  Created on: 4 jun. 2023
 *      Author: Ludo
 */

#include "ddrm.h"

#include "adc.h"
#include "dinfox.h"
#include "error.h"
#include "load.h"
#include "ddrm_reg.h"
#include "node.h"

/*** DDRM local structures ***/

typedef union {
	struct {
		unsigned dden : 1;
	};
	uint8_t all;
} DDRM_flags_t;

/*** DDRM local global variables ***/

#ifdef DDRM
static DDRM_flags_t ddrm_flags;
#endif

/*** DDRM local functions ***/

#ifdef DDRM
/*******************************************************************/
static void _DDRM_reset_analog_data(void) {
	// Local variables.
	NODE_status_t node_status = NODE_SUCCESS;
	// Reset fields to error value.
	node_status = NODE_write_field(NODE_REQUEST_SOURCE_INTERNAL, DDRM_REG_ADDR_ANALOG_DATA_1, DDRM_REG_ANALOG_DATA_1_MASK_VIN, DINFOX_VOLTAGE_ERROR_VALUE);
	NODE_stack_error();
	node_status = NODE_write_field(NODE_REQUEST_SOURCE_INTERNAL, DDRM_REG_ADDR_ANALOG_DATA_1, DDRM_REG_ANALOG_DATA_1_MASK_VOUT, DINFOX_VOLTAGE_ERROR_VALUE);
	NODE_stack_error();
	node_status = NODE_write_field(NODE_REQUEST_SOURCE_INTERNAL, DDRM_REG_ADDR_ANALOG_DATA_2, DDRM_REG_ANALOG_DATA_2_MASK_IOUT, DINFOX_CURRENT_ERROR_VALUE);
	NODE_stack_error();
}
#endif

/*** DDRM functions ***/

#ifdef DDRM
/*******************************************************************/
void DDRM_init_registers(void) {
	// Local variables.
	NODE_status_t node_status = NODE_SUCCESS;
	LOAD_status_t load_status = LOAD_SUCCESS;
	uint8_t dc_dc_state = 0;
	// Read init state.
	load_status = LOAD_get_output_state(&dc_dc_state);
	LOAD_stack_error();
	// Init context.
	ddrm_flags.dden = (dc_dc_state == 0) ? 0 : 1;
	// Status and control register 1.
	node_status = NODE_write_field(NODE_REQUEST_SOURCE_INTERNAL, DDRM_REG_ADDR_STATUS_CONTROL_1, DDRM_REG_STATUS_CONTROL_1_MASK_DDEN, (uint32_t) dc_dc_state);
	NODE_stack_error();
	// Load default values.
	_DDRM_reset_analog_data();
}
#endif

#ifdef DDRM
/*******************************************************************/
NODE_status_t DDRM_update_register(uint8_t reg_addr) {
	// Local variables.
	NODE_status_t status = NODE_SUCCESS;
	NODE_status_t node_status = NODE_SUCCESS;
	LOAD_status_t load_status = LOAD_SUCCESS;
	uint8_t dc_dc_state = 0;
	// Check address.
	switch (reg_addr) {
	case DDRM_REG_ADDR_STATUS_CONTROL_1:
		// Relay state.
		load_status = LOAD_get_output_state(&dc_dc_state);
		LOAD_stack_error();
		node_status = NODE_write_field(NODE_REQUEST_SOURCE_INTERNAL, DDRM_REG_ADDR_STATUS_CONTROL_1, DDRM_REG_STATUS_CONTROL_1_MASK_DDEN, (uint32_t) dc_dc_state);
		NODE_stack_error();
		break;
	default:
		// Nothing to do for other registers.
		break;
	}
	return status;
}
#endif

#ifdef DDRM
/*******************************************************************/
NODE_status_t DDRM_check_register(uint8_t reg_addr) {
	// Local variables.
	NODE_status_t status = NODE_SUCCESS;
	NODE_status_t node_status = NODE_SUCCESS;
	LOAD_status_t load_status = LOAD_SUCCESS;
	uint32_t dc_dc_state = 0;
	// Check address.
	switch (reg_addr) {
	case DDRM_REG_ADDR_STATUS_CONTROL_1:
		// Check relay control bit.
		node_status = NODE_read_field(NODE_REQUEST_SOURCE_INTERNAL, DDRM_REG_ADDR_STATUS_CONTROL_1, DDRM_REG_STATUS_CONTROL_1_MASK_DDEN, &dc_dc_state);
		NODE_stack_error();
		// Check bit change.
		if (((ddrm_flags.dden == 0) && (dc_dc_state != 0)) || ((ddrm_flags.dden != 0) && (dc_dc_state == 0))) {
			// Set relay state.
			load_status = LOAD_set_output_state((uint8_t) dc_dc_state);
			LOAD_stack_error();
			// Update local flag.
			ddrm_flags.dden = (dc_dc_state == 0) ? 0 : 1;
		}
		break;
	default:
		// Nothing to do for other registers.
		break;
	}
	return status;
}
#endif

#ifdef DDRM
/*******************************************************************/
NODE_status_t DDRM_mtrg_callback(ADC_status_t* adc_status) {
	// Local variables.
	NODE_status_t status = NODE_SUCCESS;
	NODE_status_t node_status = NODE_SUCCESS;
	POWER_status_t power_status = POWER_SUCCESS;
	ADC_status_t adc1_status = ADC_SUCCESS;
	uint32_t adc_data = 0;
	// Reset results.
	_DDRM_reset_analog_data();
	// Perform analog measurements.
	power_status = POWER_enable(POWER_DOMAIN_ANALOG, LPTIM_DELAY_MODE_SLEEP);
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
		adc1_status = ADC1_get_data(ADC_DATA_INDEX_VIN_MV, &adc_data);
		ADC1_stack_error();
		if (adc1_status == ADC_SUCCESS) {
			node_status = NODE_write_field(NODE_REQUEST_SOURCE_INTERNAL, DDRM_REG_ADDR_ANALOG_DATA_1, DDRM_REG_ANALOG_DATA_1_MASK_VIN, (uint32_t) DINFOX_convert_mv(adc_data));
			NODE_stack_error();
		}
		// Relay output voltage.
		adc1_status = ADC1_get_data(ADC_DATA_INDEX_VOUT_MV, &adc_data);
		ADC1_stack_error();
		if (adc1_status == ADC_SUCCESS) {
			node_status = NODE_write_field(NODE_REQUEST_SOURCE_INTERNAL, DDRM_REG_ADDR_ANALOG_DATA_1, DDRM_REG_ANALOG_DATA_1_MASK_VOUT, (uint32_t) DINFOX_convert_mv(adc_data));
			NODE_stack_error();
		}
		// Relay output current.
		adc1_status = ADC1_get_data(ADC_DATA_INDEX_IOUT_UA, &adc_data);
		ADC1_stack_error();
		if (adc1_status == ADC_SUCCESS) {
			node_status = NODE_write_field(NODE_REQUEST_SOURCE_INTERNAL, DDRM_REG_ADDR_ANALOG_DATA_2, DDRM_REG_ANALOG_DATA_2_MASK_IOUT, (uint32_t) DINFOX_convert_ua(adc_data));
			NODE_stack_error();
		}
	}
	return status;
}
#endif

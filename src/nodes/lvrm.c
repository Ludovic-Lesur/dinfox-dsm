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

#ifdef LVRM

/*** LVRM local structures ***/

typedef union {
	struct {
		unsigned rlst : 1;
	};
	uint8_t all;
} LVRM_flags_t;

/*** LVRM local global variables ***/

static LVRM_flags_t lvrm_flags;

/*** LVRM local functions ***/

/* RESET RRM ANALOG DATA.
 * @param:	None.
 * @return:	None.
 */
static void _LVRM_reset_analog_data(void) {
	// Local variables.
	NODE_status_t node_status = NODE_SUCCESS;
	// Reset fields to error value.
	node_status = NODE_write_field(NODE_REQUEST_SOURCE_INTERNAL, LVRM_REG_ADDR_ANALOG_DATA_1, LVRM_REG_ANALOG_DATA_1_MASK_VCOM, DINFOX_VOLTAGE_ERROR_VALUE);
	NODE_stack_error();
	node_status = NODE_write_field(NODE_REQUEST_SOURCE_INTERNAL, LVRM_REG_ADDR_ANALOG_DATA_1, LVRM_REG_ANALOG_DATA_1_MASK_VOUT, DINFOX_VOLTAGE_ERROR_VALUE);
	NODE_stack_error();
	node_status = NODE_write_field(NODE_REQUEST_SOURCE_INTERNAL, LVRM_REG_ADDR_ANALOG_DATA_2, LVRM_REG_ANALOG_DATA_2_MASK_IOUT, DINFOX_CURRENT_ERROR_VALUE);
	NODE_stack_error();
}

/*** LVRM functions ***/

/* INIT LVRM REGISTERS.
 * @param:	None.
 * @return:	None.
 */
void LVRM_init_registers(void) {
	// Local variables.
	NODE_status_t node_status = NODE_SUCCESS;
	LOAD_status_t load_status = LOAD_SUCCESS;
	uint8_t relay_state = 0;
	// Read init state.
	load_status = LOAD_get_output_state(&relay_state);
	LOAD_stack_error();
	// Init context.
	lvrm_flags.rlst = (relay_state == 0) ? 0 : 1;
	// Status and control register 1.
	node_status = NODE_write_field(NODE_REQUEST_SOURCE_INTERNAL, LVRM_REG_ADDR_STATUS_CONTROL_1, LVRM_REG_STATUS_CONTROL_1_MASK_RLST, (uint32_t) relay_state);
	NODE_stack_error();
	// Load defaults values.
	_LVRM_reset_analog_data();
}

/* UPDATE LVRM REGISTER.
 * @param reg_addr:	Address of the register to update.
 * @return status:	Function execution status.
 */
NODE_status_t LVRM_update_register(uint8_t reg_addr) {
	// Local variables.
	NODE_status_t status = NODE_SUCCESS;
	NODE_status_t node_status = NODE_SUCCESS;
	LOAD_status_t load_status = LOAD_SUCCESS;
	uint8_t relay_state = 0;
	// Check address.
	switch (reg_addr) {
	case LVRM_REG_ADDR_STATUS_CONTROL_1:
		// Relay state.
		load_status = LOAD_get_output_state(&relay_state);
		LOAD_stack_error();
		node_status = NODE_write_field(NODE_REQUEST_SOURCE_INTERNAL, LVRM_REG_ADDR_STATUS_CONTROL_1, LVRM_REG_STATUS_CONTROL_1_MASK_RLST, (uint32_t) relay_state);
		NODE_stack_error();
		break;
	default:
		// Nothing to do for other registers.
		break;
	}
	return status;
}

/* CHECK LVRM NODE ACTIONS.
 * @param reg_addr:	Address of the register to check.
 * @return status:	Function execution status.
 */
NODE_status_t LVRM_check_register(uint8_t reg_addr) {
	// Local variables.
	NODE_status_t status = NODE_SUCCESS;
	NODE_status_t node_status = NODE_SUCCESS;
	LOAD_status_t load_status = LOAD_SUCCESS;
	uint32_t relay_state = 0;
	// Check address.
	switch (reg_addr) {
	case LVRM_REG_ADDR_STATUS_CONTROL_1:
		// Check relay control bit.
		node_status = NODE_read_field(NODE_REQUEST_SOURCE_INTERNAL, LVRM_REG_ADDR_STATUS_CONTROL_1, LVRM_REG_STATUS_CONTROL_1_MASK_RLST, &relay_state);
		NODE_stack_error();
		// Check bit change.
		if (lvrm_flags.rlst != relay_state) {
			// Set relay state.
			load_status = LOAD_set_output_state((uint8_t) relay_state);
			LOAD_stack_error();
			// Update local flag.
			lvrm_flags.rlst = relay_state;
		}
		break;
	default:
		// Nothing to do for other registers.
		break;
	}
	return status;
}

/* MEASURE TRIGGER CALLBACK.
 * @param adc_status:	Pointer to the ADC measurements status.
 * @return status:		Function execution status.
 */
NODE_status_t LVRM_mtrg_callback(ADC_status_t* adc_status) {
	// Local variables.
	NODE_status_t status = NODE_SUCCESS;
	NODE_status_t node_status = NODE_SUCCESS;
	ADC_status_t adc1_status = ADC_SUCCESS;
	uint32_t adc_data = 0;
	// Reset results.
	_LVRM_reset_analog_data();
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
		adc1_status = ADC1_get_data(ADC_DATA_INDEX_VCOM_MV, &adc_data);
		ADC1_stack_error();
		if (adc1_status == ADC_SUCCESS) {
			node_status = NODE_write_field(NODE_REQUEST_SOURCE_INTERNAL, LVRM_REG_ADDR_ANALOG_DATA_1, LVRM_REG_ANALOG_DATA_1_MASK_VCOM, (uint32_t) DINFOX_convert_mv(adc_data));
			NODE_stack_error();
		}
		// Relay output voltage.
		adc1_status = ADC1_get_data(ADC_DATA_INDEX_VOUT_MV, &adc_data);
		ADC1_stack_error();
		if (adc1_status == ADC_SUCCESS) {
			node_status = NODE_write_field(NODE_REQUEST_SOURCE_INTERNAL, LVRM_REG_ADDR_ANALOG_DATA_1, LVRM_REG_ANALOG_DATA_1_MASK_VOUT, (uint32_t) DINFOX_convert_mv(adc_data));
			NODE_stack_error();
		}
		// Relay output current.
		adc1_status = ADC1_get_data(ADC_DATA_INDEX_IOUT_UA, &adc_data);
		ADC1_stack_error();
		if (adc1_status == ADC_SUCCESS) {
			node_status = NODE_write_field(NODE_REQUEST_SOURCE_INTERNAL, LVRM_REG_ADDR_ANALOG_DATA_2, LVRM_REG_ANALOG_DATA_2_MASK_IOUT, (uint32_t) DINFOX_convert_ua(adc_data));
			NODE_stack_error();
		}
	}
	return status;
}

#endif /* LVRM */

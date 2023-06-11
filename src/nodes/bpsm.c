/*
 * bpsm.c
 *
 *  Created on: 4 jun. 2023
 *      Author: Ludo
 */

#include "bpsm.h"

#include "adc.h"
#include "dinfox_common.h"
#include "load.h"
#include "bpsm_reg.h"
#include "node.h"

#ifdef BPSM

/*** BPSM local structures ***/

typedef union {
	struct {
		unsigned chen : 1;
		unsigned bken : 1;
	};
	uint8_t all;
} BPSM_flags_t;

/*** BPSM local global variables ***/

static BPSM_flags_t bpsm_flags;

/*** BPSM local functions ***/

/* RESET BPSM ANALOG DATA.
 * @param:			None.
 * @return status:	Function execution status.
 */
static NODE_status_t _BPSM_reset_analog_data(void) {
	// Local variables.
	NODE_status_t status = NODE_SUCCESS;
	// Reset fields to error value.
	status = NODE_write_field(NODE_REQUEST_SOURCE_INTERNAL, BPSM_REG_ADDR_ANALOG_DATA_1, BPSM_REG_ANALOG_DATA_1_MASK_VSRC, DINFOX_VOLTAGE_ERROR_VALUE);
	if (status != NODE_SUCCESS) goto errors;
	status = NODE_write_field(NODE_REQUEST_SOURCE_INTERNAL, BPSM_REG_ADDR_ANALOG_DATA_1, BPSM_REG_ANALOG_DATA_1_MASK_VSTR, DINFOX_VOLTAGE_ERROR_VALUE);
	if (status != NODE_SUCCESS) goto errors;
	status = NODE_write_field(NODE_REQUEST_SOURCE_INTERNAL, BPSM_REG_ADDR_ANALOG_DATA_2, BPSM_REG_ANALOG_DATA_2_MASK_VBKP, DINFOX_VOLTAGE_ERROR_VALUE);
	if (status != NODE_SUCCESS) goto errors;
errors:
	return status;
}

/*** BPSM functions ***/

/* INIT BPSM REGISTERS.
 * @param:			None.
 * @return status:	Function execution status.
 */
NODE_status_t BPSM_init_registers(void) {
	// Local variables.
	NODE_status_t status = NODE_SUCCESS;
	LOAD_status_t load_status = LOAD_SUCCESS;
	uint8_t state = 0;
	// Read init state.
	state = LOAD_get_charge_state();
	// Init context.
	bpsm_flags.chen = (state == 0) ? 0 : 1;
	// Status and control register 1.
	status = NODE_write_field(NODE_REQUEST_SOURCE_INTERNAL, BPSM_REG_ADDR_STATUS_CONTROL_1, BPSM_REG_STATUS_CONTROL_1_MASK_CHEN, (uint32_t) state);
	// Read init state.
	load_status = LOAD_get_output_state(&state);
	LOAD_status_check(NODE_ERROR_BASE_LOAD);
	// Init context.
	bpsm_flags.bken = (state == 0) ? 0 : 1;
	// Status and control register 1.
	status = NODE_write_field(NODE_REQUEST_SOURCE_INTERNAL, BPSM_REG_ADDR_STATUS_CONTROL_1, BPSM_REG_STATUS_CONTROL_1_MASK_BKEN, (uint32_t) state);
	if (status != NODE_SUCCESS) goto errors;
	// Load default values.
	status = _BPSM_reset_analog_data();
	if (status != NODE_SUCCESS) goto errors;
errors:
	return status;
}

/* UPDATE BPSM REGISTER.
 * @param reg_addr:	Address of the register to update.
 * @return status:	Function execution status.
 */
NODE_status_t BPSM_update_register(uint8_t reg_addr) {
	// Local variables.
	NODE_status_t status = NODE_SUCCESS;
	LOAD_status_t load_status = LOAD_SUCCESS;
	uint8_t state = 0;
	// Check address.
	switch (reg_addr) {
	case BPSM_REG_ADDR_STATUS_CONTROL_1:
		// Charge status.
		state = LOAD_get_charge_status();
		// Write field.
		status = NODE_write_field(NODE_REQUEST_SOURCE_INTERNAL, BPSM_REG_ADDR_STATUS_CONTROL_1, BPSM_REG_STATUS_CONTROL_1_MASK_CHST, (uint32_t) state);
		if (status != NODE_SUCCESS) goto errors;
		// Charge state.
		state = LOAD_get_charge_state();
		// Write field.
		status = NODE_write_field(NODE_REQUEST_SOURCE_INTERNAL, BPSM_REG_ADDR_STATUS_CONTROL_1, BPSM_REG_STATUS_CONTROL_1_MASK_CHEN, (uint32_t) state);
		if (status != NODE_SUCCESS) goto errors;
		// Backup_output state.
		load_status = LOAD_get_output_state(&state);
		LOAD_status_check(NODE_ERROR_BASE_LOAD);
		// Write field.
		status = NODE_write_field(NODE_REQUEST_SOURCE_INTERNAL, BPSM_REG_ADDR_STATUS_CONTROL_1, BPSM_REG_STATUS_CONTROL_1_MASK_BKEN, (uint32_t) state);
		if (status != NODE_SUCCESS) goto errors;
		break;
	default:
		// Nothing to do for other registers.
		break;
	}
errors:
	return status;
}

/* CHECK BPSM NODE ACTIONS.
 * @param reg_addr:	Address of the register to check.
 * @return status:	Function execution status.
 */
NODE_status_t BPSM_check_register(uint8_t reg_addr) {
	// Local variables.
	NODE_status_t status = NODE_SUCCESS;
	LOAD_status_t load_status = LOAD_SUCCESS;
	uint32_t state = 0;
	// Check address.
	switch (reg_addr) {
	case BPSM_REG_ADDR_STATUS_CONTROL_1:
		// Check charge control bit.
		status = NODE_read_field(NODE_REQUEST_SOURCE_INTERNAL, BPSM_REG_ADDR_STATUS_CONTROL_1, BPSM_REG_STATUS_CONTROL_1_MASK_CHEN, &state);
		if (status != NODE_SUCCESS) goto errors;
		// Check bit change.
		if (((bpsm_flags.chen == 0) && (state != 0)) || ((bpsm_flags.chen != 0) && (state == 0))) {
			// Set charge state.
			LOAD_set_charge_state((uint8_t) state);
			// Update local flag.
			bpsm_flags.chen = (state == 0) ? 0 : 1;
		}
		// Check relay control bit.
		status = NODE_read_field(NODE_REQUEST_SOURCE_INTERNAL, BPSM_REG_ADDR_STATUS_CONTROL_1, BPSM_REG_STATUS_CONTROL_1_MASK_BKEN, &state);
		if (status != NODE_SUCCESS) goto errors;
		// Check bit change.
		if (((bpsm_flags.bken == 0) && (state != 0)) || ((bpsm_flags.bken != 0) && (state == 0))) {
			// Set relay state.
			load_status = LOAD_set_output_state((uint8_t) state);
			LOAD_status_check(NODE_ERROR_BASE_LOAD);
			// Update local flag.
			bpsm_flags.bken = (state == 0) ? 0 : 1;
		}
		break;
	default:
		// Nothing to do for other registers.
		break;
	}
errors:
	return status;
}

/* MEASURE TRIGGER CALLBACK.
 * @param:			None.
 * @return status:	Function execution status.
 */
NODE_status_t BPSM_mtrg_callback(void) {
	// Local variables.
	NODE_status_t status = NODE_SUCCESS;
	ADC_status_t adc1_status = ADC_SUCCESS;
	uint32_t adc_data = 0;
	// Reset results.
	status = _BPSM_reset_analog_data();
	if (status != NODE_SUCCESS) goto errors;
	// Perform measurements.
	adc1_status = ADC1_perform_measurements();
	ADC1_status_check(NODE_ERROR_BASE_ADC);
	// Relay common voltage.
	adc1_status = ADC1_get_data(ADC_DATA_INDEX_VSRC_MV, &adc_data);
	ADC1_status_check(NODE_ERROR_BASE_ADC);
	status = NODE_write_field(NODE_REQUEST_SOURCE_INTERNAL, BPSM_REG_ADDR_ANALOG_DATA_1, BPSM_REG_ANALOG_DATA_1_MASK_VSRC, (uint32_t) DINFOX_convert_mv(adc_data));
	if (status != NODE_SUCCESS) goto errors;
	// Relay output voltage.
	adc1_status = ADC1_get_data(ADC_DATA_INDEX_VSTR_MV, &adc_data);
	ADC1_status_check(NODE_ERROR_BASE_ADC);
	status = NODE_write_field(NODE_REQUEST_SOURCE_INTERNAL, BPSM_REG_ADDR_ANALOG_DATA_1, BPSM_REG_ANALOG_DATA_1_MASK_VSTR, (uint32_t) DINFOX_convert_mv(adc_data));
	if (status != NODE_SUCCESS) goto errors;
	// Relay output current.
	adc1_status = ADC1_get_data(ADC_DATA_INDEX_VBKP_MV, &adc_data);
	ADC1_status_check(NODE_ERROR_BASE_ADC);
	status = NODE_write_field(NODE_REQUEST_SOURCE_INTERNAL, BPSM_REG_ADDR_ANALOG_DATA_2, BPSM_REG_ANALOG_DATA_2_MASK_VBKP, (uint32_t) DINFOX_convert_mv(adc_data));
	if (status != NODE_SUCCESS) goto errors;
errors:
	return status;
}

#endif /* BPSM */

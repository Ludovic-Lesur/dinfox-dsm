/*
 * lvrm.c
 *
 *  Created on: 4 jun. 2023
 *      Author: Ludo
 */

#include "lvrm.h"

#include "adc.h"
#include "dinfox_common.h"
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

/* INIT LVRM REGISTERS.
 * @param:			None.
 * @return status:	Function execution status.
 */
NODE_status_t LVRM_init_registers(void) {
	// Local variables.
	NODE_status_t status = NODE_SUCCESS;
	LOAD_status_t load_status = LOAD_SUCCESS;
	uint8_t relay_state = 0;
	// Read init state.
	load_status = LOAD_get_output_state(&relay_state);
	LOAD_status_check(NODE_ERROR_BASE_LOAD);
	// Init context.
	lvrm_flags.rlst = (relay_state == 0) ? 0 : 1;
	// Status and control register 1.
	status = NODE_write_field(NODE_REQUEST_SOURCE_INTERNAL, LVRM_REG_ADDR_STATUS_CONTROL_1, LVRM_REG_STATUS_CONTROL_1_MASK_RLST, (uint32_t) relay_state);
	if (status != NODE_SUCCESS) goto errors;
errors:
	return status;
}

/* UPDATE LVRM REGISTER.
 * @param reg_addr:	Address of the register to update.
 * @return status:	Function execution status.
 */
NODE_status_t LVRM_update_register(uint8_t reg_addr) {
	// Local variables.
	NODE_status_t status = NODE_SUCCESS;
	LOAD_status_t load_status = LOAD_SUCCESS;
	uint8_t relay_state = 0;
	// Check address.
	switch (reg_addr) {
	case LVRM_REG_ADDR_STATUS_CONTROL_1:
		// Relay state.
		load_status = LOAD_get_output_state(&relay_state);
		LOAD_status_check(NODE_ERROR_BASE_LOAD);
		status = NODE_write_field(NODE_REQUEST_SOURCE_INTERNAL, LVRM_REG_ADDR_STATUS_CONTROL_1, LVRM_REG_STATUS_CONTROL_1_MASK_RLST, (uint32_t) relay_state);
		if (status != NODE_SUCCESS) goto errors;
		break;
	default:
		// Nothing to do for other registers.
		break;
	}
errors:
	return status;
}

/* CHECK LVRM NODE ACTIONS.
 * @param reg_addr:	Address of the register to check.
 * @return status:	Function execution status.
 */
NODE_status_t LVRM_check_register(uint8_t reg_addr) {
	// Local variables.
	NODE_status_t status = NODE_SUCCESS;
	LOAD_status_t load_status = LOAD_SUCCESS;
	uint32_t relay_state = 0;
	// Check address.
	switch (reg_addr) {
	case LVRM_REG_ADDR_STATUS_CONTROL_1:
		// Check relay control bit.
		status = NODE_read_field(NODE_REQUEST_SOURCE_INTERNAL, LVRM_REG_ADDR_STATUS_CONTROL_1, LVRM_REG_STATUS_CONTROL_1_MASK_RLST, &relay_state);
		if (status != NODE_SUCCESS) goto errors;
		// Check bit change.
		if (((lvrm_flags.rlst == 0) && (relay_state != 0)) || ((lvrm_flags.rlst != 0) && (relay_state == 0))) {
			// Set relay state.
			load_status = LOAD_set_output_state((uint8_t) relay_state);
			LOAD_status_check(NODE_ERROR_BASE_LOAD);
			// Update local flag.
			lvrm_flags.rlst = (relay_state == 0) ? 0 : 1;
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
NODE_status_t LVRM_mtrg_callback(void) {
	// Local variables.
	NODE_status_t status = NODE_SUCCESS;
	ADC_status_t adc1_status = ADC_SUCCESS;
	uint32_t adc_data = 0;
	// Perform measurements.
	adc1_status = ADC1_perform_measurements();
	ADC1_status_check(NODE_ERROR_BASE_ADC);
	// Relay common voltage.
	adc1_status = ADC1_get_data(ADC_DATA_INDEX_VCOM_MV, &adc_data);
	ADC1_status_check(NODE_ERROR_BASE_ADC);
	status = NODE_write_field(NODE_REQUEST_SOURCE_INTERNAL, LVRM_REG_ADDR_ANALOG_DATA_1, LVRM_REG_ANALOG_DATA_1_MASK_VCOM, (uint32_t) DINFOX_convert_mv(adc_data));
	if (status != NODE_SUCCESS) goto errors;
	// Relay output voltage.
	adc1_status = ADC1_get_data(ADC_DATA_INDEX_VOUT_MV, &adc_data);
	ADC1_status_check(NODE_ERROR_BASE_ADC);
	status = NODE_write_field(NODE_REQUEST_SOURCE_INTERNAL, LVRM_REG_ADDR_ANALOG_DATA_1, LVRM_REG_ANALOG_DATA_1_MASK_VOUT, (uint32_t) DINFOX_convert_mv(adc_data));
	if (status != NODE_SUCCESS) goto errors;
	// Relay output current.
	adc1_status = ADC1_get_data(ADC_DATA_INDEX_IOUT_UA, &adc_data);
	ADC1_status_check(NODE_ERROR_BASE_ADC);
	status = NODE_write_field(NODE_REQUEST_SOURCE_INTERNAL, LVRM_REG_ADDR_ANALOG_DATA_2, LVRM_REG_ANALOG_DATA_2_MASK_IOUT, (uint32_t) DINFOX_convert_ua(adc_data));
	if (status != NODE_SUCCESS) goto errors;
errors:
	return status;
}

#endif /* LVRM */

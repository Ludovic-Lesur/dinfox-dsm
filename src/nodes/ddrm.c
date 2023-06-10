/*
 * ddrm.c
 *
 *  Created on: 4 jun. 2023
 *      Author: Ludo
 */

#include "ddrm.h"

#include "adc.h"
#include "dinfox_types.h"
#include "load.h"
#include "ddrm_reg.h"
#include "node.h"

#ifdef DDRM

/*** DDRM local structures ***/

typedef union {
	struct {
		unsigned dden : 1;
	};
	uint8_t all;
} DDRM_flags_t;

/*** DDRM local global variables ***/

static DDRM_flags_t ddrm_flags;

/*** DDRM functions ***/

/* INIT DDRM REGISTERS.
 * @param:			None.
 * @return status:	Function execution status.
 */
NODE_status_t DDRM_init_registers(void) {
	// Local variables.
	NODE_status_t status = NODE_SUCCESS;
	LOAD_status_t load_status = LOAD_SUCCESS;
	uint8_t dc_dc_state = 0;
	// Read init state.
	load_status = LOAD_get_output_state(&dc_dc_state);
	LOAD_status_check(NODE_ERROR_BASE_LOAD);
	// Init context.
	ddrm_flags.dden = (dc_dc_state == 0) ? 0 : 1;
	// Status and control register 1.
	status = NODE_write_field(NODE_REQUEST_SOURCE_INTERNAL, DDRM_REG_ADDR_STATUS_CONTROL_1, DDRM_REG_STATUS_CONTROL_1_MASK_DDEN, (uint32_t) dc_dc_state);
	if (status != NODE_SUCCESS) goto errors;
errors:
	return status;
}

/* UPDATE DDRM VOLATILE REGISTERS.
 * @param reg_addr:	Address of the register to update.
 * @return status:	Function execution status.
 */
NODE_status_t DDRM_update_register(uint8_t reg_addr) {
	// Local variables.
	NODE_status_t status = NODE_SUCCESS;
	LOAD_status_t load_status = LOAD_SUCCESS;
	uint8_t dc_dc_state = 0;
	// Check address.
	switch (reg_addr) {
	case DDRM_REG_ADDR_STATUS_CONTROL_1:
		// Relay state.
		load_status = LOAD_get_output_state(&dc_dc_state);
		LOAD_status_check(NODE_ERROR_BASE_LOAD);
		status = NODE_write_field(NODE_REQUEST_SOURCE_INTERNAL, DDRM_REG_ADDR_STATUS_CONTROL_1, DDRM_REG_STATUS_CONTROL_1_MASK_DDEN, (uint32_t) dc_dc_state);
		if (status != NODE_SUCCESS) goto errors;
		break;
	default:
		// Nothing to do for other registers.
		break;
	}
errors:
	return status;
}

/* CHECK DDRM CONTROL REGISTERS.
 * @param reg_addr:	Address of the register to check.
 * @return status:	Function execution status.
 */
NODE_status_t DDRM_check_register(uint8_t reg_addr) {
	// Local variables.
	NODE_status_t status = NODE_SUCCESS;
	LOAD_status_t load_status = LOAD_SUCCESS;
	uint32_t dc_dc_state = 0;
	// Check address.
	switch (reg_addr) {
	case DDRM_REG_ADDR_STATUS_CONTROL_1:
		// Check relay control bit.
		status = NODE_read_field(NODE_REQUEST_SOURCE_INTERNAL, DDRM_REG_ADDR_STATUS_CONTROL_1, DDRM_REG_STATUS_CONTROL_1_MASK_DDEN, &dc_dc_state);
		if (status != NODE_SUCCESS) goto errors;
		// Check bit change.
		if (((ddrm_flags.dden == 0) && (dc_dc_state != 0)) || ((ddrm_flags.dden != 0) && (dc_dc_state == 0))) {
			// Set relay state.
			load_status = LOAD_set_output_state((uint8_t) dc_dc_state);
			LOAD_status_check(NODE_ERROR_BASE_LOAD);
			// Update local flag.
			ddrm_flags.dden = (dc_dc_state == 0) ? 0 : 1;
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
NODE_status_t DDRM_mtrg_callback(void) {
	// Local variables.
	NODE_status_t status = NODE_SUCCESS;
	ADC_status_t adc1_status = ADC_SUCCESS;
	uint32_t adc_data = 0;
	// Perform measurements.
	adc1_status = ADC1_perform_measurements();
	ADC1_status_check(NODE_ERROR_BASE_ADC);
	// Relay common voltage.
	adc1_status = ADC1_get_data(ADC_DATA_INDEX_VIN_MV, &adc_data);
	ADC1_status_check(NODE_ERROR_BASE_ADC);
	status = NODE_write_field(NODE_REQUEST_SOURCE_INTERNAL, DDRM_REG_ADDR_ANALOG_DATA_1, DDRM_REG_ANALOG_DATA_1_MASK_VIN, (uint32_t) DINFOX_TYPES_convert_mv(adc_data));
	if (status != NODE_SUCCESS) goto errors;
	// Relay output voltage.
	adc1_status = ADC1_get_data(ADC_DATA_INDEX_VOUT_MV, &adc_data);
	ADC1_status_check(NODE_ERROR_BASE_ADC);
	status = NODE_write_field(NODE_REQUEST_SOURCE_INTERNAL, DDRM_REG_ADDR_ANALOG_DATA_1, DDRM_REG_ANALOG_DATA_1_MASK_VOUT, (uint32_t) DINFOX_TYPES_convert_mv(adc_data));
	if (status != NODE_SUCCESS) goto errors;
	// Relay output current.
	adc1_status = ADC1_get_data(ADC_DATA_INDEX_IOUT_UA, &adc_data);
	ADC1_status_check(NODE_ERROR_BASE_ADC);
	status = NODE_write_field(NODE_REQUEST_SOURCE_INTERNAL, DDRM_REG_ADDR_ANALOG_DATA_2, DDRM_REG_ANALOG_DATA_2_MASK_IOUT, (uint32_t) DINFOX_TYPES_convert_ua(adc_data));
	if (status != NODE_SUCCESS) goto errors;
errors:
	return status;
}

#endif /* DDRM */

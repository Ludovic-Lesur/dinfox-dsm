/*
 * bpsm.c
 *
 *  Created on: 04 jun. 2023
 *      Author: Ludo
 */

#include "bpsm.h"

#include "adc.h"
#include "dinfox.h"
#include "error.h"
#include "load.h"
#include "bpsm_reg.h"
#include "node.h"

/*** BPSM local structures ***/

/*******************************************************************/
typedef struct {
	DINFOX_bit_representation_t chenst;
	DINFOX_bit_representation_t bkenst;
} BPSM_context_t;

/*** BPSM local global variables ***/

#ifdef BPSM
static BPSM_context_t bpsm_ctx;
#endif

/*** BPSM local functions ***/

#ifdef BPSM
/*******************************************************************/
static void _BPSM_reset_analog_data(void) {
	// Local variables.
	uint32_t analog_data_1 = 0;
	uint32_t analog_data_1_mask = 0;
	uint32_t analog_data_2 = 0;
	uint32_t analog_data_2_mask = 0;
	// VSRC / VSTR.
	DINFOX_write_field(&analog_data_1, &analog_data_1_mask, DINFOX_VOLTAGE_ERROR_VALUE, BPSM_REG_ANALOG_DATA_1_MASK_VSRC);
	DINFOX_write_field(&analog_data_1, &analog_data_1_mask, DINFOX_VOLTAGE_ERROR_VALUE, BPSM_REG_ANALOG_DATA_1_MASK_VSTR);
	NODE_write_register(NODE_REQUEST_SOURCE_INTERNAL, BPSM_REG_ADDR_ANALOG_DATA_1, analog_data_1_mask, analog_data_1);
	// VBKP.
	DINFOX_write_field(&analog_data_2, &analog_data_2_mask, DINFOX_VOLTAGE_ERROR_VALUE, BPSM_REG_ANALOG_DATA_2_MASK_VBKP);
	NODE_write_register(NODE_REQUEST_SOURCE_INTERNAL, BPSM_REG_ADDR_ANALOG_DATA_2, analog_data_2_mask, analog_data_2);
}
#endif

/*** BPSM functions ***/

#ifdef BPSM
/*******************************************************************/
void BPSM_init_registers(void) {
	// Read init state.
	BPSM_update_register(BPSM_REG_ADDR_STATUS);
	// Load default values.
	_BPSM_reset_analog_data();
}
#endif

#ifdef BPSM
/*******************************************************************/
NODE_status_t BPSM_update_register(uint8_t reg_addr) {
	// Local variables.
	NODE_status_t status = NODE_SUCCESS;
	uint32_t reg_value = 0;
	uint32_t reg_mask = 0;
	DINFOX_bit_representation_t chrgst = DINFOX_BIT_ERROR;
	// Check address.
	switch (reg_addr) {
	case BPSM_REG_ADDR_STATUS:
		// Voltage divider ratio.
		DINFOX_write_field(&reg_value, &reg_mask, BPSM_VSTR_VOLTAGE_DIVIDER_RATIO, BPSM_REG_STATUS_MASK_VSTR_RATIO);
		// Charge status.
		chrgst = LOAD_get_charge_status();
		DINFOX_write_field(&reg_value, &reg_mask, ((uint32_t) chrgst), BPSM_REG_STATUS_MASK_CHRGST);
		// Charge state.
		bpsm_ctx.chenst = LOAD_get_charge_state();
		DINFOX_write_field(&reg_value, &reg_mask, ((uint32_t) bpsm_ctx.chenst), BPSM_REG_STATUS_MASK_CHENST);
		// Backup_output state.
		bpsm_ctx.bkenst = LOAD_get_output_state();
		DINFOX_write_field(&reg_value, &reg_mask, ((uint32_t) bpsm_ctx.bkenst), BPSM_REG_STATUS_MASK_BKENST);
		break;
	default:
		// Nothing to do for other registers.
		break;
	}
	NODE_write_register(NODE_REQUEST_SOURCE_INTERNAL, reg_addr, reg_mask, reg_value);
	return status;
}
#endif

#ifdef BPSM
/*******************************************************************/
NODE_status_t BPSM_check_register(uint8_t reg_addr, uint32_t reg_mask) {
	// Local variables.
	NODE_status_t status = NODE_SUCCESS;
	LOAD_status_t load_status = LOAD_SUCCESS;
	uint32_t reg_value = 0;
	DINFOX_bit_representation_t chen = DINFOX_BIT_ERROR;
	DINFOX_bit_representation_t bken = DINFOX_BIT_ERROR;
	// Read register.
	NODE_read_register(NODE_REQUEST_SOURCE_INTERNAL, reg_addr, &reg_value);
	// Check address.
	switch (reg_addr) {
	case BPSM_REG_ADDR_CONTROL_1:
		// CHEN.
		if ((reg_mask & BPSM_REG_CONTROL_1_MASK_CHEN) != 0) {
			// Read bit.
			chen = DINFOX_read_field(reg_value, BPSM_REG_CONTROL_1_MASK_CHEN);
			// Compare to current state.
			if (chen != bpsm_ctx.chenst) {
				// Set charge state.
				LOAD_set_charge_state(chen);
			}
		}
		// BKEN.
		if ((reg_mask & BPSM_REG_CONTROL_1_MASK_BKEN) != 0) {
			// Read bit.
			bken = DINFOX_read_field(reg_value, BPSM_REG_CONTROL_1_MASK_BKEN);
			// Compare to current state.
			if (bken != bpsm_ctx.bkenst) {
				// Set output state.
				load_status = LOAD_set_output_state(bken);
				LOAD_exit_error(NODE_ERROR_BASE_LOAD);
			}
		}
		break;
	default:
		// Nothing to do for other registers.
		break;
	}
errors:
	// Update status register.
	BPSM_update_register(BPSM_REG_ADDR_STATUS);
	return status;
}
#endif

#ifdef BPSM
/*******************************************************************/
NODE_status_t BPSM_mtrg_callback(ADC_status_t* adc_status) {
	// Local variables.
	NODE_status_t status = NODE_SUCCESS;
	POWER_status_t power_status = POWER_SUCCESS;
	ADC_status_t adc1_status = ADC_SUCCESS;
	uint32_t adc_data = 0;
	uint32_t analog_data_1 = 0;
	uint32_t analog_data_1_mask = 0;
	uint32_t analog_data_2 = 0;
	uint32_t analog_data_2_mask = 0;
	// Reset results.
	_BPSM_reset_analog_data();
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
		adc1_status = ADC1_get_data(ADC_DATA_INDEX_VSRC_MV, &adc_data);
		ADC1_exit_error(NODE_ERROR_BASE_ADC1);
		if (adc1_status == ADC_SUCCESS) {
			DINFOX_write_field(&analog_data_1, &analog_data_1_mask, (uint32_t) DINFOX_convert_mv(adc_data), BPSM_REG_ANALOG_DATA_1_MASK_VSRC);
		}
		// Relay output voltage.
		adc1_status = ADC1_get_data(ADC_DATA_INDEX_VSTR_MV, &adc_data);
		ADC1_exit_error(NODE_ERROR_BASE_ADC1);
		if (adc1_status == ADC_SUCCESS) {
			DINFOX_write_field(&analog_data_1, &analog_data_1_mask, (uint32_t) DINFOX_convert_mv(adc_data), BPSM_REG_ANALOG_DATA_1_MASK_VSTR);
		}
		// Relay output current.
		adc1_status = ADC1_get_data(ADC_DATA_INDEX_VBKP_MV, &adc_data);
		ADC1_exit_error(NODE_ERROR_BASE_ADC1);
		if (adc1_status == ADC_SUCCESS) {
			DINFOX_write_field(&analog_data_2, &analog_data_2_mask, (uint32_t) DINFOX_convert_mv(adc_data), BPSM_REG_ANALOG_DATA_2_MASK_VBKP);
		}
		// Write registers.
		NODE_write_register(NODE_REQUEST_SOURCE_INTERNAL, BPSM_REG_ADDR_ANALOG_DATA_1, analog_data_1_mask, analog_data_1);
		NODE_write_register(NODE_REQUEST_SOURCE_INTERNAL, BPSM_REG_ADDR_ANALOG_DATA_2, analog_data_2_mask, analog_data_2);
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

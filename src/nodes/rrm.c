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

/*** RRM local macros ***/

// Note: IOUT measurement uses LT6106 and OPA187 chips whose minimum operating voltage is 4.5V.
#define RRM_IOUT_MEASUREMENT_VSH_MIN_MV		4500

/*** RRM local structures ***/

/*******************************************************************/
typedef union {
	DINFOX_bit_representation_t renst;
} RRM_context_t;

/*** RRM global variables ***/

#ifdef RRM
const DINFOX_register_access_t NODE_REG_ACCESS[RRM_REG_ADDR_LAST] = {
	COMMON_REG_ACCESS
	DINFOX_REG_ACCESS_READ_ONLY,
	DINFOX_REG_ACCESS_READ_WRITE,
	DINFOX_REG_ACCESS_READ_ONLY,
	DINFOX_REG_ACCESS_READ_ONLY
};
#endif

/*** RRM local global variables ***/

#ifdef RRM
static RRM_context_t rrm_ctx;
#endif

/*** RRM local functions ***/

#ifdef RRM
/*******************************************************************/
static void _RRM_load_fixed_configuration(void) {
	// Local variables.
	uint32_t reg_value = 0;
	uint32_t reg_mask = 0;
	// Regulator control mode.
#ifdef RRM_REN_FORCED_HARDWARE
	DINFOX_write_field(&reg_value, &reg_mask, 0b1, RRM_REG_CONFIGURATION_0_MASK_RFH);
#else
	DINFOX_write_field(&reg_value, &reg_mask, 0b0, RRM_REG_CONFIGURATION_0_MASK_RFH);
#endif
	NODE_write_register(NODE_REQUEST_SOURCE_INTERNAL, RRM_REG_ADDR_CONFIGURATION_0, reg_mask, reg_value);
}
#endif

#ifdef RRM
/*******************************************************************/
static void _RRM_load_dynamic_configuration(void) {
	// Local variables.
	uint8_t reg_addr = 0;
	uint32_t reg_value = 0;
	// Load configuration registers from NVM.
	for (reg_addr=RRM_REG_ADDR_CONFIGURATION_1 ; reg_addr<RRM_REG_ADDR_STATUS_1 ; reg_addr++) {
		// Read NVM.
		reg_value = DINFOX_read_nvm_register(reg_addr);
		// Write register.
		NODE_write_register(NODE_REQUEST_SOURCE_INTERNAL, reg_addr, DINFOX_REG_MASK_ALL, reg_value);
	}
}
#endif

#ifdef RRM
/*******************************************************************/
static void _RRM_reset_analog_data(void) {
	// Local variables.
	uint32_t reg_analog_data_1 = 0;
	uint32_t reg_analog_data_1_mask = 0;
	uint32_t reg_analog_data_2 = 0;
	uint32_t reg_analog_data_2_mask = 0;
	// VIN / VOUT.
	DINFOX_write_field(&reg_analog_data_1, &reg_analog_data_1_mask, DINFOX_VOLTAGE_ERROR_VALUE, RRM_REG_ANALOG_DATA_1_MASK_VIN);
	DINFOX_write_field(&reg_analog_data_1, &reg_analog_data_1_mask, DINFOX_VOLTAGE_ERROR_VALUE, RRM_REG_ANALOG_DATA_1_MASK_VOUT);
	NODE_write_register(NODE_REQUEST_SOURCE_INTERNAL, RRM_REG_ADDR_ANALOG_DATA_1, reg_analog_data_1_mask, reg_analog_data_1);
	// IOUT.
	DINFOX_write_field(&reg_analog_data_2, &reg_analog_data_2_mask, DINFOX_VOLTAGE_ERROR_VALUE, RRM_REG_ANALOG_DATA_2_MASK_IOUT);
	NODE_write_register(NODE_REQUEST_SOURCE_INTERNAL, RRM_REG_ADDR_ANALOG_DATA_2, reg_analog_data_2_mask, reg_analog_data_2);
}
#endif

/*** RRM functions ***/

#ifdef RRM
/*******************************************************************/
void RRM_init_registers(void) {
#ifdef NVM_FACTORY_RESET
	// Local variables.
	uint32_t reg_value = 0;
	uint32_t reg_mask = 0;
	// IOUT offset.
	DINFOX_write_field(&reg_value, &reg_mask, 0, RRM_REG_CONFIGURATION_1_MASK_IOUT_OFFSET);
	NODE_write_register(NODE_REQUEST_SOURCE_EXTERNAL, RRM_REG_ADDR_CONFIGURATION_1, reg_mask, reg_value);
#endif
	// Read init state.
	RRM_update_register(RRM_REG_ADDR_STATUS_1);
	// Load default values.
	_RRM_load_fixed_configuration();
	_RRM_load_dynamic_configuration();
	_RRM_reset_analog_data();
}
#endif

#ifdef RRM
/*******************************************************************/
NODE_status_t RRM_update_register(uint8_t reg_addr) {
	// Local variables.
	NODE_status_t status = NODE_SUCCESS;
	uint32_t reg_value = 0;
	uint32_t reg_mask = 0;
	// Check address.
	switch (reg_addr) {
	case RRM_REG_ADDR_STATUS_1:
		// Regulator state.
#ifdef RRM_REN_FORCED_HARDWARE
		rrm_ctx.renst = DINFOX_BIT_FORCED_HARDWARE;
#else
		rrm_ctx.renst = LOAD_get_output_state();
#endif
		DINFOX_write_field(&reg_value, &reg_mask, ((uint32_t) rrm_ctx.renst), RRM_REG_STATUS_1_MASK_RENST);
		break;
	default:
		// Nothing to do for other registers.
		break;
	}
	// Write register.
	NODE_write_register(NODE_REQUEST_SOURCE_INTERNAL, reg_addr, reg_mask, reg_value);
	return status;
}
#endif

#ifdef RRM
/*******************************************************************/
NODE_status_t RRM_check_register(uint8_t reg_addr, uint32_t reg_mask) {
	// Local variables.
	NODE_status_t status = NODE_SUCCESS;
	POWER_status_t power_status = POWER_SUCCESS;
	ADC_status_t adc1_status = ADC_SUCCESS;
#ifndef RRM_REN_FORCED_HARDWARE
	LOAD_status_t load_status = LOAD_SUCCESS;
	DINFOX_bit_representation_t ren = DINFOX_BIT_ERROR;
#endif
	uint32_t reg_value = 0;
	uint32_t output_current_ua = 0;
	uint32_t reg_config_1 = 0;
	uint32_t reg_config_1_mask = 0;
	// Read register.
	status = NODE_read_register(NODE_REQUEST_SOURCE_INTERNAL, reg_addr, &reg_value);
	if (status != NODE_SUCCESS) goto errors;
	// Check address.
	switch (reg_addr) {
	case RRM_REG_ADDR_CONTROL_1:
		// REN.
		if ((reg_mask & RRM_REG_CONTROL_1_MASK_REN) != 0) {
			// Check pin mode.
#ifdef RRM_REN_FORCED_HARDWARE
			status = NODE_ERROR_FORCED_HARDWARE;
			goto errors;
#else
			// Read bit.
			ren = DINFOX_read_field(reg_value, RRM_REG_CONTROL_1_MASK_REN);
			// Compare to current state.
			if (ren != rrm_ctx.renst) {
				// Set regulator state.
				load_status = LOAD_set_output_state(ren);
				LOAD_exit_error(NODE_ERROR_BASE_LOAD);
			}
#endif
		}
		// ZCCT.
		if ((reg_mask & RRM_REG_CONTROL_1_MASK_ZCCT) != 0) {
			// Read bit.
			if (DINFOX_read_field(reg_value, RRM_REG_CONTROL_1_MASK_ZCCT) != 0) {
				// Clear request.
				NODE_write_register(NODE_REQUEST_SOURCE_INTERNAL, RRM_REG_ADDR_CONTROL_1, RRM_REG_CONTROL_1_MASK_ZCCT, 0);
				// Perform current measurement.
				power_status = POWER_enable(POWER_DOMAIN_ANALOG, LPTIM_DELAY_MODE_ACTIVE);
				POWER_exit_error(NODE_ERROR_BASE_POWER);
				adc1_status = ADC1_perform_measurements();
				ADC1_exit_error(NODE_ERROR_BASE_ADC1);
				power_status = POWER_disable(POWER_DOMAIN_ANALOG);
				POWER_exit_error(NODE_ERROR_BASE_POWER);
				// Get output current.
				adc1_status = ADC1_get_data(ADC_DATA_INDEX_IOUT_UA, &output_current_ua);
				ADC1_exit_error(NODE_ERROR_BASE_ADC1);
				// Write register and NVM.
				DINFOX_write_field(&reg_config_1, &reg_config_1_mask, DINFOX_convert_ua(output_current_ua), RRM_REG_CONFIGURATION_1_MASK_IOUT_OFFSET);
				NODE_write_register(NODE_REQUEST_SOURCE_EXTERNAL, RRM_REG_ADDR_CONFIGURATION_1, reg_config_1_mask, reg_config_1);
			}
		}
		break;
	default:
		// Nothing to do for other registers.
		break;
	}
errors:
	// Update status register.
	RRM_update_register(RRM_REG_ADDR_STATUS_1);
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
	uint32_t vsh_mv = 0;
	uint32_t lt6106_offset_current_ua = 0;
	uint32_t reg_config_1 = 0;
	uint32_t reg_analog_data_1 = 0;
	uint32_t reg_analog_data_1_mask = 0;
	uint32_t reg_analog_data_2 = 0;
	uint32_t reg_analog_data_2_mask = 0;
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
		// Regulator input voltage.
		adc1_status = ADC1_get_data(ADC_DATA_INDEX_VIN_MV, &adc_data);
		ADC1_exit_error(NODE_ERROR_BASE_ADC1);
		if (adc1_status == ADC_SUCCESS) {
			DINFOX_write_field(&reg_analog_data_1, &reg_analog_data_1_mask, (uint32_t) DINFOX_convert_mv(adc_data), RRM_REG_ANALOG_DATA_1_MASK_VIN);
		}
		// Regulator output voltage.
		adc1_status = ADC1_get_data(ADC_DATA_INDEX_VOUT_MV, &adc_data);
		ADC1_exit_error(NODE_ERROR_BASE_ADC1);
		if (adc1_status == ADC_SUCCESS) {
			DINFOX_write_field(&reg_analog_data_1, &reg_analog_data_1_mask, (uint32_t) DINFOX_convert_mv(adc_data), RRM_REG_ANALOG_DATA_1_MASK_VOUT);
		}
		vsh_mv = adc_data;
		// Check IOUT measurement validity.
		if (vsh_mv >= RRM_IOUT_MEASUREMENT_VSH_MIN_MV) {
			// Regulator output current.
			adc1_status = ADC1_get_data(ADC_DATA_INDEX_IOUT_UA, &adc_data);
			ADC1_exit_error(NODE_ERROR_BASE_ADC1);
			if (adc1_status == ADC_SUCCESS) {
				// Read IOUT offset.
				NODE_read_register(NODE_REQUEST_SOURCE_INTERNAL, RRM_REG_ADDR_CONFIGURATION_1, &reg_config_1);
				lt6106_offset_current_ua = DINFOX_get_ua(DINFOX_read_field(reg_config_1, RRM_REG_CONFIGURATION_1_MASK_IOUT_OFFSET));
				// Remove offset.
				adc_data = (adc_data < lt6106_offset_current_ua) ? 0 : (adc_data - lt6106_offset_current_ua);
				DINFOX_write_field(&reg_analog_data_2, &reg_analog_data_2_mask, (uint32_t) DINFOX_convert_ua(adc_data), RRM_REG_ANALOG_DATA_2_MASK_IOUT);
			}
		}
		// Write registers.
		NODE_write_register(NODE_REQUEST_SOURCE_INTERNAL, RRM_REG_ADDR_ANALOG_DATA_1, reg_analog_data_1_mask, reg_analog_data_1);
		NODE_write_register(NODE_REQUEST_SOURCE_INTERNAL, RRM_REG_ADDR_ANALOG_DATA_2, reg_analog_data_2_mask, reg_analog_data_2);
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

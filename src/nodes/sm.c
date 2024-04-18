/*
 * sm.c
 *
 *  Created on: 10 jun. 2023
 *      Author: Ludo
 */

#include "sm.h"

#include "adc.h"
#include "i2c.h"
#include "digital.h"
#include "dinfox.h"
#include "error.h"
#include "load.h"
#include "mode.h"
#include "node.h"
#include "sht3x.h"
#include "sm_reg.h"

/*** SM local functions ***/

#ifdef SM
/*******************************************************************/
static void _SM_reset_analog_data(void) {
	// Local variables.
	uint32_t reg_analog_data_1 = 0;
	uint32_t reg_analog_data_1_mask = 0;
	uint32_t reg_analog_data_2 = 0;
	uint32_t reg_analog_data_2_mask = 0;
	uint32_t reg_analog_data_3 = 0;
	uint32_t reg_analog_data_3_mask = 0;
	// Reset fields to error value.
	// AIN0 / AIN1.
	DINFOX_write_field(&reg_analog_data_1, &reg_analog_data_1_mask, DINFOX_VOLTAGE_ERROR_VALUE, SM_REG_ANALOG_DATA_1_MASK_VAIN0);
	DINFOX_write_field(&reg_analog_data_1, &reg_analog_data_1_mask, DINFOX_VOLTAGE_ERROR_VALUE, SM_REG_ANALOG_DATA_1_MASK_VAIN1);
	NODE_write_register(NODE_REQUEST_SOURCE_INTERNAL, SM_REG_ADDR_ANALOG_DATA_1, reg_analog_data_1_mask, reg_analog_data_1);
	// AIN2 / AIN3.
	DINFOX_write_field(&reg_analog_data_2, &reg_analog_data_2_mask, DINFOX_VOLTAGE_ERROR_VALUE, SM_REG_ANALOG_DATA_2_MASK_VAIN2);
	DINFOX_write_field(&reg_analog_data_2, &reg_analog_data_2_mask, DINFOX_VOLTAGE_ERROR_VALUE, SM_REG_ANALOG_DATA_2_MASK_VAIN3);
	NODE_write_register(NODE_REQUEST_SOURCE_INTERNAL, SM_REG_ADDR_ANALOG_DATA_2, reg_analog_data_2_mask, reg_analog_data_2);
	// TAMB / HAMB.
	DINFOX_write_field(&reg_analog_data_3, &reg_analog_data_3_mask, DINFOX_TEMPERATURE_ERROR_VALUE, SM_REG_ANALOG_DATA_3_MASK_TAMB);
	DINFOX_write_field(&reg_analog_data_3, &reg_analog_data_3_mask, DINFOX_HUMIDITY_ERROR_VALUE, SM_REG_ANALOG_DATA_3_MASK_HAMB);
	NODE_write_register(NODE_REQUEST_SOURCE_INTERNAL, SM_REG_ADDR_ANALOG_DATA_3, reg_analog_data_3_mask, reg_analog_data_3);
}
#endif

#ifdef SM
/*******************************************************************/
static void _SM_reset_digital_data(void) {
	// Local variables.
	uint32_t reg_digital_data = 0;
	uint32_t reg_digital_data_mask = 0;
	// Reset fields to error value.
	DINFOX_write_field(&reg_digital_data, &reg_digital_data_mask, DINFOX_BIT_ERROR, SM_REG_DIGITAL_DATA_MASK_DIO0);
	DINFOX_write_field(&reg_digital_data, &reg_digital_data_mask, DINFOX_BIT_ERROR, SM_REG_DIGITAL_DATA_MASK_DIO1);
	DINFOX_write_field(&reg_digital_data, &reg_digital_data_mask, DINFOX_BIT_ERROR, SM_REG_DIGITAL_DATA_MASK_DIO2);
	DINFOX_write_field(&reg_digital_data, &reg_digital_data_mask, DINFOX_BIT_ERROR, SM_REG_DIGITAL_DATA_MASK_DIO3);
	// Write register.
	NODE_write_register(NODE_REQUEST_SOURCE_INTERNAL, SM_REG_ADDR_DIGITAL_DATA, reg_digital_data_mask, reg_digital_data);
}
#endif

/*** SM functions ***/

#ifdef SM
/*******************************************************************/
void SM_init_registers(void) {
	// Read init state.
	SM_update_register(SM_REG_ADDR_CONFIGURATION_0);
	SM_update_register(SM_REG_ADDR_CONFIGURATION_1);
	SM_update_register(SM_REG_ADDR_CONFIGURATION_2);
	// Load default values.
	_SM_reset_analog_data();
	_SM_reset_digital_data();
}
#endif

#ifdef SM
/*******************************************************************/
NODE_status_t SM_update_register(uint8_t reg_addr) {
	// Local variables.
	NODE_status_t status = NODE_SUCCESS;
	uint32_t reg_value = 0;
	uint32_t reg_mask = 0;
	// Check address.
	switch (reg_addr) {
	case SM_REG_ADDR_CONFIGURATION_0:
#ifdef SM_AIN_ENABLE
		// Analog inputs enable flag.
		DINFOX_write_field(&reg_value, &reg_mask, 0b1, SM_REG_CONFIGURATION_0_MASK_AINF);
#else
		DINFOX_write_field(&reg_value, &reg_mask, 0b0, SM_REG_CONFIGURATION_0_MASK_AINF);
#endif
		// Digital inputs enable flag.
#ifdef SM_DIO_ENABLE
		DINFOX_write_field(&reg_value, &reg_mask, 0b1, SM_REG_CONFIGURATION_0_MASK_DIOF);
#else
		DINFOX_write_field(&reg_value, &reg_mask, 0b0, SM_REG_CONFIGURATION_0_MASK_DIOF);
#endif
		// Digital sensors enable flag.
#ifdef SM_DIGITAL_SENSORS_ENABLE
		DINFOX_write_field(&reg_value, &reg_mask, 0b1, SM_REG_CONFIGURATION_0_MASK_DIGF);
#else
		DINFOX_write_field(&reg_value, &reg_mask, 0b0, SM_REG_CONFIGURATION_0_MASK_DIGF);
#endif
		break;
#ifdef SM_AIN_ENABLE
	case SM_REG_ADDR_CONFIGURATION_1:
		// Analog inputs type and gain.
		DINFOX_write_field(&reg_value, &reg_mask, ((SM_AIN0_CONVERSION_TYPE == ADC_CONVERSION_TYPE_VOLTAGE_AMPLIFICATION) ? 0b1 : 0b0), SM_REG_CONFIGURATION_1_MASK_AI0T);
		DINFOX_write_field(&reg_value, &reg_mask, SM_AIN0_GAIN, SM_REG_CONFIGURATION_1_MASK_AI0G);
		DINFOX_write_field(&reg_value, &reg_mask, ((SM_AIN1_CONVERSION_TYPE == ADC_CONVERSION_TYPE_VOLTAGE_AMPLIFICATION) ? 0b1 : 0b0), SM_REG_CONFIGURATION_1_MASK_AI1T);
		DINFOX_write_field(&reg_value, &reg_mask, SM_AIN1_GAIN, SM_REG_CONFIGURATION_1_MASK_AI1G);
		break;
	case SM_REG_ADDR_CONFIGURATION_2:
		// Analog inputs type and gain.
		DINFOX_write_field(&reg_value, &reg_mask, ((SM_AIN2_CONVERSION_TYPE == ADC_CONVERSION_TYPE_VOLTAGE_AMPLIFICATION) ? 0b1 : 0b0), SM_REG_CONFIGURATION_2_MASK_AI2T);
		DINFOX_write_field(&reg_value, &reg_mask, SM_AIN2_GAIN, SM_REG_CONFIGURATION_2_MASK_AI2G);
		DINFOX_write_field(&reg_value, &reg_mask, ((SM_AIN2_CONVERSION_TYPE == ADC_CONVERSION_TYPE_VOLTAGE_AMPLIFICATION) ? 0b1 : 0b0), SM_REG_CONFIGURATION_2_MASK_AI3T);
		DINFOX_write_field(&reg_value, &reg_mask, SM_AIN2_GAIN, SM_REG_CONFIGURATION_2_MASK_AI3G);
		break;
#endif
	default:
		// Nothing to do for other registers.
		break;
	}
	NODE_write_register(NODE_REQUEST_SOURCE_INTERNAL, reg_addr, reg_mask, reg_value);
	return status;
}
#endif

#ifdef SM
/*******************************************************************/
NODE_status_t SM_check_register(uint8_t reg_addr, uint32_t reg_mask) {
	// Local variables.
	NODE_status_t status = NODE_SUCCESS;
	// None control bit in SM registers.
	UNUSED(reg_addr);
	UNUSED(reg_mask);
	return status;
}
#endif

#ifdef SM
/*******************************************************************/
NODE_status_t SM_mtrg_callback(ADC_status_t* adc_status) {
	// Local variables.
	NODE_status_t status = NODE_SUCCESS;
	POWER_status_t power_status = POWER_SUCCESS;
	ADC_status_t adc1_status = ADC_SUCCESS;
#ifdef SM_AIN_ENABLE
	uint32_t adc_data = 0;
	uint32_t reg_analog_data_1 = 0;
	uint32_t reg_analog_data_1_mask = 0;
	uint32_t reg_analog_data_2 = 0;
	uint32_t reg_analog_data_2_mask = 0;
#endif
#ifdef SM_DIO_ENABLE
	DIGITAL_status_t digital_status = DIGITAL_SUCCESS;
	uint8_t state = 0;
	uint32_t reg_digital_data = 0;
	uint32_t reg_digital_data_mask = 0;
#endif
#ifdef SM_DIGITAL_SENSORS_ENABLE
	SHT3X_status_t sht3x_status = SHT3X_SUCCESS;
	int8_t tamb_degrees = 0;
	uint8_t hamb_percent = 0;
	uint32_t reg_analog_data_3 = 0;
	uint32_t reg_analog_data_3_mask = 0;
#endif
	// Reset results.
	_SM_reset_analog_data();
	_SM_reset_digital_data();
#ifdef SM_AIN_ENABLE
	// Perform analog measurements.
	power_status = POWER_enable(POWER_DOMAIN_ANALOG, LPTIM_DELAY_MODE_ACTIVE);
	POWER_exit_error(NODE_ERROR_BASE_POWER);
	adc1_status = ADC1_perform_measurements();
	ADC1_exit_error(NODE_ERROR_BASE_ADC1);
	power_status = POWER_disable(POWER_DOMAIN_ANALOG);
	POWER_exit_error(NODE_ERROR_BASE_POWER);
	// Check status.
	if (adc1_status == ADC_SUCCESS) {
		// AIN0.
		adc1_status = ADC1_get_data(ADC_DATA_INDEX_AIN0_MV, &adc_data);
		ADC1_exit_error(NODE_ERROR_BASE_ADC1);
		if (adc1_status == ADC_SUCCESS) {
			DINFOX_write_field(&reg_analog_data_1, &reg_analog_data_1_mask, (uint32_t) DINFOX_convert_mv(adc_data), SM_REG_ANALOG_DATA_1_MASK_VAIN0);
		}
		// AIN0.
		adc1_status = ADC1_get_data(ADC_DATA_INDEX_AIN1_MV, &adc_data);
		ADC1_exit_error(NODE_ERROR_BASE_ADC1);
		if (adc1_status == ADC_SUCCESS) {
			DINFOX_write_field(&reg_analog_data_1, &reg_analog_data_1_mask, (uint32_t) DINFOX_convert_mv(adc_data), SM_REG_ANALOG_DATA_1_MASK_VAIN1);
		}
		NODE_write_register(NODE_REQUEST_SOURCE_INTERNAL, SM_REG_ADDR_ANALOG_DATA_1, reg_analog_data_1_mask, reg_analog_data_1);
		// AIN2.
		adc1_status = ADC1_get_data(ADC_DATA_INDEX_AIN2_MV, &adc_data);
		ADC1_exit_error(NODE_ERROR_BASE_ADC1);
		if (adc1_status == ADC_SUCCESS) {
			DINFOX_write_field(&reg_analog_data_2, &reg_analog_data_2_mask, (uint32_t) DINFOX_convert_mv(adc_data), SM_REG_ANALOG_DATA_2_MASK_VAIN2);
		}
		// AIN3.
		adc1_status = ADC1_get_data(ADC_DATA_INDEX_AIN3_MV, &adc_data);
		ADC1_exit_error(NODE_ERROR_BASE_ADC1);
		if (adc1_status == ADC_SUCCESS) {
			DINFOX_write_field(&reg_analog_data_2, &reg_analog_data_2_mask, (uint32_t) DINFOX_convert_mv(adc_data), SM_REG_ANALOG_DATA_2_MASK_VAIN3);
		}
		// Write register.
		NODE_write_register(NODE_REQUEST_SOURCE_INTERNAL, SM_REG_ADDR_ANALOG_DATA_2, reg_analog_data_2_mask, reg_analog_data_2);
	}
#endif
#ifdef SM_DIO_ENABLE
	// Perform digital measurements.
	power_status = POWER_enable(POWER_DOMAIN_DIGITAL, LPTIM_DELAY_MODE_SLEEP);
	POWER_exit_error(NODE_ERROR_BASE_POWER);
	DIGITAL_perform_measurements();
	power_status = POWER_disable(POWER_DOMAIN_DIGITAL);
	POWER_exit_error(NODE_ERROR_BASE_POWER);
	if (digital_status == DIGITAL_SUCCESS) {
		// DIO0.
		digital_status = DIGITAL_read(DIGITAL_DATA_INDEX_DIO0, &state);
		DIGITAL_exit_error(NODE_ERROR_BASE_DIGITAL);
		if (digital_status == DIGITAL_SUCCESS) {
			DINFOX_write_field(&reg_digital_data, &reg_digital_data_mask, (uint32_t) state, SM_REG_DIGITAL_DATA_MASK_DIO0);
		}
		// DIO1.
		digital_status = DIGITAL_read(DIGITAL_DATA_INDEX_DIO1, &state);
		DIGITAL_exit_error(NODE_ERROR_BASE_DIGITAL);
		if (digital_status == DIGITAL_SUCCESS) {
			DINFOX_write_field(&reg_digital_data, &reg_digital_data_mask, (uint32_t) state, SM_REG_DIGITAL_DATA_MASK_DIO1);
		}
		// DIO2.
		digital_status = DIGITAL_read(DIGITAL_DATA_INDEX_DIO2, &state);
		DIGITAL_exit_error(NODE_ERROR_BASE_DIGITAL);
		if (digital_status == DIGITAL_SUCCESS) {
			DINFOX_write_field(&reg_digital_data, &reg_digital_data_mask, (uint32_t) state, SM_REG_DIGITAL_DATA_MASK_DIO2);
		}
		// DIO3.
		digital_status = DIGITAL_read(DIGITAL_DATA_INDEX_DIO3, &state);
		DIGITAL_exit_error(NODE_ERROR_BASE_DIGITAL);
		if (digital_status == DIGITAL_SUCCESS) {
			DINFOX_write_field(&reg_digital_data, &reg_digital_data_mask, (uint32_t) state, SM_REG_DIGITAL_DATA_MASK_DIO3);
		}
		// Write register.
		NODE_write_register(NODE_REQUEST_SOURCE_INTERNAL, SM_REG_ADDR_DIGITAL_DATA, reg_digital_data_mask, reg_digital_data);
	}
#endif
#ifdef SM_DIGITAL_SENSORS_ENABLE
	// Perform temperature / humidity measurements.
	power_status = POWER_enable(POWER_DOMAIN_SENSORS, LPTIM_DELAY_MODE_STOP);
	POWER_exit_error(NODE_ERROR_BASE_POWER);
	sht3x_status = SHT3X_perform_measurements(SHT3X_I2C_ADDRESS);
	SHT3X_exit_error(NODE_ERROR_BASE_SHT3X);
	power_status = POWER_disable(POWER_DOMAIN_SENSORS);
	POWER_exit_error(NODE_ERROR_BASE_POWER);
	if (sht3x_status == SHT3X_SUCCESS) {
		// TAMB.
		sht3x_status = SHT3X_get_temperature(&tamb_degrees);
		SHT3X_exit_error(NODE_ERROR_BASE_SHT3X);
		if (sht3x_status == SHT3X_SUCCESS) {
			DINFOX_write_field(&reg_analog_data_3, &reg_analog_data_3_mask, (uint32_t) DINFOX_convert_degrees(tamb_degrees), SM_REG_ANALOG_DATA_3_MASK_TAMB);
		}
		// HAMB.
		sht3x_status = SHT3X_get_humidity(&hamb_percent);
		SHT3X_exit_error(NODE_ERROR_BASE_SHT3X);
		if (sht3x_status == SHT3X_SUCCESS) {
			DINFOX_write_field(&reg_analog_data_3, &reg_analog_data_3_mask, (uint32_t) hamb_percent, SM_REG_ANALOG_DATA_3_MASK_HAMB);
		}
		// Write register.
		NODE_write_register(NODE_REQUEST_SOURCE_INTERNAL, SM_REG_ADDR_ANALOG_DATA_3, reg_analog_data_3_mask, reg_analog_data_3);
	}
#endif
	// Update ADC status.
	if (adc_status != NULL) {
		(*adc_status) = adc1_status;
	}
	return status;
errors:
#ifdef SM_AIN_ENABLE
	POWER_disable(POWER_DOMAIN_ANALOG);
#endif
#ifdef SM_DIO_ENABLE
	POWER_disable(POWER_DOMAIN_DIGITAL);
#endif
#ifdef SM_DIGITAL_SENSORS_ENABLE
	POWER_disable(POWER_DOMAIN_SENSORS);
#endif
	// Update ADC status.
	if (adc_status != NULL) {
		(*adc_status) = adc1_status;
	}
	return status;
}
#endif

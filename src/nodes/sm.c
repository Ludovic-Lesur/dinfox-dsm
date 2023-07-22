/*
 * sm.c
 *
 *  Created on: 10 jun. 2023
 *      Author: Ludo
 */

#include "bpsm.h"

#include "adc.h"
#include "i2c.h"
#include "digital.h"
#include "dinfox.h"
#include "error.h"
#include "load.h"
#include "node.h"
#include "sht3x.h"
#include "sm_reg.h"

/*** SM local functions ***/

#ifdef SM
/*******************************************************************/
static void _SM_reset_analog_data(void) {
	// Local variables.
	NODE_status_t node_status = NODE_SUCCESS;
	// Reset fields to error value.
	node_status = NODE_write_field(NODE_REQUEST_SOURCE_INTERNAL, SM_REG_ADDR_ANALOG_DATA_1, SM_REG_ANALOG_DATA_1_MASK_VAIN0, DINFOX_VOLTAGE_ERROR_VALUE);
	NODE_stack_error();
	node_status = NODE_write_field(NODE_REQUEST_SOURCE_INTERNAL, SM_REG_ADDR_ANALOG_DATA_1, SM_REG_ANALOG_DATA_1_MASK_VAIN1, DINFOX_VOLTAGE_ERROR_VALUE);
	NODE_stack_error();
	node_status = NODE_write_field(NODE_REQUEST_SOURCE_INTERNAL, SM_REG_ADDR_ANALOG_DATA_2, SM_REG_ANALOG_DATA_2_MASK_VAIN2, DINFOX_VOLTAGE_ERROR_VALUE);
	NODE_stack_error();
	node_status = NODE_write_field(NODE_REQUEST_SOURCE_INTERNAL, SM_REG_ADDR_ANALOG_DATA_2, SM_REG_ANALOG_DATA_2_MASK_VAIN3, DINFOX_VOLTAGE_ERROR_VALUE);
	NODE_stack_error();
	node_status = NODE_write_field(NODE_REQUEST_SOURCE_INTERNAL, SM_REG_ADDR_ANALOG_DATA_3, SM_REG_ANALOG_DATA_3_MASK_TAMB, DINFOX_TEMPERATURE_ERROR_VALUE);
	NODE_stack_error();
	node_status = NODE_write_field(NODE_REQUEST_SOURCE_INTERNAL, SM_REG_ADDR_ANALOG_DATA_3, SM_REG_ANALOG_DATA_3_MASK_HAMB, DINFOX_HUMIDITY_ERROR_VALUE);
	NODE_stack_error();
}
#endif

/*** SM functions ***/

#ifdef SM
/*******************************************************************/
void SM_init_registers(void) {
	// Load default values.
	_SM_reset_analog_data();
}
#endif

#ifdef SM
/*******************************************************************/
NODE_status_t SM_update_register(uint8_t reg_addr) {
	// Local variables.
	NODE_status_t status = NODE_SUCCESS;
	// Nothing to do on SM registers.
	return status;
}
#endif

#ifdef SM
/*******************************************************************/
NODE_status_t SM_check_register(uint8_t reg_addr) {
	// Local variables.
	NODE_status_t status = NODE_SUCCESS;
	// None control bit in SM registers.
	return status;
}
#endif

#ifdef SM
/*******************************************************************/
NODE_status_t SM_mtrg_callback(ADC_status_t* adc_status) {
	// Local variables.
	NODE_status_t status = NODE_SUCCESS;
	NODE_status_t node_status = NODE_SUCCESS;
#ifdef SM_AIN_ENABLE
	ADC_status_t adc1_status = ADC_SUCCESS;
	uint32_t adc_data = 0;
#endif
#ifdef SM_DIO_ENABLE
	DIGITAL_status_t digital_status = DIGITAL_SUCCESS;
	uint8_t state = 0;
#endif
#ifdef SM_DIGITAL_SENSORS_ENABLE
	I2C_status_t i2c1_status = I2C_SUCCESS;
	SHT3X_status_t sht3x_status = SHT3X_SUCCESS;
	int8_t tamb_degrees = 0;
	uint8_t hamb_percent = 0;
#endif
	// Reset results.
	_SM_reset_analog_data();
#ifdef SM_AIN_ENABLE
	// Perform analog measurements.
	adc1_status = ADC1_perform_measurements();
	ADC1_stack_error();
	// Update parameter.
	if (adc_status != NULL) {
		(*adc_status) = adc1_status;
	}
	// Check status.
	if (adc1_status == ADC_SUCCESS) {
		// AIN0.
		adc1_status = ADC1_get_data(ADC_DATA_INDEX_AIN0_MV, &adc_data);
		ADC1_stack_error();
		if (adc1_status == ADC_SUCCESS) {
			node_status = NODE_write_field(NODE_REQUEST_SOURCE_INTERNAL, SM_REG_ADDR_ANALOG_DATA_1, SM_REG_ANALOG_DATA_1_MASK_VAIN0, (uint32_t) DINFOX_convert_mv(adc_data));
			NODE_stack_error();
		}
		// AIN0.
		adc1_status = ADC1_get_data(ADC_DATA_INDEX_AIN1_MV, &adc_data);
		ADC1_stack_error();
		if (adc1_status == ADC_SUCCESS) {
			node_status = NODE_write_field(NODE_REQUEST_SOURCE_INTERNAL, SM_REG_ADDR_ANALOG_DATA_1, SM_REG_ANALOG_DATA_1_MASK_VAIN1, (uint32_t) DINFOX_convert_mv(adc_data));
			NODE_stack_error();
		}
		// AIN2.
		adc1_status = ADC1_get_data(ADC_DATA_INDEX_AIN2_MV, &adc_data);
		ADC1_stack_error();
		if (adc1_status == ADC_SUCCESS) {
			node_status = NODE_write_field(NODE_REQUEST_SOURCE_INTERNAL, SM_REG_ADDR_ANALOG_DATA_2, SM_REG_ANALOG_DATA_2_MASK_VAIN2, (uint32_t) DINFOX_convert_mv(adc_data));
			NODE_stack_error();
		}
		// AIN3.
		adc1_status = ADC1_get_data(ADC_DATA_INDEX_AIN3_MV, &adc_data);
		ADC1_stack_error();
		if (adc1_status == ADC_SUCCESS) {
			node_status = NODE_write_field(NODE_REQUEST_SOURCE_INTERNAL, SM_REG_ADDR_ANALOG_DATA_2, SM_REG_ANALOG_DATA_2_MASK_VAIN3, (uint32_t) DINFOX_convert_mv(adc_data));
			NODE_stack_error();
		}
	}
#endif
#ifdef SM_DIO_ENABLE
	// Perform digital measurements.
	digital_status = DIGITAL_perform_measurements();
	DIGITAL_stack_error();
	if (digital_status == DIGITAL_SUCCESS) {
		// DIO0.
		digital_status = DIGITAL_read(DIGITAL_DATA_INDEX_DIO0, &state);
		DIGITAL_stack_error();
		if (digital_status == DIGITAL_SUCCESS) {
			node_status = NODE_write_field(NODE_REQUEST_SOURCE_INTERNAL, SM_REG_ADDR_DIGITAL_DATA, SM_REG_DIGITAL_DATA_MASK_DIO0, (uint32_t) state);
			NODE_stack_error();
		}
		// DIO1.
		digital_status = DIGITAL_read(DIGITAL_DATA_INDEX_DIO1, &state);
		DIGITAL_stack_error();
		if (digital_status == DIGITAL_SUCCESS) {
			node_status = NODE_write_field(NODE_REQUEST_SOURCE_INTERNAL, SM_REG_ADDR_DIGITAL_DATA, SM_REG_DIGITAL_DATA_MASK_DIO1, (uint32_t) state);
			NODE_stack_error();
		}
		// DIO2.
		digital_status = DIGITAL_read(DIGITAL_DATA_INDEX_DIO2, &state);
		DIGITAL_stack_error();
		if (digital_status == DIGITAL_SUCCESS) {
			node_status = NODE_write_field(NODE_REQUEST_SOURCE_INTERNAL, SM_REG_ADDR_DIGITAL_DATA, SM_REG_DIGITAL_DATA_MASK_DIO2, (uint32_t) state);
			NODE_stack_error();
		}
		// DIO3.
		digital_status = DIGITAL_read(DIGITAL_DATA_INDEX_DIO3, &state);
		DIGITAL_stack_error();
		if (digital_status == DIGITAL_SUCCESS) {
			node_status = NODE_write_field(NODE_REQUEST_SOURCE_INTERNAL, SM_REG_ADDR_DIGITAL_DATA, SM_REG_DIGITAL_DATA_MASK_DIO3, (uint32_t) state);
			NODE_stack_error();
		}
	}
#endif
#ifdef SM_DIGITAL_SENSORS_ENABLE
	// Perform temperature / humidity measurements.
	i2c1_status = I2C1_power_on();
	I2C1_stack_error();
	sht3x_status = SHT3X_perform_measurements(SHT3X_I2C_ADDRESS);
	SHT3X_stack_error();
	I2C1_power_off();
	if (sht3x_status == SHT3X_SUCCESS) {
		// Tamb.
		sht3x_status = SHT3X_get_temperature(&tamb_degrees);
		SHT3X_stack_error();
		if (sht3x_status == SHT3X_SUCCESS) {
			node_status = NODE_write_field(NODE_REQUEST_SOURCE_INTERNAL, SM_REG_ADDR_ANALOG_DATA_3, SM_REG_ANALOG_DATA_3_MASK_TAMB, (uint32_t) DINFOX_convert_degrees(tamb_degrees));
			NODE_stack_error();
		}
		// Tamb.
		sht3x_status = SHT3X_get_humidity(&hamb_percent);
		SHT3X_stack_error();
		if (sht3x_status == SHT3X_SUCCESS) {
			node_status = NODE_write_field(NODE_REQUEST_SOURCE_INTERNAL, SM_REG_ADDR_ANALOG_DATA_3, SM_REG_ANALOG_DATA_3_MASK_HAMB, (uint32_t) hamb_percent);
			NODE_stack_error();
		}
	}
#endif
	return status;
}
#endif

/*
 * adc.c
 *
 *  Created on: 18 apr. 2020
 *      Author: Ludo
 */

#include "adc.h"

#include "adc_reg.h"
#include "gpio.h"
#include "lptim.h"
#include "mapping.h"
#include "math.h"
#include "mode.h"
#include "rcc_reg.h"
#include "types.h"

/*** ADC local macros ***/

#define ADC_MEDIAN_FILTER_LENGTH			9
#define ADC_CENTER_AVERAGE_LENGTH			3

#define ADC_FULL_SCALE_12BITS				4095

#define ADC_VREFINT_VOLTAGE_MV				((VREFINT_CAL * VREFINT_VCC_CALIB_MV) / (ADC_FULL_SCALE_12BITS))
#define ADC_VMCU_DEFAULT_MV					3000

#define ADC_VOLTAGE_DIVIDER_RATIO_VIN		10
#define ADC_VOLTAGE_DIVIDER_RATIO_VSRC		10
#define ADC_VOLTAGE_DIVIDER_RATIO_VSTR		10
#define ADC_VOLTAGE_DIVIDER_RATIO_VCOM		10
#define ADC_VOLTAGE_DIVIDER_RATIO_VOUT		10
#define ADC_VOLTAGE_DIVIDER_RATIO_VBKP		10

#define ADC_LT6106_VOLTAGE_GAIN				59
#define ADC_LT6106_SHUNT_RESISTOR_MOHMS		10
#define ADC_LT6106_OFFSET_CURRENT_UA		25000 // 250µV MAXIMUM / 10mR = 25mA.

#define ADC_TIMEOUT_COUNT					1000000

/*** ADC local structures ***/

typedef enum {
#ifdef LVRM
	ADC_CHANNEL_IOUT = 0,
	ADC_CHANNEL_VOUT = 4,
	ADC_CHANNEL_VCOM = 6,
#endif
#ifdef BPSM
	ADC_CHANNEL_VBKP = 0,
	ADC_CHANNEL_VSTR = 4,
	ADC_CHANNEL_VSRC = 6,
#endif
#ifdef DDRM
	ADC_CHANNEL_IOUT = 0,
	ADC_CHANNEL_VOUT = 4,
	ADC_CHANNEL_VIN = 7,
#endif
#ifdef RRM
	ADC_CHANNEL_IOUT = 4,
	ADC_CHANNEL_VOUT = 6,
	ADC_CHANNEL_VIN = 7,
#endif
	ADC_CHANNEL_VREFINT = 17,
	ADC_CHANNEL_TMCU = 18,
	ADC_CHANNEL_LAST = 19
} ADC_channel_t;

typedef struct {
	uint32_t vrefint_12bits;
	uint32_t data[ADC_DATA_INDEX_LAST];
	int8_t tmcu_degrees;
} ADC_context_t;

/*** ADC local global variables ***/

static ADC_context_t adc_ctx;

/*** ADC local functions ***/

/* PERFORM A SINGLE ADC CONVERSION.
 * @param adc_channel:			Channel to convert.
 * @param adc_result_12bits:	Pointer to 32-bits value that will contain ADC raw result on 12 bits.
 * @return status:				Function execution status.
 */
static ADC_status_t _ADC1_single_conversion(ADC_channel_t adc_channel, uint32_t* adc_result_12bits) {
	// Local variables.
	ADC_status_t status = ADC_SUCCESS;
	uint32_t loop_count = 0;
	// Check parameters.
	if (adc_channel >= ADC_CHANNEL_LAST) {
		status = ADC_ERROR_CHANNEL;
		goto errors;
	}
	if (adc_result_12bits == NULL) {
		status = ADC_ERROR_NULL_PARAMETER;
		goto errors;
	}
	// Select input channel.
	ADC1 -> CHSELR &= 0xFFF80000; // Reset all bits.
	ADC1 -> CHSELR |= (0b1 << adc_channel);
	// Clear all flags.
	ADC1 -> ISR |= 0x0000089F;
	// Read raw supply voltage.
	ADC1 -> CR |= (0b1 << 2); // ADSTART='1'.
	while (((ADC1 -> ISR) & (0b1 << 2)) == 0) {
		// Wait end of conversion ('EOC='1') or timeout.
		loop_count++;
		if (loop_count > ADC_TIMEOUT_COUNT) {
			status = ADC_ERROR_TIMEOUT;
			goto errors;
		}
	}
	(*adc_result_12bits) = (ADC1 -> DR);
errors:
	return status;
}

/* PERFORM SEVERAL CONVERSIONS FOLLOWED BY A MEDIAN FILTER.
 * @param adc_channel:			Channel to convert.
 * @param adc_result_12bits:	Pointer to 32-bits value that will contain ADC filtered result on 12 bits.
 * @return status:				Function execution status.
 */
static ADC_status_t _ADC1_filtered_conversion(ADC_channel_t adc_channel, uint32_t* adc_result_12bits) {
	// Local variables.
	ADC_status_t status = ADC_SUCCESS;
	MATH_status_t math_status = MATH_SUCCESS;
	uint32_t adc_sample_buf[ADC_MEDIAN_FILTER_LENGTH] = {0x00};
	uint8_t idx = 0;
	// Check parameters.
	if (adc_channel >= ADC_CHANNEL_LAST) {
		status = ADC_ERROR_CHANNEL;
		goto errors;
	}
	if (adc_result_12bits == NULL) {
		status = ADC_ERROR_NULL_PARAMETER;
		goto errors;
	}
	// Perform all conversions.
	for (idx=0 ; idx<ADC_MEDIAN_FILTER_LENGTH ; idx++) {
		status = _ADC1_single_conversion(adc_channel, &(adc_sample_buf[idx]));
		if (status != ADC_SUCCESS) goto errors;
	}
	// Apply median filter.
	math_status = MATH_median_filter_u32(adc_sample_buf, ADC_MEDIAN_FILTER_LENGTH, ADC_CENTER_AVERAGE_LENGTH, adc_result_12bits);
	MATH_status_check(ADC_ERROR_BASE_MATH);
errors:
	return status;
}

/* PERFORM INTERNAL REFERENCE VOLTAGE CONVERSION.
 * @param:			None.
 * @return status:	Function execution status.
 */
static ADC_status_t _ADC1_compute_vrefint(void) {
	// Local variables.
	ADC_status_t status = ADC_SUCCESS;
	// Read raw reference voltage.
	status = _ADC1_filtered_conversion(ADC_CHANNEL_VREFINT, &adc_ctx.vrefint_12bits);
	return status;
}

/* COMPUTE MCU SUPPLY VOLTAGE.
 * @param:			None.
 * @return status:	Function execution status.
 */
static void _ADC1_compute_vmcu(void) {
	// Retrieve supply voltage from bandgap result.
	adc_ctx.data[ADC_DATA_INDEX_VMCU_MV] = (VREFINT_CAL * VREFINT_VCC_CALIB_MV) / (adc_ctx.vrefint_12bits);
}

/* COMPUTE MCU TEMPERATURE THANKS TO INTERNAL VOLTAGE REFERENCE.
 * @param:			None.
 * @return status:	Function execution status.
 */
static ADC_status_t _ADC1_compute_tmcu(void) {
	// Local variables.
	ADC_status_t status = ADC_SUCCESS;
	uint32_t raw_temp_sensor_12bits = 0;
	int32_t raw_temp_calib_mv = 0;
	int32_t temp_calib_degrees = 0;
	// Read raw temperature.
	status = _ADC1_filtered_conversion(ADC_CHANNEL_TMCU, &raw_temp_sensor_12bits);
	if (status != ADC_SUCCESS) goto errors;
	// Compute temperature according to MCU factory calibration (see p.301 and p.847 of RM0377 datasheet).
	raw_temp_calib_mv = ((int32_t) raw_temp_sensor_12bits * adc_ctx.data[ADC_DATA_INDEX_VMCU_MV]) / (TS_VCC_CALIB_MV) - TS_CAL1; // Equivalent raw measure for calibration power supply (VCC_CALIB).
	temp_calib_degrees = raw_temp_calib_mv * ((int32_t) (TS_CAL2_TEMP-TS_CAL1_TEMP));
	temp_calib_degrees = (temp_calib_degrees) / ((int32_t) (TS_CAL2 - TS_CAL1));
	adc_ctx.tmcu_degrees = temp_calib_degrees + TS_CAL1_TEMP;
errors:
	return status;
}

#ifdef LVRM
/* COMPUTE COMMON RELAY VOLTAGE.
 * @param:			None.
 * @return status:	Function execution status.
 */
static ADC_status_t _ADC1_compute_vcom(void) {
	// Local variables.
	ADC_status_t status = ADC_SUCCESS;
	uint32_t vin_12bits = 0;
	// Get raw result.
	status = _ADC1_filtered_conversion(ADC_CHANNEL_VCOM, &vin_12bits);
	if (status != ADC_SUCCESS) goto errors;
	// Convert to mV using VREFINT.
	adc_ctx.data[ADC_DATA_INDEX_VCOM_MV] = (ADC_VREFINT_VOLTAGE_MV * vin_12bits * ADC_VOLTAGE_DIVIDER_RATIO_VCOM) / (adc_ctx.vrefint_12bits);
errors:
	return status;
}
#endif

#ifdef BPSM
/* COMPUTE SOURCE VOLTAGE.
 * @param:			None.
 * @return status:	Function execution status.
 */
static ADC_status_t _ADC1_compute_vsrc(void) {
	// Local variables.
	ADC_status_t status = ADC_SUCCESS;
	uint32_t vin_12bits = 0;
	// Get raw result.
	status = _ADC1_filtered_conversion(ADC_CHANNEL_VSRC, &vin_12bits);
	if (status != ADC_SUCCESS) goto errors;
	// Convert to mV using VREFINT.
	adc_ctx.data[ADC_DATA_INDEX_VSRC_MV] = (ADC_VREFINT_VOLTAGE_MV * vin_12bits * ADC_VOLTAGE_DIVIDER_RATIO_VSRC) / (adc_ctx.vrefint_12bits);
errors:
	return status;
}
#endif

#if (defined DDRM) || (defined RRM)
/* COMPUTE INPUT VOLTAGE.
 * @param:			None.
 * @return status:	Function execution status.
 */
static ADC_status_t _ADC1_compute_vin(void) {
	// Local variables.
	ADC_status_t status = ADC_SUCCESS;
	uint32_t vin_12bits = 0;
	// Get raw result.
	status = _ADC1_filtered_conversion(ADC_CHANNEL_VIN, &vin_12bits);
	if (status != ADC_SUCCESS) goto errors;
	// Convert to mV using VREFINT.
	adc_ctx.data[ADC_DATA_INDEX_VIN_MV] = (ADC_VREFINT_VOLTAGE_MV * vin_12bits * ADC_VOLTAGE_DIVIDER_RATIO_VIN) / (adc_ctx.vrefint_12bits);
errors:
	return status;
}
#endif

#if (defined LVRM) || (defined DDRM) || (defined RRM)
/* COMPUTE OUTPUT VOLTAGE.
 * @param:			None.
 * @return status:	Function execution status.
 */
static ADC_status_t _ADC1_compute_vout(void) {
	// Local variables.
	ADC_status_t status = ADC_SUCCESS;
	uint32_t vout_12bits = 0;
	// Get raw result.
	status = _ADC1_filtered_conversion(ADC_CHANNEL_VOUT, &vout_12bits);
	if (status != ADC_SUCCESS) goto errors;
	// Convert to mV using VREFINT.
	adc_ctx.data[ADC_DATA_INDEX_VOUT_MV] = (ADC_VREFINT_VOLTAGE_MV * vout_12bits * ADC_VOLTAGE_DIVIDER_RATIO_VOUT) / (adc_ctx.vrefint_12bits);
errors:
	return status;
}
#endif

#ifdef BPSM
/* COMPUTE STORAGE ELEMENT VOLTAGE.
 * @param:			None.
 * @return status:	Function execution status.
 */
static ADC_status_t _ADC1_compute_vstr(void) {
	// Local variables.
	ADC_status_t status = ADC_SUCCESS;
	uint32_t vout_12bits = 0;
	// Get raw result.
	status = _ADC1_filtered_conversion(ADC_CHANNEL_VSTR, &vout_12bits);
	if (status != ADC_SUCCESS) goto errors;
	// Convert to mV using VREFINT.
	adc_ctx.data[ADC_DATA_INDEX_VSTR_MV] = (ADC_VREFINT_VOLTAGE_MV * vout_12bits * ADC_VOLTAGE_DIVIDER_RATIO_VSTR) / (adc_ctx.vrefint_12bits);
errors:
	return status;
}
#endif

#ifdef BPSM
/* COMPUTE STORAGE ELEMENT VOLTAGE.
 * @param:			None.
 * @return status:	Function execution status.
 */
static ADC_status_t _ADC1_compute_vbkp(void) {
	// Local variables.
	ADC_status_t status = ADC_SUCCESS;
	uint32_t vout_12bits = 0;
	// Get raw result.
	status = _ADC1_filtered_conversion(ADC_CHANNEL_VBKP, &vout_12bits);
	if (status != ADC_SUCCESS) goto errors;
	// Convert to mV using VREFINT.
	adc_ctx.data[ADC_DATA_INDEX_VBKP_MV] = (ADC_VREFINT_VOLTAGE_MV * vout_12bits * ADC_VOLTAGE_DIVIDER_RATIO_VBKP) / (adc_ctx.vrefint_12bits);
errors:
	return status;
}
#endif

#if (defined LVRM) || (defined DDRM) || (defined RRM)
/* COMPUTE OUTPUT CURRENT.
 * @param:			None.
 * @return status:	Function execution status.
 */
static ADC_status_t _ADC1_compute_iout(void) {
	// Local variables.
	ADC_status_t status = ADC_SUCCESS;
	uint32_t iout_12bits = 0;
	uint64_t num = 0;
	uint64_t den = 0;
	// Get raw result.
	status = _ADC1_filtered_conversion(ADC_CHANNEL_IOUT, &iout_12bits);
	if (status != ADC_SUCCESS) goto errors;
	// Convert to uA using VREFINT.
	num = iout_12bits;
	num *= ADC_VREFINT_VOLTAGE_MV;
	num *= 1000000;
	den = adc_ctx.vrefint_12bits;
	den *= ADC_LT6106_VOLTAGE_GAIN;
	den *= ADC_LT6106_SHUNT_RESISTOR_MOHMS;
	adc_ctx.data[ADC_DATA_INDEX_IOUT_UA] = (num) / (den);
	// Remove offset current.
	if (adc_ctx.data[ADC_DATA_INDEX_IOUT_UA] < ADC_LT6106_OFFSET_CURRENT_UA) {
		adc_ctx.data[ADC_DATA_INDEX_IOUT_UA] = 0;
	}
	else {
		adc_ctx.data[ADC_DATA_INDEX_IOUT_UA] -= ADC_LT6106_OFFSET_CURRENT_UA;
	}
errors:
	return status;
}
#endif

/*** ADC functions ***/

/* INIT ADC1 PERIPHERAL.
 * @param:			None.
 * @return status:	Function execution status.
 */
ADC_status_t ADC1_init(void) {
	// Local variables.
	ADC_status_t status = ADC_SUCCESS;
	LPTIM_status_t lptim1_status = LPTIM_SUCCESS;
	uint8_t idx = 0;
	uint32_t loop_count = 0;
	// Init GPIOs.
#ifdef BPSM
	GPIO_configure(&GPIO_MNTR_EN, GPIO_MODE_OUTPUT, GPIO_TYPE_PUSH_PULL, GPIO_SPEED_LOW, GPIO_PULL_NONE);
#endif
#if (defined LVRM) || (defined BPSM) || (defined DDRM)
	GPIO_configure(&GPIO_ADC1_IN0, GPIO_MODE_ANALOG, GPIO_TYPE_OPEN_DRAIN, GPIO_SPEED_LOW, GPIO_PULL_NONE);
#endif
	GPIO_configure(&GPIO_ADC1_IN4, GPIO_MODE_ANALOG, GPIO_TYPE_OPEN_DRAIN, GPIO_SPEED_LOW, GPIO_PULL_NONE);
#if (defined LVRM) || (defined BPSM)
	GPIO_configure(&GPIO_ADC1_IN6, GPIO_MODE_ANALOG, GPIO_TYPE_OPEN_DRAIN, GPIO_SPEED_LOW, GPIO_PULL_NONE);
#endif
#if (defined DDRM) || (defined RRM)
	GPIO_configure(&GPIO_ADC1_IN7, GPIO_MODE_ANALOG, GPIO_TYPE_OPEN_DRAIN, GPIO_SPEED_LOW, GPIO_PULL_NONE);
#endif
	// Init context.
	adc_ctx.vrefint_12bits = 0;
	for (idx=0 ; idx<ADC_DATA_INDEX_LAST ; idx++) adc_ctx.data[idx] = 0;
	adc_ctx.data[ADC_DATA_INDEX_VMCU_MV] = ADC_VMCU_DEFAULT_MV;
	// Enable peripheral clock.
	RCC -> APB2ENR |= (0b1 << 9); // ADCEN='1'.
	// Ensure ADC is disabled.
	if (((ADC1 -> CR) & (0b1 << 0)) != 0) {
		ADC1 -> CR |= (0b1 << 1); // ADDIS='1'.
	}
	// Enable ADC voltage regulator.
	ADC1 -> CR |= (0b1 << 28);
	lptim1_status = LPTIM1_delay_milliseconds(10, 0);
	LPTIM1_status_check(ADC_ERROR_BASE_LPTIM);
	// ADC configuration.
	ADC1 -> CCR |= (0b1 << 25); // Enable low frequency clock (LFMEN='1').
	ADC1 -> CFGR2 |= (0b11 << 30); // Use PCLK2 as ADCCLK (MSI).
	ADC1 -> SMPR |= (0b111 << 0); // Maximum sampling time.
	// ADC calibration.
	ADC1 -> CR |= (0b1 << 31); // ADCAL='1'.
	while ((((ADC1 -> CR) & (0b1 << 31)) != 0) && (((ADC1 -> ISR) & (0b1 << 11)) == 0)) {
		// Wait until calibration is done or timeout.
		loop_count++;
		if (loop_count > ADC_TIMEOUT_COUNT) {
			status = ADC_ERROR_CALIBRATION;
			goto errors;
		}
	}
errors:
	return status;
}

/* PERFORM INTERNAL ADC MEASUREMENTS.
 * @param:			None.
 * @return status:	Function execution status.
 */
ADC_status_t ADC1_perform_measurements(void) {
	// Local variables.
	ADC_status_t status = ADC_SUCCESS;
	LPTIM_status_t lptim1_status = LPTIM_SUCCESS;
	uint32_t loop_count = 0;
	// Enable ADC peripheral.
	ADC1 -> CR |= (0b1 << 0); // ADEN='1'.
	while (((ADC1 -> ISR) & (0b1 << 0)) == 0) {
		// Wait for ADC to be ready (ADRDY='1') or timeout.
		loop_count++;
		if (loop_count > ADC_TIMEOUT_COUNT) {
			status = ADC_ERROR_TIMEOUT;
			goto errors;
		}
	}
#ifdef BPSM
	// Enable voltage dividers.
	GPIO_write(&GPIO_MNTR_EN, 1);
#endif
	// Wake-up VREFINT and temperature sensor.
	ADC1 -> CCR |= (0b11 << 22); // TSEN='1' and VREFEF='1'.
	lptim1_status = LPTIM1_delay_milliseconds(10, 0);
	LPTIM1_status_check(ADC_ERROR_BASE_LPTIM);
	// Perform measurements.
	status = _ADC1_compute_vrefint();
	if (status != ADC_SUCCESS) goto errors;
	// Input voltage.
#ifdef LVRM
	status = _ADC1_compute_vcom();
	if (status != ADC_SUCCESS) goto errors;
#endif
#if (defined DDRM) || (defined RRM)
	status = _ADC1_compute_vin();
	if (status != ADC_SUCCESS) goto errors;
#endif
#ifdef BPSM
	status = _ADC1_compute_vsrc();
	if (status != ADC_SUCCESS) goto errors;
#endif
	// Storage element voltage.
#ifdef BPSM
	status = _ADC1_compute_vstr();
	if (status != ADC_SUCCESS) goto errors;
#endif
	// Output voltage
#if (defined LVRM) || (defined DDRM) || (defined RRM)
	status = _ADC1_compute_vout();
	if (status != ADC_SUCCESS) goto errors;
	status = _ADC1_compute_iout();
	if (status != ADC_SUCCESS) goto errors;
#endif
#ifdef BPSM
	status = _ADC1_compute_vbkp();
	if (status != ADC_SUCCESS) goto errors;
#endif
	status = _ADC1_compute_tmcu();
	if (status != ADC_SUCCESS) goto errors;
	_ADC1_compute_vmcu();
errors:
	// Switch internal voltage reference off.
	ADC1 -> CCR &= ~(0b11 << 22); // TSEN='0' and VREFEF='0'.
#ifdef BPSM
	// Disable voltage dividers.
	GPIO_write(&GPIO_MNTR_EN, 0);
#endif
	// Disable ADC peripheral.
	ADC1 -> CR |= (0b1 << 1); // ADDIS='1'.
	return status;
}

/* GET ADC DATA.
 * @param data_idx:	Index of the data to retrieve.
 * @param data:		Pointer that will contain ADC data.
 * @return status:	Function execution status.
 */
ADC_status_t ADC1_get_data(ADC_data_index_t data_idx, uint32_t* data) {
	// Local variables.
	ADC_status_t status = ADC_SUCCESS;
	// Check parameters.
	if (data_idx >= ADC_DATA_INDEX_LAST) {
		status = ADC_ERROR_DATA_INDEX;
		goto errors;
	}
	if (data == NULL) {
		status = ADC_ERROR_NULL_PARAMETER;
		goto errors;
	}
	(*data) = adc_ctx.data[data_idx];
errors:
	return status;
}

/* GET MCU TEMPERATURE.
 * @param tmcu_degrees:	Pointer to 8-bits value that will contain MCU temperature in degrees (2-complement).
 * @return status:		Function execution status.
 */
ADC_status_t ADC1_get_tmcu(int8_t* tmcu_degrees) {
	// Local variables.
	ADC_status_t status = ADC_SUCCESS;
	// Check parameter.
	if (tmcu_degrees == NULL) {
		status = ADC_ERROR_NULL_PARAMETER;
		goto errors;
	}
	(*tmcu_degrees) = adc_ctx.tmcu_degrees;
errors:
	return status;
}

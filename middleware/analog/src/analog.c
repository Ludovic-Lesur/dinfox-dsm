/*
 * analog.c
 *
 *  Created on: 15 dec. 2024
 *      Author: Ludo
 */

#include "analog.h"

#include "adc.h"
#include "dsm_flags.h"
#include "dsm_flags_slave.h"
#include "error.h"
#include "error_base.h"
#include "mcu_mapping.h"
#include "types.h"

/*** ANALOG local macros ***/

#ifdef BCM
#define ANALOG_DIVIDER_RATIO_SOURCE_VOLTAGE         25
#define ANALOG_DIVIDER_RATIO_STORAGE_VOLTAGE        25
#define ANALOG_DIVIDER_RATIO_BACKUP_VOLTAGE         10
#endif
#ifdef BPSM
#define ANALOG_DIVIDER_RATIO_SOURCE_VOLTAGE         10
#define ANALOG_DIVIDER_RATIO_STORAGE_VOLTAGE        2
#define ANALOG_DIVIDER_RATIO_BACKUP_VOLTAGE         10
#endif
#ifdef DDRM
#define ANALOG_DIVIDER_RATIO_INPUT_VOLTAGE          10
#define ANALOG_DIVIDER_RATIO_OUTPUT_VOLTAGE         10
#endif
#if ((defined LVRM) && (defined HW1_0))
#define ANALOG_DIVIDER_RATIO_INPUT_VOLTAGE          10
#define ANALOG_DIVIDER_RATIO_OUTPUT_VOLTAGE         10
#endif
#if ((defined LVRM) && (defined HW2_0))
#define ANALOG_DIVIDER_RATIO_INPUT_VOLTAGE          10
#define ANALOG_DIVIDER_RATIO_OUTPUT_VOLTAGE         10
#endif
#ifdef RRM
#define ANALOG_DIVIDER_RATIO_INPUT_VOLTAGE          10
#define ANALOG_DIVIDER_RATIO_OUTPUT_VOLTAGE         10
#endif
#ifdef GPSM
#define ANALOG_DIVIDER_RATIO_GPS_VOLTAGE            2
#define ANALOG_DIVIDER_RATIO_ANTENNA_VOLTAGE        2
#endif
#ifdef UHFM
#define ANALOG_DIVIDER_RATIO_RADIO_VOLTAGE          2
#endif

#define ANALOG_MCU_VOLTAGE_MV_DEFAULT               3300
#define ANALOG_MCU_TEMPERATURE_DEGREES_DEFAULT      25

#define ANALOG_OUTPUT_CURRENT_VOLTAGE_GAIN          59
#define ANALOG_OUTPUT_CURRENT_SHUNT_RESISTOR_MOHMS  10
#define ANALOG_OUTPUT_CURRENT_OFFSET_UA             25000

#define ANALOG_CHARGE_CURRENT_VOLTAGE_GAIN          20
#define ANALOG_CHARGE_CURRENT_VOLTAGE_OFFSET_12BITS 15

#define ANALOG_ERROR_VALUE                          0xFFFF

/*** ANALOG local structures ***/

#ifdef SM
/*******************************************************************/
typedef struct {
    ADC_channel_t adc_channel;
    ANALOG_gain_type_t gain_type;
    int32_t gain;
} ANALOG_channel_configuration_t;
#endif

/*******************************************************************/
typedef struct {
    int32_t mcu_voltage_mv;
} ANALOG_context_t;

/*** ANALOG local global variables ***/

#if ((defined SM) && (defined SM_AIN_ENABLE))
static const ANALOG_channel_configuration_t ANALOG_CHANNEL_CONFIGURATION[ANALOG_CHANNEL_LAST] = {
    { ADC_CHANNEL_AIN0_VOLTAGE, SM_AIN0_GAIN_TYPE, SM_AIN0_GAIN },
    { ADC_CHANNEL_AIN1_VOLTAGE, SM_AIN1_GAIN_TYPE, SM_AIN1_GAIN },
    { ADC_CHANNEL_AIN2_VOLTAGE, SM_AIN2_GAIN_TYPE, SM_AIN2_GAIN },
    { ADC_CHANNEL_AIN3_VOLTAGE, SM_AIN3_GAIN_TYPE, SM_AIN3_GAIN },
};
#endif

static ANALOG_context_t analog_ctx = {
    .mcu_voltage_mv = ANALOG_MCU_VOLTAGE_MV_DEFAULT
};

/*** ANALOG functions ***/

/*******************************************************************/
ANALOG_status_t ANALOG_init(void) {
    // Local variables.
    ANALOG_status_t status = ANALOG_SUCCESS;
    ADC_status_t adc_status = ADC_SUCCESS;
#if ((defined MPMCM) && !(defined MPMCM_ANALOG_MEASURE_ENABLE))
    ADC_SGL_configuration_t adc_config;
#endif
    // Init context.
    analog_ctx.mcu_voltage_mv = ANALOG_MCU_VOLTAGE_MV_DEFAULT;
    // Init internal ADC.
#ifdef MPMCM
#ifndef MPMCM_ANALOG_MEASURE_ENABLE
    adc_config.clock = ADC_CLOCK_SYSCLK;
    adc_config.clock_prescaler = ADC_CLOCK_PRESCALER_NONE;
    adc_status = ADC_SGL_init(ADC_INSTANCE_ANALOG, NULL, &adc_config);
#endif
#else
    adc_status = ADC_init(&ADC_GPIO);
#endif
    ADC_exit_error(ANALOG_ERROR_BASE_ADC);
errors:
    return status;
}

/*******************************************************************/
ANALOG_status_t ANALOG_de_init(void) {
    // Local variables.
    ANALOG_status_t status = ANALOG_SUCCESS;
    ADC_status_t adc_status = ADC_SUCCESS;
    // Release internal ADC.
#ifdef MPMCM
#ifndef MPMCM_ANALOG_MEASURE_ENABLE
    adc_status = ADC_SGL_de_init(ADC_INSTANCE_ANALOG);
#endif
#else
    adc_status = ADC_de_init();
#endif
    ADC_stack_error(ERROR_BASE_ANALOG + ANALOG_ERROR_BASE_ADC);
    return status;
}

/*******************************************************************/
ANALOG_status_t ANALOG_convert_channel(ANALOG_channel_t channel, int32_t* analog_data) {
    // Local variables.
    ANALOG_status_t status = ANALOG_SUCCESS;
    ADC_status_t adc_status = ADC_SUCCESS;
#if (!(defined MPMCM) || ((defined MPMCM) && !(defined MPMCM_ANALOG_MEASURE_ENABLE)))
    int32_t adc_data_12bits = 0;
#endif
#if ((defined BCM) || (defined LVRM) || (defined DDRM) || (defined RRM))
    int64_t num = 0;
    int64_t den = 0;
    int32_t output_current_ua = 0;
#endif
#if ((defined SM) && (defined SM_AIN_ENABLE))
    uint8_t ainx_index = 0;
#endif
    // Check parameter.
    if (analog_data == NULL) {
        status = ANALOG_ERROR_NULL_PARAMETER;
        goto errors;
    }
    // Check channel.
    switch (channel) {
    case ANALOG_CHANNEL_MCU_VOLTAGE_MV:
        // MCU voltage.
#ifdef MPMCM
#ifndef MPMCM_ANALOG_MEASURE_ENABLE
        adc_status = ADC_SGL_convert_channel(ADC_INSTANCE_ANALOG, ADC_CHANNEL_VBAT, &adc_data_12bits);
#endif
#else
        adc_status = ADC_convert_channel(ADC_CHANNEL_VREFINT, &adc_data_12bits);
#endif
        ADC_exit_error(ANALOG_ERROR_BASE_ADC);
        // Convert to mV.
#ifdef MPMCM
#ifndef MPMCM_ANALOG_MEASURE_ENABLE
        adc_status = ADC_compute_mcu_voltage(adc_data_12bits, analog_data);
#else
        (*analog_data) = ANALOG_MCU_VOLTAGE_MV_DEFAULT;
#endif
#else
        adc_status = ADC_compute_mcu_voltage(adc_data_12bits, ADC_get_vrefint_voltage_mv(), analog_data);
#endif
        ADC_exit_error(ANALOG_ERROR_BASE_ADC);
        // Update local value for temperature computation.
        analog_ctx.mcu_voltage_mv = (*analog_data);
        break;
    case ANALOG_CHANNEL_MCU_TEMPERATURE_DEGREES:
        // MCU temperature.
#ifdef MPMCM
#ifndef MPMCM_ANALOG_MEASURE_ENABLE
        adc_status = ADC_SGL_convert_channel(ADC_INSTANCE_ANALOG, ADC_CHANNEL_TEMPERATURE_SENSOR, &adc_data_12bits);
#endif
#else
        adc_status = ADC_convert_channel(ADC_CHANNEL_TEMPERATURE_SENSOR, &adc_data_12bits);
#endif
        ADC_exit_error(ANALOG_ERROR_BASE_ADC);
        // Convert to degrees.
#ifdef MPMCM
#ifndef MPMCM_ANALOG_MEASURE_ENABLE
        adc_status = ADC_compute_mcu_temperature(adc_data_12bits, analog_data);
#else
        (*analog_data) = ANALOG_MCU_TEMPERATURE_DEGREES_DEFAULT;
#endif
#else
        adc_status = ADC_compute_mcu_temperature(analog_ctx.mcu_voltage_mv, adc_data_12bits, analog_data);
#endif
        ADC_exit_error(ANALOG_ERROR_BASE_ADC);
        break;
#ifdef BCM
    case ANALOG_CHANNEL_SOURCE_VOLTAGE_MV:
        // Bus voltage.
        adc_status = ADC_convert_channel(ADC_CHANNEL_SOURCE_VOLTAGE, &adc_data_12bits);
        ADC_exit_error(ANALOG_ERROR_BASE_ADC);
        // Convert to mV.
        (*analog_data) = (adc_data_12bits * analog_ctx.mcu_voltage_mv * ANALOG_DIVIDER_RATIO_SOURCE_VOLTAGE) / (ADC_FULL_SCALE);
        break;
    case ANALOG_CHANNEL_STORAGE_VOLTAGE_MV:
        // Supercap voltage.
        adc_status = ADC_convert_channel(ADC_CHANNEL_STORAGE_VOLTAGE, &adc_data_12bits);
        ADC_exit_error(ANALOG_ERROR_BASE_ADC);
        // Convert to mV.
        (*analog_data) = (adc_data_12bits * analog_ctx.mcu_voltage_mv * ANALOG_DIVIDER_RATIO_STORAGE_VOLTAGE) / (ADC_FULL_SCALE);
        break;
    case ANALOG_CHANNEL_CHARGE_CURRENT_UA:
        // Supercap voltage.
        adc_status = ADC_convert_channel(ADC_CHANNEL_CHARGE_CURRENT, &adc_data_12bits);
        ADC_exit_error(ANALOG_ERROR_BASE_ADC);
        // Remove offset.
        adc_data_12bits = ((adc_data_12bits < ANALOG_CHARGE_CURRENT_VOLTAGE_OFFSET_12BITS) ? 0 : (adc_data_12bits - ANALOG_CHARGE_CURRENT_VOLTAGE_OFFSET_12BITS));
        // Convert to uA.
        num = (int64_t) adc_data_12bits;
        num *= (int64_t) analog_ctx.mcu_voltage_mv;
        num *= (int64_t) MATH_POWER_10[6];
        den = (int64_t) ADC_FULL_SCALE;
        den *= (int64_t) ANALOG_CHARGE_CURRENT_VOLTAGE_GAIN;
        den *= (int64_t) BCM_CHARGE_CURRENT_SHUNT_RESISTOR_MOHMS;
        output_current_ua = (den == 0) ? 0 : (int32_t) ((num) / (den));
        // Set result.
        (*analog_data) = output_current_ua;
        break;
    case ANALOG_CHANNEL_BACKUP_VOLTAGE_MV:
        // Supercap voltage.
        adc_status = ADC_convert_channel(ADC_CHANNEL_BACKUP_VOLTAGE, &adc_data_12bits);
        ADC_exit_error(ANALOG_ERROR_BASE_ADC);
        // Convert to mV.
        (*analog_data) = (adc_data_12bits * analog_ctx.mcu_voltage_mv * ANALOG_DIVIDER_RATIO_BACKUP_VOLTAGE) / (ADC_FULL_SCALE);
        break;
#endif
#ifdef BPSM
    case ANALOG_CHANNEL_SOURCE_VOLTAGE_MV:
        // Bus voltage.
        adc_status = ADC_convert_channel(ADC_CHANNEL_SOURCE_VOLTAGE, &adc_data_12bits);
        ADC_exit_error(ANALOG_ERROR_BASE_ADC);
        // Convert to mV.
        (*analog_data) = (adc_data_12bits * analog_ctx.mcu_voltage_mv * ANALOG_DIVIDER_RATIO_SOURCE_VOLTAGE) / (ADC_FULL_SCALE);
        break;
    case ANALOG_CHANNEL_STORAGE_VOLTAGE_MV:
        // Supercap voltage.
        adc_status = ADC_convert_channel(ADC_CHANNEL_STORAGE_VOLTAGE, &adc_data_12bits);
        ADC_exit_error(ANALOG_ERROR_BASE_ADC);
        // Convert to mV.
        (*analog_data) = (adc_data_12bits * analog_ctx.mcu_voltage_mv * ANALOG_DIVIDER_RATIO_STORAGE_VOLTAGE) / (ADC_FULL_SCALE);
        break;
    case ANALOG_CHANNEL_BACKUP_VOLTAGE_MV:
        // Supercap voltage.
        adc_status = ADC_convert_channel(ADC_CHANNEL_BACKUP_VOLTAGE, &adc_data_12bits);
        ADC_exit_error(ANALOG_ERROR_BASE_ADC);
        // Convert to mV.
        (*analog_data) = (adc_data_12bits * analog_ctx.mcu_voltage_mv * ANALOG_DIVIDER_RATIO_BACKUP_VOLTAGE) / (ADC_FULL_SCALE);
        break;
#endif
#if ((defined DDRM) || (defined LVRM) || (defined RRM))
    case ANALOG_CHANNEL_INPUT_VOLTAGE_MV:
        // Bus voltage.
        adc_status = ADC_convert_channel(ADC_CHANNEL_INPUT_VOLTAGE, &adc_data_12bits);
        ADC_exit_error(ANALOG_ERROR_BASE_ADC);
        // Convert to mV.
        (*analog_data) = (adc_data_12bits * analog_ctx.mcu_voltage_mv * ANALOG_DIVIDER_RATIO_INPUT_VOLTAGE) / (ADC_FULL_SCALE);
        break;
    case ANALOG_CHANNEL_OUTPUT_VOLTAGE_MV:
        // Bus voltage.
        adc_status = ADC_convert_channel(ADC_CHANNEL_OUTPUT_VOLTAGE, &adc_data_12bits);
        ADC_exit_error(ANALOG_ERROR_BASE_ADC);
        // Convert to mV.
        (*analog_data) = (adc_data_12bits * analog_ctx.mcu_voltage_mv * ANALOG_DIVIDER_RATIO_OUTPUT_VOLTAGE) / (ADC_FULL_SCALE);
        break;
    case ANALOG_CHANNEL_OUTPUT_CURRENT_UA:
        // Bus voltage.
        adc_status = ADC_convert_channel(ADC_CHANNEL_OUTPUT_CURRENT, &adc_data_12bits);
        ADC_exit_error(ANALOG_ERROR_BASE_ADC);
        // Convert to uA.
        num = (int64_t) adc_data_12bits;
        num *= (int64_t) analog_ctx.mcu_voltage_mv;
        num *= (int64_t) MATH_POWER_10[6];
        den = (int64_t) ADC_FULL_SCALE;
        den *= (int64_t) ANALOG_OUTPUT_CURRENT_VOLTAGE_GAIN;
        den *= (int64_t) ANALOG_OUTPUT_CURRENT_SHUNT_RESISTOR_MOHMS;
        output_current_ua = (den == 0) ? 0 : (int32_t) ((num) / (den));
        // Remove offset.
        (*analog_data) = (output_current_ua < ANALOG_OUTPUT_CURRENT_OFFSET_UA) ? 0 : (output_current_ua - ANALOG_OUTPUT_CURRENT_OFFSET_UA);
        break;
#endif
#ifdef GPSM
    case ANALOG_CHANNEL_GPS_VOLTAGE_MV:
        // Bus voltage.
        adc_status = ADC_convert_channel(ADC_CHANNEL_GPS_VOLTAGE, &adc_data_12bits);
        ADC_exit_error(ANALOG_ERROR_BASE_ADC);
        // Convert to mV.
        (*analog_data) = (adc_data_12bits * analog_ctx.mcu_voltage_mv * ANALOG_DIVIDER_RATIO_GPS_VOLTAGE) / (ADC_FULL_SCALE);
        break;
    case ANALOG_CHANNEL_ANTENNA_VOLTAGE_MV:
        // Bus voltage.
        adc_status = ADC_convert_channel(ADC_CHANNEL_ANTENNA_VOLTAGE, &adc_data_12bits);
        ADC_exit_error(ANALOG_ERROR_BASE_ADC);
        // Convert to mV.
        (*analog_data) = (adc_data_12bits * analog_ctx.mcu_voltage_mv * ANALOG_DIVIDER_RATIO_ANTENNA_VOLTAGE) / (ADC_FULL_SCALE);
        break;
#endif
#if ((defined SM) && (defined SM_AIN_ENABLE))
    case ANALOG_CHANNEL_AIN0_VOLTAGE_MV:
    case ANALOG_CHANNEL_AIN1_VOLTAGE_MV:
    case ANALOG_CHANNEL_AIN2_VOLTAGE_MV:
    case ANALOG_CHANNEL_AIN3_VOLTAGE_MV:
        // Convert index.
        ainx_index = (channel - ANALOG_CHANNEL_AIN0_VOLTAGE_MV);
        // Convert channel.
        adc_status = ADC_convert_channel(ANALOG_CHANNEL_CONFIGURATION[ainx_index].adc_channel, &adc_data_12bits);
        ADC_exit_error(ANALOG_ERROR_BASE_ADC);
        // Apply gain.
        switch (ANALOG_CHANNEL_CONFIGURATION[ainx_index].gain_type) {
        case ANALOG_GAIN_TYPE_ATTENUATION:
            (*analog_data) = (adc_data_12bits * analog_ctx.mcu_voltage_mv * ANALOG_CHANNEL_CONFIGURATION[ainx_index].gain) / (ADC_FULL_SCALE);
            break;
        case ANALOG_GAIN_TYPE_AMPLIFICATION:
            (*analog_data) = (adc_data_12bits * analog_ctx.mcu_voltage_mv) / (ADC_FULL_SCALE * ANALOG_CHANNEL_CONFIGURATION[ainx_index].gain);
            break;
        default:
            status = ANALOG_ERROR_GAIN_TYPE;
            goto errors;
        }
        break;
#endif
#ifdef UHFM
    case ANALOG_CHANNEL_RADIO_VOLTAGE_MV:
        // Bus voltage.
        adc_status = ADC_convert_channel(ADC_CHANNEL_RADIO_VOLTAGE, &adc_data_12bits);
        ADC_exit_error(ANALOG_ERROR_BASE_ADC);
        // Convert to mV.
        (*analog_data) = (adc_data_12bits * analog_ctx.mcu_voltage_mv * ANALOG_DIVIDER_RATIO_RADIO_VOLTAGE) / (ADC_FULL_SCALE);
        break;
#endif
    default:
        status = ANALOG_ERROR_CHANNEL;
        goto errors;
    }
errors:
    return status;
}

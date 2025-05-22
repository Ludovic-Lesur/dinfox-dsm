/*
 * analog.c
 *
 *  Created on: 15 dec. 2024
 *      Author: Ludo
 */

#include "analog.h"

#include "adc.h"
#include "dsm_flags.h"
#include "error.h"
#include "error_base.h"
#include "mcu_mapping.h"
#include "types.h"

/*** ANALOG local macros ***/

#ifdef BCM
#define ANALOG_DIVIDER_RATIO_VSRC           25
#define ANALOG_DIVIDER_RATIO_VSTR           25
#define ANALOG_DIVIDER_RATIO_VBKP           10
#endif
#ifdef BPSM
#define ANALOG_DIVIDER_RATIO_VSRC           10
#define ANALOG_DIVIDER_RATIO_VSTR           2
#define ANALOG_DIVIDER_RATIO_VBKP           10
#endif
#ifdef DDRM
#define ANALOG_DIVIDER_RATIO_VIN            10
#define ANALOG_DIVIDER_RATIO_VOUT           10
#endif
#if ((defined LVRM) && (defined HW1_0))
#define ANALOG_DIVIDER_RATIO_VIN            10
#define ANALOG_DIVIDER_RATIO_VOUT           10
#endif
#if ((defined LVRM) && (defined HW2_0))
#define ANALOG_DIVIDER_RATIO_VIN            10
#define ANALOG_DIVIDER_RATIO_VOUT           10
#endif
#ifdef RRM
#define ANALOG_DIVIDER_RATIO_VIN            10
#define ANALOG_DIVIDER_RATIO_VOUT           10
#endif
#ifdef GPSM
#define ANALOG_DIVIDER_RATIO_VGPS           2
#define ANALOG_DIVIDER_RATIO_VANT           2
#endif
#ifdef UHFM
#define ANALOG_DIVIDER_RATIO_VRF            2
#endif

#define ANALOG_VMCU_MV_DEFAULT              3000
#define ANALOG_TMCU_DEGREES_DEFAULT         25

#define ANALOG_IOUT_VOLTAGE_GAIN            59
#define ANALOG_IOUT_SHUNT_RESISTOR_MOHMS    10
#define ANALOG_IOUT_OFFSET_UA               25000

#define ANALOG_ISTR_VOLTAGE_GAIN            20
#define ANALOG_ISTR_SHUNT_RESISTOR_MOHMS    50

#define ANALOG_ERROR_VALUE                  0xFFFF

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
    int32_t vmcu_mv;
} ANALOG_context_t;

/*** ANALOG local global variables ***/

#if ((defined SM) && (defined SM_AIN_ENABLE))
static const ANALOG_channel_configuration_t ANALOG_CHANNEL_CONFIGURATION[ANALOG_CHANNEL_LAST] = {
    { ADC_CHANNEL_AIN0, SM_AIN0_GAIN_TYPE, SM_AIN0_GAIN },
    { ADC_CHANNEL_AIN1, SM_AIN1_GAIN_TYPE, SM_AIN1_GAIN },
    { ADC_CHANNEL_AIN2, SM_AIN2_GAIN_TYPE, SM_AIN2_GAIN },
    { ADC_CHANNEL_AIN3, SM_AIN3_GAIN_TYPE, SM_AIN3_GAIN },
};
#endif

static ANALOG_context_t analog_ctx = {
    .vmcu_mv = ANALOG_VMCU_MV_DEFAULT
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
    analog_ctx.vmcu_mv = ANALOG_VMCU_MV_DEFAULT;
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
    int32_t iout_ua = 0;
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
    case ANALOG_CHANNEL_VMCU_MV:
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
        adc_status = ADC_compute_vmcu(adc_data_12bits, analog_data);
#else
        (*analog_data) = ANALOG_VMCU_MV_DEFAULT;
#endif
#else
        adc_status = ADC_compute_vmcu(adc_data_12bits, ADC_get_vrefint_voltage_mv(), analog_data);
#endif
        ADC_exit_error(ANALOG_ERROR_BASE_ADC);
        // Update local value for temperature computation.
        analog_ctx.vmcu_mv = (*analog_data);
        break;
    case ANALOG_CHANNEL_TMCU_DEGREES:
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
        adc_status = ADC_compute_tmcu(adc_data_12bits, analog_data);
#else
        (*analog_data) = ANALOG_TMCU_DEGREES_DEFAULT;
#endif
#else
        adc_status = ADC_compute_tmcu(analog_ctx.vmcu_mv, adc_data_12bits, analog_data);
#endif
        ADC_exit_error(ANALOG_ERROR_BASE_ADC);
        break;
#ifdef BCM
    case ANALOG_CHANNEL_VSRC_MV:
        // Bus voltage.
        adc_status = ADC_convert_channel(ADC_CHANNEL_VSRC, &adc_data_12bits);
        ADC_exit_error(ANALOG_ERROR_BASE_ADC);
        // Convert to mV.
        (*analog_data) = (adc_data_12bits * analog_ctx.vmcu_mv * ANALOG_DIVIDER_RATIO_VSRC) / (ADC_FULL_SCALE);
        break;
    case ANALOG_CHANNEL_VSTR_MV:
        // Supercap voltage.
        adc_status = ADC_convert_channel(ADC_CHANNEL_VSTR, &adc_data_12bits);
        ADC_exit_error(ANALOG_ERROR_BASE_ADC);
        // Convert to mV.
        (*analog_data) = (adc_data_12bits * analog_ctx.vmcu_mv * ANALOG_DIVIDER_RATIO_VSTR) / (ADC_FULL_SCALE);
        break;
    case ANALOG_CHANNEL_ISTR_UA:
        // Supercap voltage.
        adc_status = ADC_convert_channel(ADC_CHANNEL_ISTR, &adc_data_12bits);
        ADC_exit_error(ANALOG_ERROR_BASE_ADC);
        // Convert to uA.
        num = (int64_t) adc_data_12bits;
        num *= (int64_t) analog_ctx.vmcu_mv;
        num *= (int64_t) MATH_POWER_10[6];
        den = (int64_t) ADC_FULL_SCALE;
        den *= (int64_t) ANALOG_ISTR_VOLTAGE_GAIN;
        den *= (int64_t) ANALOG_ISTR_SHUNT_RESISTOR_MOHMS;
        iout_ua = (den == 0) ? 0 : (int32_t) ((num) / (den));
        // Set result.
        (*analog_data) = iout_ua;
        break;
    case ANALOG_CHANNEL_VBKP_MV:
        // Supercap voltage.
        adc_status = ADC_convert_channel(ADC_CHANNEL_VBKP, &adc_data_12bits);
        ADC_exit_error(ANALOG_ERROR_BASE_ADC);
        // Convert to mV.
        (*analog_data) = (adc_data_12bits * analog_ctx.vmcu_mv * ANALOG_DIVIDER_RATIO_VBKP) / (ADC_FULL_SCALE);
        break;
#endif
#ifdef BPSM
    case ANALOG_CHANNEL_VSRC_MV:
        // Bus voltage.
        adc_status = ADC_convert_channel(ADC_CHANNEL_VSRC, &adc_data_12bits);
        ADC_exit_error(ANALOG_ERROR_BASE_ADC);
        // Convert to mV.
        (*analog_data) = (adc_data_12bits * analog_ctx.vmcu_mv * ANALOG_DIVIDER_RATIO_VSRC) / (ADC_FULL_SCALE);
        break;
    case ANALOG_CHANNEL_VSTR_MV:
        // Supercap voltage.
        adc_status = ADC_convert_channel(ADC_CHANNEL_VSTR, &adc_data_12bits);
        ADC_exit_error(ANALOG_ERROR_BASE_ADC);
        // Convert to mV.
        (*analog_data) = (adc_data_12bits * analog_ctx.vmcu_mv * ANALOG_DIVIDER_RATIO_VSTR) / (ADC_FULL_SCALE);
        break;
    case ANALOG_CHANNEL_VBKP_MV:
        // Supercap voltage.
        adc_status = ADC_convert_channel(ADC_CHANNEL_VBKP, &adc_data_12bits);
        ADC_exit_error(ANALOG_ERROR_BASE_ADC);
        // Convert to mV.
        (*analog_data) = (adc_data_12bits * analog_ctx.vmcu_mv * ANALOG_DIVIDER_RATIO_VBKP) / (ADC_FULL_SCALE);
        break;
#endif
#if ((defined DDRM) || (defined LVRM) || (defined RRM))
    case ANALOG_CHANNEL_VIN_MV:
        // Bus voltage.
        adc_status = ADC_convert_channel(ADC_CHANNEL_VIN, &adc_data_12bits);
        ADC_exit_error(ANALOG_ERROR_BASE_ADC);
        // Convert to mV.
        (*analog_data) = (adc_data_12bits * analog_ctx.vmcu_mv * ANALOG_DIVIDER_RATIO_VIN) / (ADC_FULL_SCALE);
        break;
    case ANALOG_CHANNEL_VOUT_MV:
        // Bus voltage.
        adc_status = ADC_convert_channel(ADC_CHANNEL_VOUT, &adc_data_12bits);
        ADC_exit_error(ANALOG_ERROR_BASE_ADC);
        // Convert to mV.
        (*analog_data) = (adc_data_12bits * analog_ctx.vmcu_mv * ANALOG_DIVIDER_RATIO_VOUT) / (ADC_FULL_SCALE);
        break;
    case ANALOG_CHANNEL_IOUT_UA:
        // Bus voltage.
        adc_status = ADC_convert_channel(ADC_CHANNEL_IOUT, &adc_data_12bits);
        ADC_exit_error(ANALOG_ERROR_BASE_ADC);
        // Convert to uA.
        num = (int64_t) adc_data_12bits;
        num *= (int64_t) analog_ctx.vmcu_mv;
        num *= (int64_t) MATH_POWER_10[6];
        den = (int64_t) ADC_FULL_SCALE;
        den *= (int64_t) ANALOG_IOUT_VOLTAGE_GAIN;
        den *= (int64_t) ANALOG_IOUT_SHUNT_RESISTOR_MOHMS;
        iout_ua = (den == 0) ? 0 : (int32_t) ((num) / (den));
        // Remove offset.
        (*analog_data) = (iout_ua < ANALOG_IOUT_OFFSET_UA) ? 0 : (iout_ua - ANALOG_IOUT_OFFSET_UA);
        break;
#endif
#ifdef GPSM
    case ANALOG_CHANNEL_VGPS_MV:
        // Bus voltage.
        adc_status = ADC_convert_channel(ADC_CHANNEL_VGPS, &adc_data_12bits);
        ADC_exit_error(ANALOG_ERROR_BASE_ADC);
        // Convert to mV.
        (*analog_data) = (adc_data_12bits * analog_ctx.vmcu_mv * ANALOG_DIVIDER_RATIO_VGPS) / (ADC_FULL_SCALE);
        break;
    case ANALOG_CHANNEL_VANT_MV:
        // Bus voltage.
        adc_status = ADC_convert_channel(ADC_CHANNEL_VANT, &adc_data_12bits);
        ADC_exit_error(ANALOG_ERROR_BASE_ADC);
        // Convert to mV.
        (*analog_data) = (adc_data_12bits * analog_ctx.vmcu_mv * ANALOG_DIVIDER_RATIO_VANT) / (ADC_FULL_SCALE);
        break;
#endif
#if ((defined SM) && (defined SM_AIN_ENABLE))
    case ANALOG_CHANNEL_AIN0_MV:
    case ANALOG_CHANNEL_AIN1_MV:
    case ANALOG_CHANNEL_AIN2_MV:
    case ANALOG_CHANNEL_AIN3_MV:
        // Convert index.
        ainx_index = (channel - ANALOG_CHANNEL_AIN0_MV);
        // Convert channel.
        adc_status = ADC_convert_channel(ANALOG_CHANNEL_CONFIGURATION[ainx_index].adc_channel, &adc_data_12bits);
        ADC_exit_error(ANALOG_ERROR_BASE_ADC);
        // Apply gain.
        switch (ANALOG_CHANNEL_CONFIGURATION[ainx_index].gain_type) {
        case ANALOG_GAIN_TYPE_ATTENUATION:
            (*analog_data) = (adc_data_12bits * analog_ctx.vmcu_mv * ANALOG_CHANNEL_CONFIGURATION[ainx_index].gain) / (ADC_FULL_SCALE);
            break;
        case ANALOG_GAIN_TYPE_AMPLIFICATION:
            (*analog_data) = (adc_data_12bits * analog_ctx.vmcu_mv) / (ADC_FULL_SCALE * ANALOG_CHANNEL_CONFIGURATION[ainx_index].gain);
            break;
        default:
            status = ANALOG_ERROR_GAIN_TYPE;
            goto errors;
        }
        break;
#endif
#ifdef UHFM
    case ANALOG_CHANNEL_VRF_MV:
        // Bus voltage.
        adc_status = ADC_convert_channel(ADC_CHANNEL_VRF, &adc_data_12bits);
        ADC_exit_error(ANALOG_ERROR_BASE_ADC);
        // Convert to mV.
        (*analog_data) = (adc_data_12bits * analog_ctx.vmcu_mv * ANALOG_DIVIDER_RATIO_VRF) / (ADC_FULL_SCALE);
        break;
#endif
    default:
        status = ANALOG_ERROR_CHANNEL;
        goto errors;
    }
errors:
    return status;
}

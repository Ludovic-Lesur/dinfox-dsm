/*
 * analog.h
 *
 *  Created on: 12 aug. 2024
 *      Author: Ludo
 */

#ifndef __ANALOG_H__
#define __ANALOG_H__

#include "adc.h"
#include "dsm_flags.h"
#include "error.h"
#include "types.h"

/*** ANALOG structures ***/

/*!******************************************************************
 * \enum ANALOG_status_t
 * \brief ANALOG driver error codes.
 *******************************************************************/
typedef enum {
    // Driver errors.
    ANALOG_SUCCESS = 0,
    ANALOG_ERROR_NULL_PARAMETER,
    ANALOG_ERROR_CHANNEL,
    ANALOG_ERROR_CALIBRATION_MISSING,
    ANALOG_ERROR_GAIN_TYPE,
    // Low level drivers errors.
    ANALOG_ERROR_BASE_ADC = ERROR_BASE_STEP,
    // Last base value.
    ANALOG_ERROR_BASE_LAST = (ANALOG_ERROR_BASE_ADC + ADC_ERROR_BASE_LAST),
} ANALOG_status_t;

/*!******************************************************************
 * \enum ANALOG_channel_t
 * \brief ANALOG channels list.
 *******************************************************************/
typedef enum {
    ANALOG_CHANNEL_MCU_VOLTAGE_MV = 0,
    ANALOG_CHANNEL_MCU_TEMPERATURE_DEGREES,
#ifdef BCM
    ANALOG_CHANNEL_SOURCE_VOLTAGE_MV,
    ANALOG_CHANNEL_STORAGE_VOLTAGE_MV,
    ANALOG_CHANNEL_CHARGE_CURRENT_UA,
    ANALOG_CHANNEL_BACKUP_VOLTAGE_MV,
#endif
#ifdef BPSM
    ANALOG_CHANNEL_SOURCE_VOLTAGE_MV,
    ANALOG_CHANNEL_STORAGE_VOLTAGE_MV,
    ANALOG_CHANNEL_BACKUP_VOLTAGE_MV,
#endif
#if ((defined DDRM) || (defined LVRM) || (defined RRM))
    ANALOG_CHANNEL_INPUT_VOLTAGE_MV,
    ANALOG_CHANNEL_OUTPUT_VOLTAGE_MV,
    ANALOG_CHANNEL_OUTPUT_CURRENT_UA,
#endif
#ifdef GPSM
    ANALOG_CHANNEL_GPS_VOLTAGE_MV,
    ANALOG_CHANNEL_ANTENNA_VOLTAGE_MV,
#endif
#if ((defined SM) && (defined SM_AIN_ENABLE))
    ANALOG_CHANNEL_AIN0_VOLTAGE_MV,
    ANALOG_CHANNEL_AIN1_VOLTAGE_MV,
    ANALOG_CHANNEL_AIN2_VOLTAGE_MV,
    ANALOG_CHANNEL_AIN3_VOLTAGE_MV,
#endif
#ifdef UHFM
    ANALOG_CHANNEL_RADIO_VOLTAGE_MV,
#endif
    ANALOG_CHANNEL_LAST
} ANALOG_channel_t;

#ifdef SM
/*!******************************************************************
 * \enum ANALOG_gain_type_t
 * \brief ANALOG gain types list.
 *******************************************************************/
typedef enum {
    ANALOG_GAIN_TYPE_ATTENUATION = 0,
    ANALOG_GAIN_TYPE_AMPLIFICATION,
    ANALOG_GAIN_TYPE_LAST
} ANALOG_gain_type_t;
#endif

/*** ANALOG functions ***/

/*!******************************************************************
 * \fn ANALOG_status_t ANALOG_init(void)
 * \brief Init ANALOG driver.
 * \param[in]   none
 * \param[out]  none
 * \retval      Function execution status.
 *******************************************************************/
ANALOG_status_t ANALOG_init(void);

/*!******************************************************************
 * \fn ANALOG_status_t ANALOG_de_init(void)
 * \brief Release ANALOG driver.
 * \param[in]   none
 * \param[out]  none
 * \retval      Function execution status.
 *******************************************************************/
ANALOG_status_t ANALOG_de_init(void);

/*!******************************************************************
 * \fn ANALOG_status_t ANALOG_convert_channel(ANALOG_channel_t channel, int32_t* analog_data)
 * \brief Convert an analog channel.
 * \param[in]   channel: Channel to convert.
 * \param[out]  analog_data: Pointer to integer that will contain the result.
 * \retval      Function execution status.
 *******************************************************************/
ANALOG_status_t ANALOG_convert_channel(ANALOG_channel_t channel, int32_t* analog_data);

/*******************************************************************/
#define ANALOG_exit_error(base) { ERROR_check_exit(analog_status, ANALOG_SUCCESS, base) }

/*******************************************************************/
#define ANALOG_stack_error(base) { ERROR_check_stack(analog_status, ANALOG_SUCCESS, base) }

/*******************************************************************/
#define ANALOG_stack_exit_error(base, code) { ERROR_check_stack_exit(analog_status, ANALOG_SUCCESS, base, code) }

#endif /* __ANALOG_H__ */

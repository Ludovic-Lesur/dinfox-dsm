/*
 * mode.h
 *
 *  Created on: 18 apr. 2020
 *      Author: Ludo
 */

#ifndef __MODE_H__
#define __MODE_H__

/*** AT commands mode ***/

//#define ATM	// AT command mode.

/*** Specific nodes options ***/

#ifdef LVRM
//#define LVRM_RLST_FORCED_HARDWARE
#endif

#ifdef BPSM
#define BPSM_BKEN_FORCED_HARDWARE
#define BPSM_CHEN_FORCED_HARDWARE
#define BPSM_CHST_FORCED_HARDWARE
#define BPSM_VSTR_VOLTAGE_DIVIDER_RATIO		2
#endif

#ifdef DDRM
//#define DDRM_DDEN_FORCED_HARDWARE
#endif

#ifdef GPSM
#define GPSM_ACTIVE_ANTENNA
//#define GPSM_BKEN_FORCED_HARDWARE
#endif

#ifdef SM
#define SM_AIN_ENABLE
#define SM_DIO_ENABLE
#define SM_DIGITAL_SENSORS_ENABLE
#ifdef SM_AIN_ENABLE
#define SM_AIN0_CONVERSION_TYPE		ADC_CONVERSION_TYPE_VOLTAGE_ATTENUATION
#define SM_AIN0_GAIN				1
#define SM_AIN1_CONVERSION_TYPE		ADC_CONVERSION_TYPE_VOLTAGE_ATTENUATION
#define SM_AIN1_GAIN				1
#define SM_AIN2_CONVERSION_TYPE		ADC_CONVERSION_TYPE_VOLTAGE_ATTENUATION
#define SM_AIN2_GAIN				1
#define SM_AIN3_CONVERSION_TYPE		ADC_CONVERSION_TYPE_VOLTAGE_ATTENUATION
#define SM_AIN3_GAIN				1
#endif
#endif

#ifdef RRM
//#define RRM_REN_FORCED_HARDWARE
#endif

/*** Debug mode ***/

//#define DEBUG		// Use programming pins for debug purpose if defined.

#endif /* __MODE_H__ */

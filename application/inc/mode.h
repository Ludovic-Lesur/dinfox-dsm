/*
 * mode.h
 *
 *  Created on: 18 apr. 2020
 *      Author: Ludo
 */

#ifndef __MODE_H__
#define __MODE_H__

/*** Board modes ***/

//#define XM_DEBUG
//#define XM_NVM_FACTORY_RESET
//#define XM_MODE_CLI

/*** Board options ***/

#ifdef XM_NVM_FACTORY_RESET
//#define XM_NODE_ADDRESS                   0x7F
#endif

#ifdef LVRM
#ifdef XM_NVM_FACTORY_RESET
#define LVRM_BMS_VBATT_LOW_THRESHOLD_MV     10000
#define LVRM_BMS_VBATT_HIGH_THRESHOLD_MV    12000
#endif
//#define LVRM_RLST_FORCED_HARDWARE
#ifndef LVRM_RLST_FORCED_HARDWARE
//#define LVRM_MODE_BMS
#endif
#endif

#ifdef BPSM
//#define BPSM_CHEN_FORCED_HARDWARE
#define BPSM_CHST_FORCED_HARDWARE
#define BPSM_BKEN_FORCED_HARDWARE
#define BPSM_DIVIDER_RATIO_VSTR             2
#ifdef XM_NVM_FACTORY_RESET
#define BPSM_CHEN_VSRC_THRESHOLD_MV         6000
#define BPSM_CHEN_TOGGLE_PERIOD_SECONDS     300
#endif
#endif

#ifdef DDRM
//#define DDRM_DDEN_FORCED_HARDWARE
#endif

#ifdef GPSM
#define GPSM_ACTIVE_ANTENNA
//#define GPSM_BKEN_FORCED_HARDWARE
#ifdef XM_NVM_FACTORY_RESET
#define GPSM_TIME_TIMEOUT_SECONDS           120
#define GPSM_GEOLOC_TIMEOUT_SECONDS         180
#define GPSM_TIMEPULSE_FREQUENCY_HZ         10000000
#define GPSM_TIMEPULSE_DUTY_CYCLE           50
#endif
#endif

#ifdef SM
#define SM_AIN_ENABLE
#define SM_DIO_ENABLE
#define SM_DIGITAL_SENSORS_ENABLE
#ifdef SM_AIN_ENABLE
#define SM_AIN0_GAIN_TYPE                   ANALOG_GAIN_TYPE_ATTENUATION
#define SM_AIN0_GAIN                        1
#define SM_AIN1_GAIN_TYPE                   ANALOG_GAIN_TYPE_ATTENUATION
#define SM_AIN1_GAIN                        1
#define SM_AIN2_GAIN_TYPE                   ANALOG_GAIN_TYPE_ATTENUATION
#define SM_AIN2_GAIN                        1
#define SM_AIN3_GAIN_TYPE                   ANALOG_GAIN_TYPE_ATTENUATION
#define SM_AIN3_GAIN                        1
#endif
#endif

#ifdef RRM
//#define RRM_REN_FORCED_HARDWARE
#endif

/*** Second level compilation flags ***/

#if ((defined BPSM) || (defined LVRM) || (defined DDRM) || (defined RRM))
#define XM_LOAD_CONTROL
#endif
#if ((defined LVRM) || (defined DDRM) || (defined RRM))
#define XM_IOUT_INDICATOR
#endif
#if ((defined XM_IOUT_INDICATOR) || (defined GPSM))
#define XM_RGB_LED
#endif

#endif /* __MODE_H__ */

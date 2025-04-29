/*
 * dsm_flags.h
 *
 *  Created on: 18 apr. 2020
 *      Author: Ludo
 */

#ifndef __DSM_FLAGS_H__
#define __DSM_FLAGS_H__

/*** Board modes ***/

//#define DSM_DEBUG
//#define DSM_NVM_FACTORY_RESET

/*** Board options ***/

#ifdef DSM_NVM_FACTORY_RESET
#define DSM_NODE_ADDRESS                    0x7F
#endif

#ifdef LVRM
#ifdef DSM_NVM_FACTORY_RESET
#define LVRM_BMS_VBATT_LOW_THRESHOLD_MV     10000
#define LVRM_BMS_VBATT_HIGH_THRESHOLD_MV    12000
#endif
//#define LVRM_RLST_FORCED_HARDWARE
#ifndef LVRM_RLST_FORCED_HARDWARE
//#define LVRM_MODE_BMS
#endif
#endif

#ifdef BCM
#define BCM_CHEN_FORCED_HARDWARE
//#define BCM_CHST_FORCED_HARDWARE
#define BCM_CHLD_FORCED_HARDWARE
#define BCM_BKEN_FORCED_HARDWARE
#ifdef DSM_NVM_FACTORY_RESET
#define BCM_CHEN_VSRC_THRESHOLD_MV         16000
#define BCM_CHEN_TOGGLE_PERIOD_SECONDS     3600
#define BCM_LVF_LOW_THRESHOLD_MV           10000
#define BCM_LVF_HIGH_THRESHOLD_MV          12000
#endif
#endif

#ifdef BPSM
//#define BPSM_CHEN_FORCED_HARDWARE
#define BPSM_CHST_FORCED_HARDWARE
#define BPSM_BKEN_FORCED_HARDWARE
#ifdef DSM_NVM_FACTORY_RESET
#define BPSM_CHEN_VSRC_THRESHOLD_MV         6000
#define BPSM_CHEN_TOGGLE_PERIOD_SECONDS     300
#define BPSM_LVF_LOW_THRESHOLD_MV           1000
#define BPSM_LVF_HIGH_THRESHOLD_MV          2000
#endif
#endif

#ifdef DDRM
//#define DDRM_DDEN_FORCED_HARDWARE
#endif

#ifdef GPSM
#define GPSM_ACTIVE_ANTENNA
//#define GPSM_BKEN_FORCED_HARDWARE
#ifdef DSM_NVM_FACTORY_RESET
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

#if ((defined BCM) || (defined BPSM) || (defined LVRM) || (defined DDRM) || (defined RRM))
#define DSM_LOAD_CONTROL
#endif
#if (((defined BCM) && !(defined BCM_CHLD_FORCED_HARDWARE)) || (defined LVRM) || (defined DDRM) || (defined RRM))
#define DSM_IOUT_INDICATOR
#endif
#if ((defined DSM_IOUT_INDICATOR) || (defined GPSM))
#define DSM_RGB_LED
#endif

#endif /* __DSM_FLAGS_H__ */

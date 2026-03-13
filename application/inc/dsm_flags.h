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
#define DSM_NODE_ADDRESS                            0x7F
#endif

#ifdef LVRM
//#define LVRM_RELAY_CONTROL_FORCED_HARDWARE
#ifndef LVRM_RELAY_CONTROL_FORCED_HARDWARE
//#define LVRM_MODE_BMS
#endif
#ifdef DSM_NVM_FACTORY_RESET
#define LVRM_BMS_INPUT_VOLTAGE_THL_MV               10000
#define LVRM_BMS_INPUT_VOLTAGE_THH_MV               12000
#endif
#endif

#ifdef BPSM
//#define BPSM_CHARGE_CONTROL_FORCED_HARDWARE
#define BPSM_CHARGE_STATUS_FORCED_HARDWARE
#define BPSM_BACKUP_CONTROL_FORCED_HARDWARE
#ifdef DSM_NVM_FACTORY_RESET
#define BPSM_CHARGE_SOURCE_VOLTAGE_TH_MV             6000
#define BPSM_CHARGE_TOGGLE_PERIOD_SECONDS            300
#define BPSM_LVF_STORAGE_VOLTAGE_THL_MV              1000
#define BPSM_LVF_STORAGE_VOLTAGE_THH_MV              2000
#define BPSM_CVF_STORAGE_VOLTAGE_THL_MV              1000
#define BPSM_CVF_STORAGE_VOLTAGE_THH_MV              2000
#endif
#endif

#ifdef DDRM
//#define DDRM_REGULATOR_CONTROL_FORCED_HARDWARE
#endif

#ifdef RRM
//#define RRM_REGULATOR_CONTROL_FORCED_HARDWARE
#endif

#ifdef SM
#define SM_AIN_ENABLE
#define SM_DIO_ENABLE
#define SM_DIGITAL_SENSORS_ENABLE
#define SM_AIN0_GAIN_TYPE                           ANALOG_GAIN_TYPE_ATTENUATION
#define SM_AIN0_GAIN                                1
#define SM_AIN1_GAIN_TYPE                           ANALOG_GAIN_TYPE_ATTENUATION
#define SM_AIN1_GAIN                                1
#define SM_AIN2_GAIN_TYPE                           ANALOG_GAIN_TYPE_ATTENUATION
#define SM_AIN2_GAIN                                1
#define SM_AIN3_GAIN_TYPE                           ANALOG_GAIN_TYPE_ATTENUATION
#define SM_AIN3_GAIN                                1
#endif

#ifdef GPSM
#define GPSM_ACTIVE_ANTENNA
//#define GPSM_BACKUP_CONTROL_FORCED_HARDWARE
#ifdef DSM_NVM_FACTORY_RESET
#define GPSM_TIME_TIMEOUT_SECONDS                   120
#define GPSM_GEOLOC_TIMEOUT_SECONDS                 180
#define GPSM_TIMEPULSE_FREQUENCY_HZ                 10000000
#define GPSM_TIMEPULSE_DUTY_CYCLE                   50
#endif
#endif

#ifdef MPMCM
// Measurements selection.
#define MPMCM_ANALOG_MEASURE_ENABLE
//#define MPMCM_LINKY_TIC_ENABLE
// Linky TIC mode.
#define MPMCM_LINKY_TIC_MODE_HISTORIC
//#define MPMCM_LINKY_TIC_MODE_STANDARD
// Transformer selection.
//#define MPMCM_TRANSFORMER_BLOCK_VC_10_2_6
#define MPMCM_TRANSFORMER_BLOCK_VB_2_1_6
// Transformer settings.
#ifdef MPMCM_TRANSFORMER_BLOCK_VC_10_2_6
#define MPMCM_TRANSFORMER_ATTENUATOR_VV             11
#ifdef DSM_NVM_FACTORY_RESET
#define MPMCM_TRANSFORMER_GAIN_DVV                  300
#endif
#endif
#ifdef MPMCM_TRANSFORMER_BLOCK_VB_2_1_6
#define MPMCM_TRANSFORMER_ATTENUATOR_VV             15
#ifdef DSM_NVM_FACTORY_RESET
#define MPMCM_TRANSFORMER_GAIN_DVV                  236
#endif
#endif
// Current sensors settings.
#define MPMCM_CURRENT_SENSOR_ATTENUATOR_CH1_VV      1
#define MPMCM_CURRENT_SENSOR_ATTENUATOR_CH2_VV      1
#define MPMCM_CURRENT_SENSOR_ATTENUATOR_CH3_VV      1
#define MPMCM_CURRENT_SENSOR_ATTENUATOR_CH4_VV      1
#ifdef DSM_NVM_FACTORY_RESET
#define MPMCM_CURRENT_SENSOR_GAIN_CH1_DAV           50
#define MPMCM_CURRENT_SENSOR_GAIN_CH2_DAV           50
#define MPMCM_CURRENT_SENSOR_GAIN_CH3_DAV           100
#define MPMCM_CURRENT_SENSOR_GAIN_CH4_DAV           200
#endif
#endif

#ifdef BCM
#define BCM_CHARGE_CURRENT_SHUNT_RESISTOR_MOHMS     50
#define BCM_CHARGE_CONTROL_FORCED_HARDWARE
//#define BCM_CHARGE_STATUS_FORCED_HARDWARE
#define BCM_CHARGE_LED_FORCED_HARDWARE
#define BCM_BACKUP_CONTROL_FORCED_HARDWARE
#ifdef DSM_NVM_FACTORY_RESET
#define BCM_CHARGE_SOURCE_VOLTAGE_TH_MV             16000
#define BCM_CHARGE_TOGGLE_PERIOD_SECONDS            3600
#define BCM_LVF_STORAGE_VOLTAGE_THL_MV              10000
#define BCM_LVF_STORAGE_VOLTAGE_THH_MV              12000
#define BCM_CVF_STORAGE_VOLTAGE_THL_MV              8000
#define BCM_CVF_STORAGE_VOLTAGE_THH_MV              10000
#endif
#endif

#endif /* __DSM_FLAGS_H__ */

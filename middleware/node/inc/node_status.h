/*
 * node_status.h
 *
 *  Created on: 15 jan. 2026
 *      Author: Ludo
 */

#ifndef __NODE_STATUS_H__
#define __NODE_STATUS_H__

#include "analog.h"
#include "digital.h"
#include "error.h"
#include "gps.h"
#include "measure.h"
#include "led.h"
#include "lptim.h"
#include "load.h"
#include "nvm.h"
#include "power.h"
#include "s2lp.h"
#include "sht3x.h"
#include "tic.h"
#include "types.h"

/*!******************************************************************
 * \enum NODE_status_t
 * \brief NODE driver error codes.
 *******************************************************************/
typedef enum {
    // Driver errors.
    NODE_SUCCESS = 0,
    NODE_ERROR_NULL_PARAMETER,
    NODE_ERROR_REGISTER_ADDRESS,
    NODE_ERROR_REGISTER_READ_ONLY,
    NODE_ERROR_REGISTER_FIELD_VALUE,
    NODE_ERROR_RADIO_STATE,
    NODE_ERROR_RADIO_POWER,
    NODE_ERROR_FORCED_HARDWARE,
    NODE_ERROR_FORCED_SOFTWARE,
    NODE_ERROR_SIGFOX_MCU_API,
    NODE_ERROR_SIGFOX_RF_API,
    NODE_ERROR_SIGFOX_EP_API,
    // Low level drivers errors.
    NODE_ERROR_BASE_NVM = ERROR_BASE_STEP,
    NODE_ERROR_BASE_LPTIM = (NODE_ERROR_BASE_NVM + NVM_ERROR_BASE_LAST),
    NODE_ERROR_BASE_DIGITAL = (NODE_ERROR_BASE_LPTIM + LPTIM_ERROR_BASE_LAST),
    NODE_ERROR_BASE_LED = (NODE_ERROR_BASE_DIGITAL + DIGITAL_ERROR_BASE_LAST),
    NODE_ERROR_BASE_LOAD = (NODE_ERROR_BASE_LED + LED_ERROR_BASE_LAST),
    NODE_ERROR_BASE_GPS = (NODE_ERROR_BASE_LOAD + LOAD_ERROR_BASE_LAST),
    NODE_ERROR_BASE_MEASURE = (NODE_ERROR_BASE_GPS + GPS_ERROR_BASE_LAST),
    NODE_ERROR_BASE_POWER = (NODE_ERROR_BASE_MEASURE + MEASURE_ERROR_BASE_LAST),
    NODE_ERROR_BASE_S2LP = (NODE_ERROR_BASE_POWER + POWER_ERROR_BASE_LAST),
    NODE_ERROR_BASE_SHT3X = (NODE_ERROR_BASE_S2LP + S2LP_ERROR_BASE_LAST),
    NODE_ERROR_BASE_TIC = (NODE_ERROR_BASE_SHT3X + SHT3X_ERROR_BASE_LAST),
    NODE_ERROR_BASE_ANALOG = (NODE_ERROR_BASE_TIC + TIC_ERROR_BASE_LAST),
    NODE_ERROR_BASE_SIGFOX_EP_ADDON_RFP_API = (NODE_ERROR_BASE_ANALOG + ANALOG_ERROR_BASE_LAST),
    // Last base value.
    NODE_ERROR_BASE_LAST = (NODE_ERROR_BASE_SIGFOX_EP_ADDON_RFP_API + ERROR_BASE_STEP)
} NODE_status_t;

#endif /* __NODE_STATUS_H__ */

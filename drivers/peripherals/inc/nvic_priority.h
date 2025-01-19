/*
 * nvic_priority.h
 *
 *  Created on: 02 jan. 2025
 *      Author: Ludo
 */

#ifndef __NVIC_PRIORITY_H__
#define __NVIC_PRIORITY_H__

#include "xm_flags.h"

/*!******************************************************************
 * \enum NVIC_priority_list_t
 * \brief NVIC interrupt priorities list.
 *******************************************************************/
typedef enum {
    // Common.
    NVIC_PRIORITY_CLOCK = 0,
    NVIC_PRIORITY_CLOCK_CALIBRATION = 1,
    NVIC_PRIORITY_DELAY = 2,
    NVIC_PRIORITY_RTC = 3,
    // RS485 interface.
    NVIC_PRIORITY_RS485 = 0,
#ifdef XM_RGB_LED
    NVIC_PRIORITY_LED = 1,
#endif
#ifdef GPSM
    NVIC_PRIORITY_GPS_UART = 0,
#endif
#ifdef UHFM
    NVIC_PRIORITY_SIGFOX_RADIO_IRQ_GPIO = 0,
    NVIC_PRIORITY_SIGFOX_TIMER = 1,
#endif
} NVIC_priority_list_t;

#endif /* __NVIC_PRIORITY_H__ */

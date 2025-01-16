/*
 * neom8x_driver_flags.h
 *
 *  Created on: 11 jan. 2025
 *      Author: Ludo
 */

#ifndef __NEOM8X_DRIVER_FLAGS_H__
#define __NEOM8X_DRIVER_FLAGS_H__

#include "lptim.h"
#include "usart.h"

/*** NEOM8x driver compilation flags ***/

#ifndef GPSM
#define NEOM8X_DRIVER_DISABLE
#endif

#define NEOM8X_DRIVER_GPIO_ERROR_BASE_LAST              0
#define NEOM8X_DRIVER_UART_ERROR_BASE_LAST              USART_ERROR_BASE_LAST
#define NEOM8X_DRIVER_DELAY_ERROR_BASE_LAST             LPTIM_ERROR_BASE_LAST

#define NEOM8X_DRIVER_GPS_DATA_TIME
#define NEOM8X_DRIVER_GPS_DATA_POSITION

#define NEOM8X_DRIVER_ALTITUDE_STABILITY_FILTER_MODE    1
#if (NEOM8X_DRIVER_ALTITUDE_STABILITY_FILTER_MODE == 1)
#define NEOM8X_DRIVER_ALTITUDE_STABILITY_THRESHOLD      5
#endif

#define NEOM8X_DRIVER_VBCKP_CONTROL

#define NEOM8X_DRIVER_TIMEPULSE

#endif /* __NEOM8X_DRIVER_FLAGS_H__ */

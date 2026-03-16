/*
 * sx126x_driver_flags.h
 *
 *  Created on: 14 sep. 2025
 *      Author: Ludo
 */

#ifndef __SX126X_DRIVER_FLAGS_H__
#define __SX126X_DRIVER_FLAGS_H__

#include "error_patch.h"
#include "lptim.h"
#ifndef MPMCM
#include "spi.h"
#endif

/*** SX126X driver compilation flags ***/

#if (!(defined UHFM) || ((defined UHFM) && (defined HW1_0)))
#define SX126X_DRIVER_DISABLE
#endif

#define SX126X_DRIVER_GPIO_ERROR_BASE_LAST      0
#define SX126X_DRIVER_SPI_ERROR_BASE_LAST       SPI_ERROR_BASE_LAST
#define SX126X_DRIVER_DELAY_ERROR_BASE_LAST     LPTIM_ERROR_BASE_LAST

//#define SX126X_DRIVER_DEVICE_SX1262

#define SX126X_DRIVER_FXOSC_HZ                  32000000

#define SX126X_DRIVER_TX_ENABLE
#define SX126X_DRIVER_RX_ENABLE

#endif /* __SX126X_DRIVER_FLAGS_H__ */

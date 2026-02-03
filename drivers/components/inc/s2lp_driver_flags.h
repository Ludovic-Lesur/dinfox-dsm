/*
 * s2lp_driver_flags.h
 *
 *  Created on: 17 nov. 2024
 *      Author: Ludo
 */

#ifndef __S2LP_DRIVER_FLAGS_H__
#define __S2LP_DRIVER_FLAGS_H__

#include "error_patch.h"
#include "lptim.h"
#ifndef MPMCM
#include "spi.h"
#endif

/*** S2LP driver compilation flags ***/

#if (!(defined UHFM) || ((defined UHFM) && (defined HW2_0)))
#define S2LP_DRIVER_DISABLE
#endif

#define S2LP_DRIVER_SPI_ERROR_BASE_LAST     SPI_ERROR_BASE_LAST
#define S2LP_DRIVER_DELAY_ERROR_BASE_LAST   LPTIM_ERROR_BASE_LAST

#define S2LP_DRIVER_XO_FREQUENCY_HZ         49152000

#define S2LP_DRIVER_TX_ENABLE
#define S2LP_DRIVER_RX_ENABLE

#endif /* __S2LP_DRIVER_FLAGS_H__ */

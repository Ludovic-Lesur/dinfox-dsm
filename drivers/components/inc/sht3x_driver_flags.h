/*
 * sht3x_driver_flags.h
 *
 *  Created on: 12 jan. 2025
 *      Author: Ludo
 */

#ifndef __SHT3X_DRIVER_FLAGS_H__
#define __SHT3X_DRIVER_FLAGS_H__

#ifndef MPMCM
#include "i2c.h"
#endif
#include "lptim.h"

#ifdef MPMCM
#define I2C_ERROR_BASE_LAST                 0
#endif

/*** SHT3x driver compilation flags ***/

#ifndef SM
#define SHT3X_DRIVER_DISABLE
#endif

#define SHT3X_DRIVER_I2C_ERROR_BASE_LAST    I2C_ERROR_BASE_LAST
#define SHT3X_DRIVER_DELAY_ERROR_BASE_LAST  LPTIM_ERROR_BASE_LAST

#endif /* __SHT3X_DRIVER_FLAGS_H__ */

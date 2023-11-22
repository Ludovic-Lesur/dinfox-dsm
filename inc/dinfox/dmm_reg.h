/*
 * dmm_reg.h
 *
 *  Created on: 14 jun. 2023
 *      Author: Ludo
 */

#ifndef __DMM_REG_H__
#define __DMM_REG_H__

#include "common_reg.h"
#include "types.h"

/*** DMM registers address ***/

/*!******************************************************************
 * \enum DMM_register_address_t
 * \brief DMM registers map.
 *******************************************************************/
typedef enum {
	DMM_REG_ADDR_CONFIGURATION_0 = COMMON_REG_ADDR_LAST,
	DMM_REG_ADDR_CONFIGURATION_1,
	DMM_REG_ADDR_STATUS_1,
	DMM_REG_ADDR_CONTROL_1,
	DMM_REG_ADDR_ANALOG_DATA_1,
	DMM_REG_ADDR_ANALOG_DATA_2,
	DMM_REG_ADDR_LAST,
} DMM_register_address_t;

/*** DMM number of specific registers ***/

#define DMM_NUMBER_OF_SPECIFIC_REG						(DMM_REG_ADDR_LAST - COMMON_REG_ADDR_LAST)

/*** DMM registers mask ***/

#define DMM_REG_CONFIGURATION_0_MASK_BMSF				0x00000001

#define DMM_REG_CONFIGURATION_1_MASK_NODES_SCAN_PERIOD	0x000000FF
#define DMM_REG_CONFIGURATION_1_MASK_SIGFOX_UL_PERIOD	0x0000FF00
#define DMM_REG_CONFIGURATION_1_MASK_SIGFOX_DL_PERIOD	0x00FF0000

#define DMM_REG_STATUS_1_MASK_NODES_COUNT				0x000000FF

#define DMM_REG_CONTROL_1_MASK_STRG						0x00000001

#define DMM_REG_ANALOG_DATA_1_MASK_VRS					0x0000FFFF
#define DMM_REG_ANALOG_DATA_1_MASK_VHMI					0xFFFF0000

#define DMM_REG_ANALOG_DATA_2_MASK_VUSB					0x0000FFFF

#endif /* __DMM_REG_H__ */

/*
 * lvrm_reg.h
 *
 *  Created on: 12 mar. 2022
 *      Author: Ludo
 */

#ifndef __LVRM_REG_H__
#define __LVRM_REG_H__

#include "common_reg.h"
#include "types.h"

/*** LVRM registers address ***/

/*!******************************************************************
 * \enum LVRM_register_address_t
 * \brief LVRM registers map.
 *******************************************************************/
typedef enum {
	LVRM_REG_ADDR_CONFIGURATION_0 = COMMON_REG_ADDR_LAST,
	LVRM_REG_ADDR_CONFIGURATION_1,
	LVRM_REG_ADDR_CONFIGURATION_2,
	LVRM_REG_ADDR_STATUS_1,
	LVRM_REG_ADDR_CONTROL_1,
	LVRM_REG_ADDR_ANALOG_DATA_1,
	LVRM_REG_ADDR_ANALOG_DATA_2,
	LVRM_REG_ADDR_LAST,
} LVRM_register_address_t;

/*** LVRM number of specific registers ***/

#define LVRM_NUMBER_OF_SPECIFIC_REG							(LVRM_REG_ADDR_LAST - COMMON_REG_ADDR_LAST)

/*** LVRM registers mask ***/

#define LVRM_REG_CONFIGURATION_0_MASK_BMSF					0x00000001
#define LVRM_REG_CONFIGURATION_0_MASK_RLFH					0x00000002

#define LVRM_REG_CONFIGURATION_1_MASK_VBATT_LOW_THRESHOLD	0x0000FFFF
#define LVRM_REG_CONFIGURATION_1_MASK_VBATT_HIGH_THRESHOLD	0xFFFF0000

#define LVRM_REG_CONFIGURATION_2_MASK_IOUT_OFFSET			0x0000FFFF

#define LVRM_REG_STATUS_1_MASK_RLSTST						0x00000003

#define LVRM_REG_CONTROL_1_MASK_RLST						0x00000001
#define LVRM_REG_CONTROL_1_MASK_ZCCT						0x00000002

#define LVRM_REG_ANALOG_DATA_1_MASK_VCOM					0x0000FFFF
#define LVRM_REG_ANALOG_DATA_1_MASK_VOUT					0xFFFF0000

#define LVRM_REG_ANALOG_DATA_2_MASK_IOUT					0x0000FFFF

#endif /* __LVRM_REG_H__ */

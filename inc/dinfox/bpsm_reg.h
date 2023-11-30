/*
 * bpsm_reg.h
 *
 *  Created on: 27 nov. 2022
 *      Author: Ludo
 */

#ifndef __BPSM_REG_H__
#define __BPSM_REG_H__

#include "common_reg.h"
#include "types.h"

/*** BPSM registers address ***/

/*!******************************************************************
 * \enum BPSM_register_address_t
 * \brief BPSM registers map.
 *******************************************************************/
typedef enum {
	BPSM_REG_ADDR_CONFIGURATION_0 = COMMON_REG_ADDR_LAST,
	BPSM_REG_ADDR_CONFIGURATION_1,
	BPSM_REG_ADDR_STATUS_1,
	BPSM_REG_ADDR_CONTROL_1,
	BPSM_REG_ADDR_ANALOG_DATA_1,
	BPSM_REG_ADDR_ANALOG_DATA_2,
	BPSM_REG_ADDR_LAST,
} BPSM_register_address_t;

/*** BPSM number of specific registers ***/

#define BPSM_NUMBER_OF_SPECIFIC_REG							(BPSM_REG_ADDR_LAST - COMMON_REG_ADDR_LAST)

/*** BPSM registers mask ***/

#define BPSM_REG_CONFIGURATION_0_MASK_VSTR_RATIO			0x000000FF

#define BPSM_REG_CONFIGURATION_1_MASK_CHEN_THRESHOLD		0x0000FFFF
#define BPSM_REG_CONFIGURATION_1_MASK_CHEN_TOGGLE_PERIOD	0xFFFF0000

#define BPSM_REG_STATUS_1_MASK_BKENST						0x00000003
#define BPSM_REG_STATUS_1_MASK_CHENST						0x0000000C
#define BPSM_REG_STATUS_1_MASK_CHRGST						0x00000030

#define BPSM_REG_CONTROL_1_MASK_BKEN						0x00000001
#define BPSM_REG_CONTROL_1_MASK_CHMD						0x00000002
#define BPSM_REG_CONTROL_1_MASK_CHEN						0x00000004

#define BPSM_REG_ANALOG_DATA_1_MASK_VSRC					0x0000FFFF
#define BPSM_REG_ANALOG_DATA_1_MASK_VSTR					0xFFFF0000

#define BPSM_REG_ANALOG_DATA_2_MASK_VBKP					0x0000FFFF

#endif /* __BPSM_REG_H__ */


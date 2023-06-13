/*
 * rrm_reg.h
 *
 *  Created on: 27 nov. 2022
 *      Author: Ludo
 */

#ifndef __RRM_REG_H__
#define __RRM_REG_H__

#include "common_reg.h"
#include "types.h"

/*** RRM registers address ***/

typedef enum {
	RRM_REG_ADDR_STATUS_CONTROL_1 = COMMON_REG_ADDR_LAST,
	RRM_REG_ADDR_ANALOG_DATA_1,
	RRM_REG_ADDR_ANALOG_DATA_2,
	RRM_REG_ADDR_LAST,
} RRM_register_address_t;

/*** RRM number of specific registers ***/

#define RRM_NUMBER_OF_SPECIFIC_REG			(RRM_REG_ADDR_LAST - COMMON_REG_ADDR_LAST)

/*** RRM registers mask ***/

#define RRM_REG_ANALOG_DATA_1_MASK_VIN		0x0000FFFF
#define RRM_REG_ANALOG_DATA_1_MASK_VOUT		0xFFFF0000

#define RRM_REG_ANALOG_DATA_2_MASK_IOUT		0x0000FFFF

#define RRM_REG_STATUS_CONTROL_1_MASK_REN	0x00000001

#endif /* __RRM_REG_H__ */

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

#ifdef BPSM

/*** BPSM registers address ***/

typedef enum {
	BPSM_REG_ADDR_STATUS_CONTROL_1 = COMMON_REG_ADDR_LAST,
	BPSM_REG_ADDR_ANALOG_DATA_1,
	BPSM_REG_ADDR_ANALOG_DATA_2,
	BPSM_REG_ADDR_LAST,
} BPSM_register_address_t;

/*** BPSM registers mask ***/

#define BPSM_REG_ANALOG_DATA_1_MASK_VSRC		0x0000FFFF
#define BPSM_REG_ANALOG_DATA_1_MASK_VSTR		0xFFFF0000

#define BPSM_REG_ANALOG_DATA_2_MASK_VBKP		0x0000FFFF

#define BPSM_REG_STATUS_CONTROL_1_MASK_BKEN		0x00000001
#define BPSM_REG_STATUS_CONTROL_1_MASK_CHEN		0x00000002
#define BPSM_REG_STATUS_CONTROL_1_MASK_CHST		0x00000004

#endif /* BPSM */

#endif /* __BPSM_REG_H__ */


/*
 * lvrm_reg.h
 *
 *  Created on: 12 mar. 2022
 *      Author: Ludo
 */

#ifndef __LVRM_REG_H__
#define __LVRM_REG_H__

#include "dinfox_reg.h"
#include "types.h"

#ifdef LVRM

/*** LVRM registers address ***/

typedef enum {
	LVRM_REG_ADDR_STATUS_CONTROL_1 = DINFOX_REG_ADDR_LAST,
	LVRM_REG_ADDR_ANALOG_DATA_1,
	LVRM_REG_ADDR_ANALOG_DATA_2,
	NODE_REG_ADDR_LAST,
} LVRM_register_address_t;

/*** LVRM registers mask ***/

#define LVRM_REG_ANALOG_DATA_1_MASK_VCOM	0x0000FFFF
#define LVRM_REG_ANALOG_DATA_1_MASK_VOUT	0xFFFF0000

#define LVRM_REG_ANALOG_DATA_2_MASK_IOUT	0x0000FFFF

#define LVRM_REG_STATUS_CONTROL_1_MASK_RLST	0x00000001

#endif /* LVRM */

#endif /* __LVRM_REG_H__ */

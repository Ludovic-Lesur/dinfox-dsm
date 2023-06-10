/*
 * sm_reg.h
 *
 *  Created on: 16 jan. 2023
 *      Author: Ludo
 */

#ifndef __SM_REG_H__
#define __SM_REG_H__

#include "dinfox_reg.h"
#include "types.h"

#ifdef SM

/*** SM registers mapping ***/

typedef enum {
	SM_REG_ADDR_ANALOG_DATA_1 = DINFOX_REG_ADDR_LAST,
	SM_REG_ADDR_ANALOG_DATA_2,
	SM_REG_ADDR_ANALOG_DATA_3,
	SM_REG_ADDR_DIGITAL_DATA,
	NODE_REG_ADDR_LAST,
} SM_register_address_t;
#endif

/*** SM registers mask ***/

#define SM_REG_ANALOG_DATA_1_MASK_VAIN0		0x0000FFFF
#define SM_REG_ANALOG_DATA_1_MASK_VAIN1		0xFFFF0000

#define SM_REG_ANALOG_DATA_2_MASK_VAIN2		0x0000FFFF
#define SM_REG_ANALOG_DATA_2_MASK_VAIN3		0xFFFF0000

#define SM_REG_ANALOG_DATA_3_MASK_TAMB		0x000000FF
#define SM_REG_ANALOG_DATA_3_MASK_HAMB		0x0000FF00

#define SM_REG_DIGITAL_DATA_MASK_DIO0		0x00000001
#define SM_REG_DIGITAL_DATA_MASK_DIO1		0x00000002
#define SM_REG_DIGITAL_DATA_MASK_DIO2		0x00000004
#define SM_REG_DIGITAL_DATA_MASK_DIO3		0x00000008

#endif /* __SM_REG_H__ */

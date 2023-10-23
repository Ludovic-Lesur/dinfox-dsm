/*
 * sm_reg.h
 *
 *  Created on: 16 jan. 2023
 *      Author: Ludo
 */

#ifndef __SM_REG_H__
#define __SM_REG_H__

#include "common_reg.h"
#include "types.h"

/*** SM registers mapping ***/

/*!******************************************************************
 * \enum SM_register_address_t
 * \brief SM registers map.
 *******************************************************************/
typedef enum {
	SM_REG_ADDR_ANALOG_DATA_1 = COMMON_REG_ADDR_LAST,
	SM_REG_ADDR_ANALOG_DATA_2,
	SM_REG_ADDR_ANALOG_DATA_3,
	SM_REG_ADDR_DIGITAL_DATA,
	SM_REG_ADDR_LAST,
} SM_register_address_t;

/*** SM number of specific registers ***/

#define SM_NUMBER_OF_SPECIFIC_REG			(SM_REG_ADDR_LAST - COMMON_REG_ADDR_LAST)

/*** SM registers mask ***/

#define SM_REG_ANALOG_DATA_1_MASK_VAIN0		0x0000FFFF
#define SM_REG_ANALOG_DATA_1_MASK_VAIN1		0xFFFF0000

#define SM_REG_ANALOG_DATA_2_MASK_VAIN2		0x0000FFFF
#define SM_REG_ANALOG_DATA_2_MASK_VAIN3		0xFFFF0000

#define SM_REG_ANALOG_DATA_3_MASK_TAMB		0x000000FF
#define SM_REG_ANALOG_DATA_3_MASK_HAMB		0x0000FF00

#define SM_REG_DIGITAL_DATA_MASK_DIO0		0x00000003
#define SM_REG_DIGITAL_DATA_MASK_DIO1		0x0000000C
#define SM_REG_DIGITAL_DATA_MASK_DIO2		0x00000030
#define SM_REG_DIGITAL_DATA_MASK_DIO3		0x000000C0

#endif /* __SM_REG_H__ */

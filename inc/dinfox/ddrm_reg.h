/*
 * ddrm_reg.h
 *
 *  Created on: 27 nov. 2022
 *      Author: Ludo
 */

#ifndef __DDRM_REG_H__
#define __DDRM_REG_H__

#include "common_reg.h"
#include "types.h"

/*** DDRM registers address ***/

/*!******************************************************************
 * \enum DDRM_register_address_t
 * \brief DDRM registers map.
 *******************************************************************/
typedef enum {
	DDRM_REG_ADDR_STATUS_CONTROL_1 = COMMON_REG_ADDR_LAST,
	DDRM_REG_ADDR_ANALOG_DATA_1,
	DDRM_REG_ADDR_ANALOG_DATA_2,
	DDRM_REG_ADDR_LAST,
} DDRM_register_address_t;

/*** BPSM number of specific registers ***/

#define DDRM_NUMBER_OF_SPECIFIC_REG			(DDRM_REG_ADDR_LAST - COMMON_REG_ADDR_LAST)

/*** DDRM registers mask ***/

#define DDRM_REG_ANALOG_DATA_1_MASK_VIN		0x0000FFFF
#define DDRM_REG_ANALOG_DATA_1_MASK_VOUT	0xFFFF0000

#define DDRM_REG_ANALOG_DATA_2_MASK_IOUT	0x0000FFFF

#define DDRM_REG_STATUS_CONTROL_1_MASK_DDEN	0x00000001

#endif /* __DDRM_REG_H__ */

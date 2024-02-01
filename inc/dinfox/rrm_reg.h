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

/*!******************************************************************
 * \enum RRM_register_address_t
 * \brief RRM registers map.
 *******************************************************************/
typedef enum {
	RRM_REG_ADDR_CONFIGURATION_0 = COMMON_REG_ADDR_LAST,
	RRM_REG_ADDR_CONFIGURATION_1,
	RRM_REG_ADDR_STATUS_1,
	RRM_REG_ADDR_CONTROL_1,
	RRM_REG_ADDR_ANALOG_DATA_1,
	RRM_REG_ADDR_ANALOG_DATA_2,
	RRM_REG_ADDR_LAST,
} RRM_register_address_t;

/*** RRM number of specific registers ***/

#define RRM_NUMBER_OF_SPECIFIC_REG					(RRM_REG_ADDR_LAST - COMMON_REG_ADDR_LAST)

/*** RRM registers mask ***/

#define RRM_REG_CONFIGURATION_0_MASK_RFH			0x00000001

#define RRM_REG_CONFIGURATION_1_MASK_IOUT_OFFSET	0x0000FFFF

#define RRM_REG_STATUS_1_MASK_RENST					0x00000003

#define RRM_REG_CONTROL_1_MASK_REN					0x00000001
#define RRM_REG_CONTROL_1_MASK_ZCCT					0x00000002

#define RRM_REG_ANALOG_DATA_1_MASK_VIN				0x0000FFFF
#define RRM_REG_ANALOG_DATA_1_MASK_VOUT				0xFFFF0000

#define RRM_REG_ANALOG_DATA_2_MASK_IOUT				0x0000FFFF

#endif /* __RRM_REG_H__ */

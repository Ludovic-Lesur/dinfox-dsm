/*
 * sm.h
 *
 *  Created on: 16 jan. 2023
 *      Author: Ludo
 */

#ifndef __SM_H__
#define __SM_H__

#include "dinfox.h"

/*** BPSM registers mapping ***/

#ifdef SM
typedef enum {
	SM_REGISTER_AIN0_MV = DINFOX_REGISTER_LAST,
	SM_REGISTER_AIN1_MV,
	SM_REGISTER_AIN2_MV,
	SM_REGISTER_AIN3_MV,
	SM_REGISTER_DIO0,
	SM_REGISTER_DIO1,
	SM_REGISTER_DIO2,
	SM_REGISTER_DIO3,
	SM_REGISTER_TAMB_DEGREES,
	SM_REGISTER_HAMB_PERCENT,
	SM_REGISTER_LAST,
} SM_register_address_t;
#endif

#endif /* __SM_H__ */

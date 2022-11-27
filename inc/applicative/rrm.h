/*
 * rrm.h
 *
 *  Created on: 27 nov. 2022
 *      Author: Ludo
 */

#ifndef __RRM_H__
#define __RRM_H__

#include "dinfox.h"
#include "mode.h"

#ifdef RRM
typedef enum {
	RRM_REGISTER_VIN_MV = DINFOX_REGISTER_LAST,
	RRM_REGISTER_VOUT_MV,
	RRM_REGISTER_IOUT_UA,
	RRM_REGISTER_VMCU_MV,
	RRM_REGISTER_TMCU_DEGREES,
	RRM_REGISTER_OUT_EN,
	RRM_REGISTER_LAST,
} RRM_register_address_t;
#endif

#endif /* __RRM_H__ */

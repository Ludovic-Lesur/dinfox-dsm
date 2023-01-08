/*
 * ddrm.h
 *
 *  Created on: 27 nov. 2022
 *      Author: Ludo
 */

#ifndef __DDRM_H__
#define __DDRM_H__

#include "dinfox.h"

/*** DDRM registers mapping ***/

#ifdef DDRM
typedef enum {
	DDRM_REGISTER_VIN_MV = DINFOX_REGISTER_LAST,
	DDRM_REGISTER_VOUT_MV,
	DDRM_REGISTER_IOUT_UA,
	DDRM_REGISTER_OUT_EN,
	DDRM_REGISTER_LAST,
} DDRM_register_address_t;
#endif

#endif /* __DDRM_H__ */

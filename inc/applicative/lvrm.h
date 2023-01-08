/*
 * lvrm.h
 *
 *  Created on: Mar 12, 2022
 *      Author: Ludo
 */

#ifndef __LVRM_H__
#define __LVRM_H__

#include "dinfox.h"

/*** LVRM registers mapping ***/

#ifdef LVRM
typedef enum {
	LVRM_REGISTER_VCOM_MV = DINFOX_REGISTER_LAST,
	LVRM_REGISTER_VOUT_MV,
	LVRM_REGISTER_IOUT_UA,
	LVRM_REGISTER_OUT_EN,
	LVRM_REGISTER_LAST,
} LVRM_register_address_t;
#endif

#endif /* __LVRM_H__ */

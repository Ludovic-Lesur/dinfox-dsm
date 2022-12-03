/*
 * lvrm.h
 *
 *  Created on: Mar 12, 2022
 *      Author: Ludo
 */

#ifndef LVRM_H
#define LVRM_H

#include "dinfox.h"
#include "mode.h"

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

#endif /* LVRM_H */

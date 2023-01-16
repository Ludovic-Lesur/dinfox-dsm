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
	SM_REGISTER_TAMB_DEGREES = DINFOX_REGISTER_LAST,
	SM_REGISTER_HAMB_PERCENT,
	SM_REGISTER_LAST,
} SM_register_address_t;
#endif

#endif /* __SM_H__ */

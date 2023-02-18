/*
 * bpsm.h
 *
 *  Created on: 27 nov. 2022
 *      Author: Ludo
 */

#ifndef __BPSM_H__
#define __BPSM_H__

#include "dinfox.h"

/*** BPSM registers mapping ***/

#ifdef BPSM
typedef enum {
	BPSM_REGISTER_VSRC_MV = DINFOX_REGISTER_LAST,
	BPSM_REGISTER_VSTR_MV,
	BPSM_REGISTER_VBKP_MV,
	BPSM_REGISTER_CHARGE_ENABLE,
	BPSM_REGISTER_CHARGE_STATUS,
	BPSM_REGISTER_BACKUP_ENABLE,
	BPSM_REGISTER_LAST,
} BPSM_register_address_t;
#endif

#endif /* __BPSM_H__ */

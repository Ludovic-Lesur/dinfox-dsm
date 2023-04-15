/*
 * gpsm.h
 *
 *  Created on: 26 mar. 2023
 *      Author: Ludo
 */

#ifndef __GPSM_H__
#define __GPSM_H__

#include "dinfox.h"

/*** GPSM registers mapping ***/

#ifdef GPSM
typedef enum {
	GPSM_REGISTER_VGPS_MV = DINFOX_REGISTER_LAST,
	GPSM_REGISTER_VANT_MV,
	GPSM_REGISTER_LAST,
} GPSM_register_address_t;
#endif

#endif /* __GPSM_H__ */

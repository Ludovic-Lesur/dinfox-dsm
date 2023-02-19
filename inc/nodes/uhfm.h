/*
 * uhfm.h
 *
 *  Created on: Mar 31, 2022
 *      Author: Ludo
 */

#ifndef __UHFM_H__
#define __UHFM_H__

#include "dinfox.h"

#ifdef UHFM
typedef enum {
	UHFM_REGISTER_VRF_MV = DINFOX_REGISTER_LAST,
	UHFM_REGISTER_LAST,
} UHFM_register_address_t;
#endif

#endif /* __UHFM_H__ */

/*
 * lbus_common.h
 *
 *  Created on: 16 feb. 2023
 *      Author: Ludo
 */

#ifndef __LBUS_COMMON_H__
#define __LBUS_COMMON_H__

#include "types.h"

/*** NODE common macros ***/

#define LBUS_ADDRESS_MASK	0x7F
#define LBUS_ADDRESS_LAST	LBUS_ADDRESS_MASK

typedef uint8_t	LBUS_address_t;

#endif /* __LBUS_COMMON_H__ */

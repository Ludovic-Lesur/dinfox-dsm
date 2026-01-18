/*
 * node_register.c
 *
 *  Created on: 15 jan. 2026
 *      Author: Ludo
 */

#include "node_register.h"

#include "types.h"

/*** NODE REGISTER global variables ***/

uint32_t NODE_RAM_REGISTER[NODE_REGISTER_ADDRESS_LAST] = { [0 ... (NODE_REGISTER_ADDRESS_LAST - 1)] = 0x00000000 };

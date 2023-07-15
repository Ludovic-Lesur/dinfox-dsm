/*
 * at_bus.h
 *
 *  Created on: 18 apr. 2020
 *      Author: Ludo
 */

#ifndef __AT_BUS_H__
#define __AT_BUS_H__

#include "mode.h"
#include "node_common.h"
#include "types.h"
#if (defined UHFM) && (defined ATM)
#include "sigfox_types.h"
#endif

/*** AT functions ***/

void AT_BUS_init(NODE_address_t self_address);
void AT_BUS_task(void);
void AT_BUS_fill_rx_buffer(uint8_t rx_byte);
#if (defined UHFM) && (defined ATM)
void AT_BUS_print_dl_payload(sfx_u8 *dl_payload, sfx_u8 dl_payload_size, sfx_s16 rssi_dbm);
#endif

#endif /* __AT_BUS_H__ */

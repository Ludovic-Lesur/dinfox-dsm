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

/*!******************************************************************
 * \fn void AT_BUS_init(NODE_address_t self_address)
 * \brief Init AT BUS interface.
 * \param[in]  	self_address: RS485 address of the node.
 * \param[out] 	none
 * \retval		none
 *******************************************************************/
void AT_BUS_init(NODE_address_t self_address);

/*!******************************************************************
 * \fn void AT_BUS_task(void)
 * \brief AT BUS interface task.
 * \param[in]  	none
 * \param[out] 	none
 * \retval		none
 *******************************************************************/
void AT_BUS_task(void);

#if (defined ATM) && (defined UHFM)
/*!******************************************************************
 * \fn void AT_BUS_print_dl_payload(sfx_u8 *dl_payload, sfx_u8 dl_payload_size, sfx_s16 rssi_dbm)
 * \brief Print a downlink frame (only used by the RFP addon during downlink test modes).
 * \param[in]  	dl_payload: Downlink payload to print.
 * \param[in] 	dl_payload_size: Number of bytes to print.
 * \param[in]	rssi_dbm: RSSI of the received downlink frame (16-bits signed value).
 * \param[out] 	none
 * \retval		Function execution status.
 *******************************************************************/
void AT_BUS_print_dl_payload(sfx_u8 *dl_payload, sfx_u8 dl_payload_size, sfx_s16 rssi_dbm);
#endif

#endif /* __AT_BUS_H__ */

/*
 * sm.h
 *
 *  Created on: 10 jun. 2023
 *      Author: Ludo
 */

#ifndef __SM_H__
#define __SM_H__

#include "sm_registers.h"
#include "node.h"
#include "una.h"

#ifdef SM

/*** SM macros ***/

#define NODE_BOARD_ID               UNA_BOARD_ID_SM
#define NODE_REGISTER_ADDRESS_LAST  SM_REGISTER_ADDRESS_LAST
#define NODE_REGISTER_ACCESS        SM_REGISTER_ACCESS
#define NODE_REGISTER_ERROR_VALUE   SM_REGISTER_ERROR_VALUE

/*** SM functions ***/

/*!******************************************************************
 * \fn NODE_status_t SM_init_registers(void)
 * \brief Init SM registers to their default value.
 * \param[in]   none
 * \param[out]  none
 * \retval      Function execution status.
 *******************************************************************/
NODE_status_t SM_init_registers(void);

/*!******************************************************************
 * \fn NODE_status_t SM_update_register(uint8_t reg_addr)
 * \brief Update SM register.
 * \param[in]   reg_addr: Address of the register to update.
 * \param[out]  none
 * \retval      Function execution status.
 *******************************************************************/
NODE_status_t SM_update_register(uint8_t reg_addr);

/*!******************************************************************
 * \fn NODE_status_t SM_check_register(uint8_t reg_addr)
 * \brief Check SM register.
 * \param[in]   reg_addr: Address of the register to check.
 * \param[in]   reg_mask: Mask of the bits to check.
 * \param[out]  none
 * \retval      Function execution status.
 *******************************************************************/
NODE_status_t SM_check_register(uint8_t reg_addr, uint32_t reg_mask);

/*!******************************************************************
 * \fn NODE_status_t SM_mtrg_callback(void)
 * \brief SM measurements callback.
 * \param[in]   none
 * \param[out]  none
 * \retval      Function execution status.
 *******************************************************************/
NODE_status_t SM_mtrg_callback(void);

#endif /* SM */

#endif /* __SM_H__ */

/*
 * rrm.h
 *
 *  Created on: 10 jun. 2023
 *      Author: Ludo
 */

#ifndef __RRM_H__
#define __RRM_H__

#include "node.h"
#include "rrm_registers.h"
#include "una.h"

#ifdef RRM

/*** RRM macros ***/

#define NODE_BOARD_ID               UNA_BOARD_ID_RRM
#define NODE_REGISTER_ADDRESS_LAST  RRM_REGISTER_ADDRESS_LAST
#define NODE_REGISTER_ACCESS        RRM_REGISTER_ACCESS
#define NODE_REGISTER_ERROR_VALUE   RRM_REGISTER_ERROR_VALUE

/*** RRM functions ***/

/*!******************************************************************
 * \fn NODE_status_t RRM_init_registers(void)
 * \brief Init RRM registers to their default value.
 * \param[in]   none
 * \param[out]  none
 * \retval      Function execution status.
 *******************************************************************/
NODE_status_t RRM_init_registers(void);

/*!******************************************************************
 * \fn NODE_status_t RRM_update_register(uint8_t reg_addr)
 * \brief Update RRM register.
 * \param[in]   reg_addr: Address of the register to update.
 * \param[out]  none
 * \retval      Function execution status.
 *******************************************************************/
NODE_status_t RRM_update_register(uint8_t reg_addr);

/*!******************************************************************
 * \fn NODE_status_t RRM_check_register(uint8_t reg_addr)
 * \brief Check RRM register.
 * \param[in]   reg_addr: Address of the register to check.
 * \param[in]   reg_mask: Mask of the bits to check.
 * \param[out]  none
 * \retval      Function execution status.
 *******************************************************************/
NODE_status_t RRM_check_register(uint8_t reg_addr, uint32_t reg_mask);

/*!******************************************************************
 * \fn NODE_status_t RRM_mtrg_callback(void)
 * \brief RRM measurements callback.
 * \param[in]   none
 * \param[out]  none
 * \retval      Function execution status.
 *******************************************************************/
NODE_status_t RRM_mtrg_callback(void);

#endif /* RRM */

#endif /* INC_NODES_RRM_H_ */

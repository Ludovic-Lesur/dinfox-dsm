/*
 * ddrm.h
 *
 *  Created on: 04 jun. 2023
 *      Author: Ludo
 */

#ifndef __DDRM_H__
#define __DDRM_H__

#include "ddrm_registers.h"
#include "node.h"
#include "una.h"

#ifdef DDRM

/*** DDRM macros ***/

#define NODE_BOARD_ID               UNA_BOARD_ID_DDRM
#define NODE_REGISTER_ADDRESS_LAST  DDRM_REGISTER_ADDRESS_LAST
#define NODE_REGISTER_ACCESS        DDRM_REGISTER_ACCESS
#define NODE_REGISTER_ERROR_VALUE   DDRM_REGISTER_ERROR_VALUE

/*** DDRM functions ***/

/*!******************************************************************
 * \fn NODE_status_t DDRM_init_registers(void)
 * \brief Init DDRM registers to their default value.
 * \param[in]   none
 * \param[out]  none
 * \retval      Function execution status.
 *******************************************************************/
NODE_status_t DDRM_init_registers(void);

/*!******************************************************************
 * \fn NODE_status_t DDRM_update_register(uint8_t reg_addr)
 * \brief Update DDRM register.
 * \param[in]   reg_addr: Address of the register to update.
 * \param[out]  none
 * \retval      Function execution status.
 *******************************************************************/
NODE_status_t DDRM_update_register(uint8_t reg_addr);

/*!******************************************************************
 * \fn NODE_status_t DDRM_check_register(uint8_t reg_addr)
 * \brief Check DDRM register.
 * \param[in]   reg_addr: Address of the register to check.
 * \param[in]   reg_mask: Mask of the bits to check.
 * \param[out]  none
 * \retval      Function execution status.
 *******************************************************************/
NODE_status_t DDRM_check_register(uint8_t reg_addr, uint32_t reg_mask);

/*!******************************************************************
 * \fn NODE_status_t DDRM_mtrg_callback(void)
 * \brief DDRM measurements callback.
 * \param[in]   none
 * \param[out]  none
 * \retval      Function execution status.
 *******************************************************************/
NODE_status_t DDRM_mtrg_callback(void);

#endif /* DDRM */

#endif /* __DDRM_H__ */

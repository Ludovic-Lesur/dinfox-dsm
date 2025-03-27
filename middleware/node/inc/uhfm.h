/*
 * uhfm.h
 *
 *  Created on: 04 jun. 2023
 *      Author: Ludo
 */

#ifndef __UHFM_H__
#define __UHFM_H__

#include "adc.h"
#include "node.h"
#include "uhfm_registers.h"
#include "una.h"

#ifdef UHFM

/*** UHFM macros ***/

#define NODE_BOARD_ID               UNA_BOARD_ID_UHFM
#define NODE_REGISTER_ADDRESS_LAST  UHFM_REGISTER_ADDRESS_LAST
#define NODE_REGISTER_ACCESS        UHFM_REGISTER_ACCESS
#define NODE_REGISTER_ERROR_VALUE   UHFM_REGISTER_ERROR_VALUE

/*** UHFM functions ***/

/*!******************************************************************
 * \fn NODE_status_t UHFM_init_registers(void)
 * \brief Init UHFM registers to their default value.
 * \param[in]   none
 * \param[out]  none
 * \retval      Function execution status.
 *******************************************************************/
NODE_status_t UHFM_init_registers(void);

/*!******************************************************************
 * \fn NODE_status_t UHFM_update_register(uint8_t reg_addr)
 * \brief Update UHFM register.
 * \param[in]   reg_addr: Address of the register to update.
 * \param[out]  none
 * \retval      Function execution status.
 *******************************************************************/
NODE_status_t UHFM_update_register(uint8_t reg_addr);

/*!******************************************************************
 * \fn NODE_status_t UHFM_check_register(uint8_t reg_addr)
 * \brief Check UHFM register.
 * \param[in]   reg_addr: Address of the register to check.
 * \param[in]   reg_mask: Mask of the bits to check.
 * \param[out]  none
 * \retval      Function execution status.
 *******************************************************************/
NODE_status_t UHFM_check_register(uint8_t reg_addr, uint32_t reg_mask);

/*!******************************************************************
 * \fn NODE_status_t UHFM_mtrg_callback(void)
 * \brief UHFM measurements callback.
 * \param[in]   none
 * \param[out]  none
 * \retval      Function execution status.
 *******************************************************************/
NODE_status_t UHFM_mtrg_callback(void);

#endif /* UHFM */

#endif /* __UHFM_H__ */

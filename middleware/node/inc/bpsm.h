/*
 * bpsm.h
 *
 *  Created on: 04 jun. 2023
 *      Author: Ludo
 */

#ifndef __BPSM_H__
#define __BPSM_H__

#include "bpsm_registers.h"
#include "node.h"
#include "una.h"

#ifdef BPSM

/*** BPSM macros ***/

#define NODE_BOARD_ID               UNA_BOARD_ID_BPSM
#define NODE_REGISTER_ADDRESS_LAST  BPSM_REGISTER_ADDRESS_LAST
#define NODE_REGISTER_ACCESS        BPSM_REGISTER_ACCESS

/*** BPSM functions ***/

/*!******************************************************************
 * \fn NODE_status_t BPSM_init_registers(void)
 * \brief Init BPSM registers to their default value.
 * \param[in]   none
 * \param[out]  none
 * \retval      Function execution status.
 *******************************************************************/
NODE_status_t BPSM_init_registers(void);

/*!******************************************************************
 * \fn NODE_status_t BPSM_update_register(uint8_t reg_addr)
 * \brief Update BPSM register.
 * \param[in]   reg_addr: Address of the register to update.
 * \param[out]  none
 * \retval      Function execution status.
 *******************************************************************/
NODE_status_t BPSM_update_register(uint8_t reg_addr);

/*!******************************************************************
 * \fn NODE_status_t BPSM_check_register(uint8_t reg_addr)
 * \brief Check BPSM register.
 * \param[in]   reg_addr: Address of the register to check.
 * \param[in]   reg_mask: Mask of the bits to check.
 * \param[out]  none
 * \retval      Function execution status.
 *******************************************************************/
NODE_status_t BPSM_check_register(uint8_t reg_addr, uint32_t reg_mask);

/*!******************************************************************
 * \fn NODE_status_t BPSM_mtrg_callback(void)
 * \brief BPSM measurements callback.
 * \param[in]   none
 * \param[out]  none
 * \retval      Function execution status.
 *******************************************************************/
NODE_status_t BPSM_mtrg_callback(void);

#ifndef BPSM_CHEN_FORCED_HARDWARE
/*!******************************************************************
 * \fn NODE_status_t BPSM_charge_process(void)
 * \brief BPSM automatic charge control process.
 * \param[in]   none
 * \param[out]  none
 * \retval      Function execution status.
 *******************************************************************/
NODE_status_t BPSM_charge_process(void);
#endif

#endif /* BPSM */

#endif /* __BPSM_H__ */

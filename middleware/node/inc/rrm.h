/*
 * rrm.h
 *
 *  Created on: 10 jun. 2023
 *      Author: Ludo
 */

#ifndef __RRM_H__
#define __RRM_H__

#include "node_status.h"
#include "rrm_registers.h"
#include "una.h"

#ifdef RRM

/*** RRM macros ***/

#define NODE_BOARD_ID               UNA_BOARD_ID_RRM
#define NODE_REGISTER_ADDRESS_LAST  RRM_REGISTER_ADDRESS_LAST
#define NODE_REGISTER               RRM_REGISTER

#define NODE_INIT                   RRM_init
#define NODE_INIT_REGISTER          RRM_init_register
#define NODE_SECURE_REGISTER        RRM_secure_register
#define NODE_PROCESS_REGISTER       RRM_process_register
#define NODE_REFRESH_REGISTER       RRM_refresh_register
#define NODE_MTRG_CALLBACK          RRM_mtrg_callback

/*** RRM functions ***/

/*!******************************************************************
 * \fn NODE_status_t RRM_init(void)
 * \brief Init RRM driver.
 * \param[in]   none
 * \param[out]  none
 * \retval      Function execution status.
 *******************************************************************/
NODE_status_t RRM_init(void);

/*!******************************************************************
 * \fn void RRM_init_register(uint8_t reg_addr, uint32_t* reg_value)
 * \brief Get RRM register initial value.
 * \param[in]   reg_addr: Address of the register to initialize.
 * \param[out]  reg_value: Pointer to the initial register value.
 * \retval      none
 *******************************************************************/
void RRM_init_register(uint8_t reg_addr, uint32_t* reg_value);

/*!******************************************************************
 * \fn void RRM_refresh_register(uint8_t reg_addr)
 * \brief Refresh RRM register.
 * \param[in]   reg_addr: Address of the register to update.
 * \param[out]  none
 * \retval      none
 *******************************************************************/
void RRM_refresh_register(uint8_t reg_addr);

/*!******************************************************************
 * \fn NODE_status_t RRM_secure_register(uint8_t reg_addr, uint32_t new_reg_value, uint32_t* reg_mask, uint32_t* reg_value)
 * \brief Secure RRM register.
 * \param[in]   reg_addr: Address of the register to secure.
 * \param[in]   new_reg_value: Register value supposed to be written.
 * \PARAM[out]  reg_mask: Pointer to the mask to secure.
 * \param[out]  reg_value: Pointer to the secured register value.
 * \retval      Function execution status.
 *******************************************************************/
NODE_status_t RRM_secure_register(uint8_t reg_addr, uint32_t new_reg_value, uint32_t* reg_mask, uint32_t* reg_value);

/*!******************************************************************
 * \fn NODE_status_t RRM_process_register(uint8_t reg_addr, uint32_t reg_mask)
 * \brief Process RRM register.
 * \param[in]   reg_addr: Address of the register to process.
 * \param[in]   reg_mask: Mask of the bits to process.
 * \param[out]  none
 * \retval      Function execution status.
 *******************************************************************/
NODE_status_t RRM_process_register(uint8_t reg_addr, uint32_t reg_mask);

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

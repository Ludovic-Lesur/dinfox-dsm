/*
 * sm.h
 *
 *  Created on: 10 jun. 2023
 *      Author: Ludo
 */

#ifndef __SM_H__
#define __SM_H__

#include "sm_registers.h"
#include "node_status.h"
#include "una.h"

#ifdef SM

/*** SM macros ***/

#define NODE_BOARD_ID               UNA_BOARD_ID_SM
#define NODE_REGISTER_ADDRESS_LAST  SM_REGISTER_ADDRESS_LAST
#define NODE_REGISTER               SM_REGISTER

#define NODE_INIT                   SM_init
#define NODE_INIT_REGISTER          SM_init_register
#define NODE_SECURE_REGISTER        SM_secure_register
#define NODE_PROCESS_REGISTER       SM_process_register
#define NODE_REFRESH_REGISTER       SM_refresh_register
#define NODE_MTRG_CALLBACK          SM_mtrg_callback

/*** SM functions ***/

/*!******************************************************************
 * \fn NODE_status_t SM_init(void)
 * \brief Init SM driver.
 * \param[in]   none
 * \param[out]  none
 * \retval      Function execution status.
 *******************************************************************/
NODE_status_t SM_init(void);

/*!******************************************************************
 * \fn void SM_init_register(uint8_t reg_addr, uint32_t* reg_value)
 * \brief Get SM register initial value.
 * \param[in]   reg_addr: Address of the register to initialize.
 * \param[out]  reg_value: Pointer to the initial register value.
 * \retval      none
 *******************************************************************/
void SM_init_register(uint8_t reg_addr, uint32_t* reg_value);

/*!******************************************************************
 * \fn void SM_refresh_register(uint8_t reg_addr)
 * \brief Refresh SM register.
 * \param[in]   reg_addr: Address of the register to update.
 * \param[out]  none
 * \retval      none
 *******************************************************************/
void SM_refresh_register(uint8_t reg_addr);

/*!******************************************************************
 * \fn NODE_status_t SM_secure_register(uint8_t reg_addr, uint32_t new_reg_value, uint32_t* reg_mask, uint32_t* reg_value)
 * \brief Secure SM register.
 * \param[in]   reg_addr: Address of the register to secure.
 * \param[in]   new_reg_value: Register value supposed to be written.
 * \PARAM[out]  reg_mask: Pointer to the mask to secure.
 * \param[out]  reg_value: Pointer to the secured register value.
 * \retval      Function execution status.
 *******************************************************************/
NODE_status_t SM_secure_register(uint8_t reg_addr, uint32_t new_reg_value, uint32_t* reg_mask, uint32_t* reg_value);

/*!******************************************************************
 * \fn NODE_status_t SM_process_register(uint8_t reg_addr, uint32_t reg_mask)
 * \brief Process SM register.
 * \param[in]   reg_addr: Address of the register to process.
 * \param[in]   reg_mask: Mask of the bits to process.
 * \param[out]  none
 * \retval      Function execution status.
 *******************************************************************/
NODE_status_t SM_process_register(uint8_t reg_addr, uint32_t reg_mask);

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

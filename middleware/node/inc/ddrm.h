/*
 * ddrm.h
 *
 *  Created on: 04 jun. 2023
 *      Author: Ludo
 */

#ifndef __DDRM_H__
#define __DDRM_H__

#include "ddrm_registers.h"
#include "node_status.h"
#include "una.h"

#ifdef DDRM

/*** DDRM macros ***/

#define NODE_BOARD_ID               UNA_BOARD_ID_DDRM
#define NODE_REGISTER_ADDRESS_LAST  DDRM_REGISTER_ADDRESS_LAST
#define NODE_REGISTER               DDRM_REGISTER

#define NODE_INIT                   DDRM_init
#define NODE_INIT_REGISTER          DDRM_init_register
#define NODE_SECURE_REGISTER        DDRM_secure_register
#define NODE_PROCESS_REGISTER       DDRM_process_register
#define NODE_REFRESH_REGISTER       DDRM_refresh_register
#define NODE_MTRG_CALLBACK          DDRM_mtrg_callback

/*** DDRM functions ***/

/*!******************************************************************
 * \fn NODE_status_t DDRM_init(void)
 * \brief Init DDRM driver.
 * \param[in]   none
 * \param[out]  none
 * \retval      Function execution status.
 *******************************************************************/
NODE_status_t DDRM_init(void);

/*!******************************************************************
 * \fn void DDRM_init_register(uint8_t reg_addr, uint32_t* reg_value)
 * \brief Get DDRM register initial value.
 * \param[in]   reg_addr: Address of the register to initialize.
 * \param[out]  reg_value: Pointer to the initial register value.
 * \retval      none
 *******************************************************************/
void DDRM_init_register(uint8_t reg_addr, uint32_t* reg_value);

/*!******************************************************************
 * \fn void DDRM_refresh_register(uint8_t reg_addr)
 * \brief Refresh DDRM register.
 * \param[in]   reg_addr: Address of the register to update.
 * \param[out]  none
 * \retval      none
 *******************************************************************/
void DDRM_refresh_register(uint8_t reg_addr);

/*!******************************************************************
 * \fn NODE_status_t DDRM_secure_register(uint8_t reg_addr, uint32_t new_reg_value, uint32_t* reg_mask, uint32_t* reg_value)
 * \brief Secure DDRM register.
 * \param[in]   reg_addr: Address of the register to secure.
 * \param[in]   new_reg_value: Register value supposed to be written.
 * \PARAM[out]  reg_mask: Pointer to the mask to secure.
 * \param[out]  reg_value: Pointer to the secured register value.
 * \retval      Function execution status.
 *******************************************************************/
NODE_status_t DDRM_secure_register(uint8_t reg_addr, uint32_t new_reg_value, uint32_t* reg_mask, uint32_t* reg_value);

/*!******************************************************************
 * \fn NODE_status_t DDRM_process_register(uint8_t reg_addr, uint32_t reg_mask)
 * \brief Process DDRM register.
 * \param[in]   reg_addr: Address of the register to process.
 * \param[in]   reg_mask: Mask of the bits to process.
 * \param[out]  none
 * \retval      Function execution status.
 *******************************************************************/
NODE_status_t DDRM_process_register(uint8_t reg_addr, uint32_t reg_mask);

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

/*
 * mpmcm.h
 *
 *  Created on: 03 sep. 2023
 *      Author: Ludo
 */

#ifndef __MPMCM_H__
#define __MPMCM_H__

#include "mpmcm_registers.h"
#include "node_status.h"
#include "una.h"

#ifdef MPMCM

/*** MPMCM macros ***/

#define NODE_BOARD_ID               UNA_BOARD_ID_MPMCM
#define NODE_REGISTER_ADDRESS_LAST  MPMCM_REGISTER_ADDRESS_LAST
#define NODE_REGISTER               MPMCM_REGISTER

#define NODE_INIT                   MPMCM_init
#define NODE_INIT_REGISTER          MPMCM_init_register
#define NODE_SECURE_REGISTER        MPMCM_secure_register
#define NODE_PROCESS_REGISTER       MPMCM_process_register
#define NODE_REFRESH_REGISTER       MPMCM_refresh_register
#define NODE_MTRG_CALLBACK          MPMCM_mtrg_callback

/*** MPMCM functions ***/

/*!******************************************************************
 * \fn NODE_status_t MPMCM_init(void)
 * \brief Init MPMCM driver.
 * \param[in]   none
 * \param[out]  none
 * \retval      Function execution status.
 *******************************************************************/
NODE_status_t MPMCM_init(void);

/*!******************************************************************
 * \fn void MPMCM_init_register(uint8_t reg_addr, uint32_t* reg_value)
 * \brief Get MPMCM register initial value.
 * \param[in]   reg_addr: Address of the register to initialize.
 * \param[out]  reg_value: Pointer to the initial register value.
 * \retval      none
 *******************************************************************/
void MPMCM_init_register(uint8_t reg_addr, uint32_t* reg_value);

/*!******************************************************************
 * \fn void MPMCM_refresh_register(uint8_t reg_addr)
 * \brief Refresh MPMCM register.
 * \param[in]   reg_addr: Address of the register to update.
 * \param[out]  none
 * \retval      none
 *******************************************************************/
void MPMCM_refresh_register(uint8_t reg_addr);

/*!******************************************************************
 * \fn NODE_status_t MPMCM_secure_register(uint8_t reg_addr, uint32_t new_reg_value, uint32_t* reg_mask, uint32_t* reg_value)
 * \brief Secure MPMCM register.
 * \param[in]   reg_addr: Address of the register to secure.
 * \param[in]   new_reg_value: Register value supposed to be written.
 * \PARAM[out]  reg_mask: Pointer to the mask to secure.
 * \param[out]  reg_value: Pointer to the secured register value.
 * \retval      Function execution status.
 *******************************************************************/
NODE_status_t MPMCM_secure_register(uint8_t reg_addr, uint32_t new_reg_value, uint32_t* reg_mask, uint32_t* reg_value);

/*!******************************************************************
 * \fn NODE_status_t MPMCM_process_register(uint8_t reg_addr, uint32_t reg_mask)
 * \brief Process MPMCM register.
 * \param[in]   reg_addr: Address of the register to process.
 * \param[in]   reg_mask: Mask of the bits to process.
 * \param[out]  none
 * \retval      Function execution status.
 *******************************************************************/
NODE_status_t MPMCM_process_register(uint8_t reg_addr, uint32_t reg_mask);

/*!******************************************************************
 * \fn NODE_status_t MPMCM_mtrg_callback(void)
 * \brief MPMCM measurements callback.
 * \param[in]   none
 * \param[out]  none
 * \retval      Function execution status.
 *******************************************************************/
NODE_status_t MPMCM_mtrg_callback(void);

#endif /* MPMCM */

#endif /* __MPMCM_H__ */

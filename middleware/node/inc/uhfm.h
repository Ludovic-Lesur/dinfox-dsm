/*
 * uhfm.h
 *
 *  Created on: 04 jun. 2023
 *      Author: Ludo
 */

#ifndef __UHFM_H__
#define __UHFM_H__

#include "adc.h"
#include "node_status.h"
#include "uhfm_registers.h"
#include "una.h"

#ifdef UHFM

/*** UHFM macros ***/

#define NODE_BOARD_ID               UNA_BOARD_ID_UHFM
#define NODE_REGISTER_ADDRESS_LAST  UHFM_REGISTER_ADDRESS_LAST
#define NODE_REGISTER               UHFM_REGISTER

#define NODE_INIT                   UHFM_init
#define NODE_INIT_REGISTER          UHFM_init_register
#define NODE_SECURE_REGISTER        UHFM_secure_register
#define NODE_PROCESS_REGISTER       UHFM_process_register
#define NODE_REFRESH_REGISTER       UHFM_refresh_register
#define NODE_MTRG_CALLBACK          UHFM_mtrg_callback

/*** UHFM functions ***/

/*!******************************************************************
 * \fn NODE_status_t UHFM_init(void)
 * \brief Init UHFM driver.
 * \param[in]   none
 * \param[out]  none
 * \retval      Function execution status.
 *******************************************************************/
NODE_status_t UHFM_init(void);

/*!******************************************************************
 * \fn void UHFM_init_register(uint8_t reg_addr, uint32_t* reg_value)
 * \brief Get UHFM register initial value.
 * \param[in]   reg_addr: Address of the register to initialize.
 * \param[out]  reg_value: Pointer to the initial register value.
 * \retval      none
 *******************************************************************/
NODE_status_t UHFM_init_register(uint8_t reg_addr, uint32_t* reg_value);

/*!******************************************************************
 * \fn NODE_status_t UHFM_refresh_register(uint8_t reg_addr)
 * \brief Refresh UHFM register.
 * \param[in]   reg_addr: Address of the register to update.
 * \param[out]  none
 * \retval      Function execution status.
 *******************************************************************/
NODE_status_t UHFM_refresh_register(uint8_t reg_addr);

/*!******************************************************************
 * \fn NODE_status_t UHFM_secure_register(uint8_t reg_addr, uint32_t new_reg_value, uint32_t* reg_mask, uint32_t* reg_value)
 * \brief Secure UHFM register.
 * \param[in]   reg_addr: Address of the register to secure.
 * \param[in]   new_reg_value: Register value supposed to be written.
 * \PARAM[out]  reg_mask: Pointer to the mask to secure.
 * \param[out]  reg_value: Pointer to the secured register value.
 * \retval      Function execution status.
 *******************************************************************/
NODE_status_t UHFM_secure_register(uint8_t reg_addr, uint32_t new_reg_value, uint32_t* reg_mask, uint32_t* reg_value);

/*!******************************************************************
 * \fn NODE_status_t UHFM_process_register(uint8_t reg_addr, uint32_t reg_mask)
 * \brief Process UHFM register.
 * \param[in]   reg_addr: Address of the register to process..
 * \param[in]   reg_mask: Mask of the bits to process.
 * \param[out]  none
 * \retval      Function execution status.
 *******************************************************************/
NODE_status_t UHFM_process_register(uint8_t reg_addr, uint32_t reg_mask);

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

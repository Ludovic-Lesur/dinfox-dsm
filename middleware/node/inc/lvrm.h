/*
 * lvrm.h
 *
 *  Created on: 04 jun. 2023
 *      Author: Ludo
 */

#ifndef __LVRM_H__
#define __LVRM_H__

#include "lvrm_registers.h"
#include "node_status.h"
#include "una.h"

#ifdef LVRM

/*** LVRM macros ***/

#define NODE_BOARD_ID               UNA_BOARD_ID_LVRM
#define NODE_REGISTER_ADDRESS_LAST  LVRM_REGISTER_ADDRESS_LAST
#define NODE_REGISTER               LVRM_REGISTER

#define NODE_INIT                   LVRM_init
#define NODE_INIT_REGISTER          LVRM_init_register
#define NODE_SECURE_REGISTER        LVRM_secure_register
#define NODE_PROCESS_REGISTER       LVRM_process_register
#define NODE_REFRESH_REGISTER       LVRM_refresh_register
#define NODE_MTRG_CALLBACK          LVRM_mtrg_callback

/*** LVRM functions ***/

/*!******************************************************************
 * \fn NODE_status_t LVRM_init(void)
 * \brief Init LVRM driver.
 * \param[in]   none
 * \param[out]  none
 * \retval      Function execution status.
 *******************************************************************/
NODE_status_t LVRM_init(void);

/*!******************************************************************
 * \fn void LVRM_init_register(uint8_t reg_addr, uint32_t* reg_value)
 * \brief Get LVRM register initial value.
 * \param[in]   reg_addr: Address of the register to initialize.
 * \param[out]  reg_value: Pointer to the initial register value.
 * \retval      none
 *******************************************************************/
void LVRM_init_register(uint8_t reg_addr, uint32_t* reg_value);

/*!******************************************************************
 * \fn void LVRM_refresh_register(uint8_t reg_addr)
 * \brief Refresh LVRM register.
 * \param[in]   reg_addr: Address of the register to update.
 * \param[out]  none
 * \retval      none
 *******************************************************************/
void LVRM_refresh_register(uint8_t reg_addr);

/*!******************************************************************
 * \fn NODE_status_t LVRM_secure_register(uint8_t reg_addr, uint32_t new_reg_value, uint32_t* reg_mask, uint32_t* reg_value)
 * \brief Secure LVRM register.
 * \param[in]   reg_addr: Address of the register to secure.
 * \param[in]   new_reg_value: Register value supposed to be written.
 * \PARAM[out]  reg_mask: Pointer to the mask to secure.
 * \param[out]  reg_value: Pointer to the secured register value.
 * \retval      Function execution status.
 *******************************************************************/
NODE_status_t LVRM_secure_register(uint8_t reg_addr, uint32_t new_reg_value, uint32_t* reg_mask, uint32_t* reg_value);

/*!******************************************************************
 * \fn NODE_status_t LVRM_process_register(uint8_t reg_addr, uint32_t reg_mask)
 * \brief Process LVRM register.
 * \param[in]   reg_addr: Address of the register to process.
 * \param[in]   reg_mask: Mask of the bits to process.
 * \param[out]  none
 * \retval      Function execution status.
 *******************************************************************/
NODE_status_t LVRM_process_register(uint8_t reg_addr, uint32_t reg_mask);

/*!******************************************************************
 * \fn NODE_status_t LVRM_mtrg_callback(void)
 * \brief LVRM measurements callback.
 * \param[in]   none
 * \param[out]  none
 * \retval      Function execution status.
 *******************************************************************/
NODE_status_t LVRM_mtrg_callback(void);

#ifdef LVRM_MODE_BMS
/*!******************************************************************
 * \fn NODE_status_t LVRM_bms_process(UNA_node_address_t lvrm_node_addr)
 * \brief BMS function.
 * \param[in]   none
 * \param[out]  none
 * \retval      Function execution status.
 *******************************************************************/
NODE_status_t LVRM_bms_process(void);
#endif

#endif /* LVRM */

#endif /* __LVRM_H__ */

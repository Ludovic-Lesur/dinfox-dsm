/*
 * gpsm.h
 *
 *  Created on: 8 jun. 2023
 *      Author: Ludo
 */

#ifndef __GPSM_H__
#define __GPSM_H__

#include "gpsm_registers.h"
#include "node_status.h"
#include "una.h"

#ifdef GPSM

/*** GPSM macros ***/

#define NODE_BOARD_ID               UNA_BOARD_ID_GPSM
#define NODE_REGISTER_ADDRESS_LAST  GPSM_REGISTER_ADDRESS_LAST
#define NODE_REGISTER               GPSM_REGISTER

#define NODE_INIT                   GPSM_init
#define NODE_INIT_REGISTER          GPSM_init_register
#define NODE_SECURE_REGISTER        GPSM_secure_register
#define NODE_PROCESS_REGISTER       GPSM_process_register
#define NODE_REFRESH_REGISTER       GPSM_refresh_register
#define NODE_MTRG_CALLBACK          GPSM_mtrg_callback

/*** GPSM functions ***/

/*!******************************************************************
 * \fn NODE_status_t GPSM_init(void)
 * \brief Init GPSM driver.
 * \param[in]   none
 * \param[out]  none
 * \retval      Function execution status.
 *******************************************************************/
NODE_status_t GPSM_init(void);

/*!******************************************************************
 * \fn void GPSM_init_register(uint8_t reg_addr, uint32_t* reg_value)
 * \brief Get GPSM register initial value.
 * \param[in]   reg_addr: Address of the register to initialize.
 * \param[out]  reg_value: Pointer to the initial register value.
 * \retval      none
 *******************************************************************/
void GPSM_init_register(uint8_t reg_addr, uint32_t* reg_value);

/*!******************************************************************
 * \fn void GPSM_refresh_register(uint8_t reg_addr)
 * \brief Refresh GPSM register.
 * \param[in]   reg_addr: Address of the register to update.
 * \param[out]  none
 * \retval      none
 *******************************************************************/
void GPSM_refresh_register(uint8_t reg_addr);

/*!******************************************************************
 * \fn NODE_status_t GPSM_secure_register(uint8_t reg_addr, uint32_t new_reg_value, uint32_t* reg_mask, uint32_t* reg_value)
 * \brief Secure GPSM register.
 * \param[in]   reg_addr: Address of the register to secure.
 * \param[in]   new_reg_value: Register value supposed to be written.
 * \PARAM[out]  reg_mask: Pointer to the mask to secure.
 * \param[out]  reg_value: Pointer to the secured register value.
 * \retval      Function execution status.
 *******************************************************************/
NODE_status_t GPSM_secure_register(uint8_t reg_addr, uint32_t new_reg_value, uint32_t* reg_mask, uint32_t* reg_value);

/*!******************************************************************
 * \fn NODE_status_t GPSM_process_register(uint8_t reg_addr, uint32_t reg_mask)
 * \brief Process GPSM register.
 * \param[in]   reg_addr: Address of the register to process.
 * \param[in]   reg_mask: Mask of the bits to process.
 * \param[out]  none
 * \retval      Function execution status.
 *******************************************************************/
NODE_status_t GPSM_process_register(uint8_t reg_addr, uint32_t reg_mask);

/*!******************************************************************
 * \fn NODE_status_t GPSM_mtrg_callback(void)
 * \brief GPSM measurements callback.
 * \param[in]   none
 * \param[out]  none
 * \retval      Function execution status.
 *******************************************************************/
NODE_status_t GPSM_mtrg_callback(void);

#endif /* GPSM */

#endif /* __GPSM_H__ */

/*
 * bpsm.h
 *
 *  Created on: 04 jun. 2023
 *      Author: Ludo
 */

#ifndef __BPSM_H__
#define __BPSM_H__

#include "bpsm_registers.h"
#include "node_status.h"
#include "una.h"

#ifdef BPSM

/*** BPSM macros ***/

#define NODE_BOARD_ID               UNA_BOARD_ID_BPSM
#define NODE_REGISTER_ADDRESS_LAST  BPSM_REGISTER_ADDRESS_LAST
#define NODE_REGISTER               BPSM_REGISTER

#define NODE_INIT                   BPSM_init
#define NODE_INIT_REGISTER          BPSM_init_register
#define NODE_SECURE_REGISTER        BPSM_secure_register
#define NODE_PROCESS_REGISTER       BPSM_process_register
#define NODE_REFRESH_REGISTER       BPSM_refresh_register
#define NODE_MTRG_CALLBACK          BPSM_mtrg_callback

/*** BPSM functions ***/

/*!******************************************************************
 * \fn NODE_status_t BPSM_init(void)
 * \brief Init BPSM driver.
 * \param[in]   none
 * \param[out]  none
 * \retval      Function execution status.
 *******************************************************************/
NODE_status_t BPSM_init(void);

/*!******************************************************************
 * \fn void BPSM_init_register(uint8_t reg_addr, uint32_t* reg_value)
 * \brief Get BPSM register initial value.
 * \param[in]   reg_addr: Address of the register to initialize.
 * \param[out]  reg_value: Pointer to the initial register value.
 * \retval      none
 *******************************************************************/
void BPSM_init_register(uint8_t reg_addr, uint32_t* reg_value);

/*!******************************************************************
 * \fn void BPSM_refresh_register(uint8_t reg_addr)
 * \brief Refresh BPSM register.
 * \param[in]   reg_addr: Address of the register to update.
 * \param[out]  none
 * \retval      none
 *******************************************************************/
void BPSM_refresh_register(uint8_t reg_addr);

/*!******************************************************************
 * \fn NODE_status_t BPSM_secure_register(uint8_t reg_addr, uint32_t new_reg_value, uint32_t* reg_mask, uint32_t* reg_value)
 * \brief Secure BPSM register.
 * \param[in]   reg_addr: Address of the register to secure.
 * \param[in]   new_reg_value: Register value supposed to be written.
 * \PARAM[out]  reg_mask: Pointer to the mask to secure.
 * \param[out]  reg_value: Pointer to the secured register value.
 * \retval      Function execution status.
 *******************************************************************/
NODE_status_t BPSM_secure_register(uint8_t reg_addr, uint32_t new_reg_value, uint32_t* reg_mask, uint32_t* reg_value);

/*!******************************************************************
 * \fn NODE_status_t BPSM_process_register(uint8_t reg_addr, uint32_t reg_mask)
 * \brief Process BPSM register.
 * \param[in]   reg_addr: Address of the register to process.
 * \param[in]   reg_mask: Mask of the bits to process.
 * \param[out]  none
 * \retval      Function execution status.
 *******************************************************************/
NODE_status_t BPSM_process_register(uint8_t reg_addr, uint32_t reg_mask);

/*!******************************************************************
 * \fn NODE_status_t BPSM_mtrg_callback(void)
 * \brief BPSM measurements callback.BPSM
 * \param[in]   none
 * \param[out]  none
 * \retval      Function execution status.
 *******************************************************************/
NODE_status_t BPSM_mtrg_callback(void);

/*!******************************************************************
 * \fn NODE_status_t BPSM_low_voltage_detector_process(void)
 * \brief BPSM low voltage detector process.
 * \param[in]   none
 * \param[out]  none
 * \retval      Function execution status.
 *******************************************************************/
NODE_status_t BPSM_low_voltage_detector_process(void);

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

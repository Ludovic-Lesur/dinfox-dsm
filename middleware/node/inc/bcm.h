/*
 * bcm.h
 *
 *  Created on: 27 mar. 2025
 *      Author: Ludo
 */

#ifndef __BCM_H__
#define __BCM_H__

#include "bcm_registers.h"
#include "node_status.h"
#include "una.h"

#ifdef BCM

/*** BCM macros ***/

#define NODE_BOARD_ID               UNA_BOARD_ID_BCM
#define NODE_REGISTER_ADDRESS_LAST  BCM_REGISTER_ADDRESS_LAST
#define NODE_REGISTER               BCM_REGISTER

#define NODE_INIT                   BCM_init
#define NODE_INIT_REGISTER          BCM_init_register
#define NODE_SECURE_REGISTER        BCM_secure_register
#define NODE_PROCESS_REGISTER       BCM_process_register
#define NODE_REFRESH_REGISTER       BCM_refresh_register
#define NODE_MTRG_CALLBACK          BCM_mtrg_callback

/*** BCM functions ***/

/*!******************************************************************
 * \fn NODE_status_t BCM_init(void)
 * \brief Init BCM driver.
 * \param[in]   none
 * \param[out]  none
 * \retval      Function execution status.
 *******************************************************************/
NODE_status_t BCM_init(void);

/*!******************************************************************
 * \fn void BCM_init_register(uint8_t reg_addr, uint32_t* reg_value)
 * \brief Get BCM register initial value.
 * \param[in]   reg_addr: Address of the register to initialize.
 * \param[out]  reg_value: Pointer to the initial register value.
 * \retval      none
 *******************************************************************/
void BCM_init_register(uint8_t reg_addr, uint32_t* reg_value);

/*!******************************************************************
 * \fn void BCM_refresh_register(uint8_t reg_addr)
 * \brief Refresh BCM register.
 * \param[in]   reg_addr: Address of the register to update.
 * \param[out]  none
 * \retval      none
 *******************************************************************/
void BCM_refresh_register(uint8_t reg_addr);

/*!******************************************************************
 * \fn NODE_status_t BCM_secure_register(uint8_t reg_addr, uint32_t new_reg_value, uint32_t* reg_mask, uint32_t* reg_value)
 * \brief Secure BCM register.
 * \param[in]   reg_addr: Address of the register to secure.
 * \param[in]   new_reg_value: Register value supposed to be written.
 * \PARAM[out]  reg_mask: Pointer to the mask to secure.
 * \param[out]  reg_value: Pointer to the secured register value.
 * \retval      Function execution status.
 *******************************************************************/
NODE_status_t BCM_secure_register(uint8_t reg_addr, uint32_t new_reg_value, uint32_t* reg_mask, uint32_t* reg_value);

/*!******************************************************************
 * \fn NODE_status_t BCM_process_register(uint8_t reg_addr, uint32_t reg_mask)
 * \brief Process BCM register.
 * \param[in]   reg_addr: Address of the register to process.
 * \param[in]   reg_mask: Mask of the bits to process.
 * \param[out]  none
 * \retval      Function execution status.
 *******************************************************************/
NODE_status_t BCM_process_register(uint8_t reg_addr, uint32_t reg_mask);

/*!******************************************************************
 * \fn NODE_status_t BCM_mtrg_callback(void)
 * \brief BCM measurements callback.
 * \param[in]   none
 * \param[out]  none
 * \retval      Function execution status.
 *******************************************************************/
NODE_status_t BCM_mtrg_callback(void);

/*!******************************************************************
 * \fn NODE_status_t BCM_low_voltage_detector_process(void)
 * \brief BCM low voltage detector process.
 * \param[in]   none
 * \param[out]  none
 * \retval      Function execution status.
 *******************************************************************/
NODE_status_t BCM_low_voltage_detector_process(void);

#ifndef BCM_CHEN_FORCED_HARDWARE
/*!******************************************************************
 * \fn NODE_status_t BCM_charge_process(void)
 * \brief BCM automatic charge control process.
 * \param[in]   none
 * \param[out]  none
 * \retval      Function execution status.
 *******************************************************************/
NODE_status_t BCM_charge_process(void);
#endif

#endif /* BCM */

#endif /* __BCM_H__ */

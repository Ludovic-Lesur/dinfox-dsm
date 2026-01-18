/*
 * common.h
 *
 *  Created on: 04 jun. 2023
 *      Author: Ludo
 */

#ifndef __COMMON_H__
#define __COMMON_H__

#include "node_status.h"
#include "types.h"

/*** COMMON functions ***/

/*!******************************************************************
 * \fn void COMMON_init_register(uint8_t reg_addr, uint32_t* reg_value)
 * \brief Get common register initial value.
 * \param[in]   reg_addr: Address of the register to initialize.
 * \param[out]  reg_value: Pointer to the initial register value.
 * \retval      none
 *******************************************************************/
void COMMON_init_register(uint8_t reg_addr, uint32_t* reg_value);

/*!******************************************************************
 * \fn void COMMON_refresh_register(uint8_t reg_addr)
 * \brief Update common register.
 * \param[in]   reg_addr: Address of the register to update.
 * \param[out]  none
 * \retval      none
 *******************************************************************/
void COMMON_refresh_register(uint8_t reg_addr);

/*!******************************************************************
 * \fn NODE_status_t COMMON_secure_register(uint8_t reg_addr, uint32_t new_reg_value, uint32_t* reg_mask, uint32_t* reg_value)
 * \brief Secure COMMON register.
 * \param[in]   reg_addr: Address of the register to secure.
 * \param[in]   new_reg_value: Register value supposed to be written.
 * \PARAM[out]  reg_mask: Pointer to the mask to secure.
 * \param[out]  reg_value: Pointer to the secured register value.
 * \retval      Function execution status.
 *******************************************************************/
NODE_status_t COMMON_secure_register(uint8_t reg_addr, uint32_t new_reg_value, uint32_t* reg_mask, uint32_t* reg_value);

/*!******************************************************************
 * \fn NODE_status_t COMMON_process_register(uint8_t reg_addr, uint32_t reg_mask)
 * \brief Process common register.
 * \param[in]   reg_addr: Address of the register to check.
 * \param[in]   reg_mask: Mask of the bits to check.
 * \param[out]  none
 * \retval      Function execution status.
 *******************************************************************/
NODE_status_t COMMON_process_register(uint8_t reg_addr, uint32_t reg_mask);

#endif /* __COMMON_H__ */

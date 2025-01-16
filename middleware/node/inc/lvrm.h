/*
 * lvrm.h
 *
 *  Created on: 04 jun. 2023
 *      Author: Ludo
 */

#ifndef __LVRM_H__
#define __LVRM_H__

#include "lvrm_registers.h"
#include "node.h"
#include "una.h"

#ifdef LVRM

/*** LVRM macros ***/

#define NODE_BOARD_ID               UNA_BOARD_ID_LVRM
#define NODE_REGISTER_ADDRESS_LAST  LVRM_REGISTER_ADDRESS_LAST
#define NODE_REGISTER_ACCESS        LVRM_REGISTER_ACCESS

/*** LVRM functions ***/

/*!******************************************************************
 * \fn NODE_status_t LVRM_init_registers(void)
 * \brief Init LVRM registers to their default value.
 * \param[in]  	none
 * \param[out] 	none
 * \retval		Function execution status.
 *******************************************************************/
NODE_status_t LVRM_init_registers(void);

/*!******************************************************************
 * \fn NODE_status_t LVRM_update_register(uint8_t reg_addr)
 * \brief Update LVRM register.
 * \param[in]  	reg_addr: Address of the register to update.
 * \param[out] 	none
 * \retval		Function execution status.
 *******************************************************************/
NODE_status_t LVRM_update_register(uint8_t reg_addr);

/*!******************************************************************
 * \fn NODE_status_t LVRM_check_register(uint8_t reg_addr)
 * \brief Check LVRM register.
 * \param[in]  	reg_addr: Address of the register to check.
 * \param[in]	reg_mask: Mask of the bits to check.
 * \param[out] 	none
 * \retval		Function execution status.
 *******************************************************************/
NODE_status_t LVRM_check_register(uint8_t reg_addr, uint32_t reg_mask);

/*!******************************************************************
 * \fn NODE_status_t LVRM_mtrg_callback(void)
 * \brief LVRM measurements callback.
 * \param[in]  	none
 * \param[out] 	none
 * \retval		Function execution status.
 *******************************************************************/
NODE_status_t LVRM_mtrg_callback(void);

#ifdef LVRM_MODE_BMS
/*!******************************************************************
 * \fn NODE_status_t LVRM_bms_process(UNA_node_address_t lvrm_node_addr)
 * \brief BMS function.
 * \param[in]  	none
 * \param[out] 	none
 * \retval		Function execution status.
 *******************************************************************/
NODE_status_t LVRM_bms_process(void);
#endif

#endif /* LVRM */

#endif /* __LVRM_H__ */

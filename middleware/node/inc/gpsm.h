/*
 * gpsm.h
 *
 *  Created on: 8 jun. 2023
 *      Author: Ludo
 */

#ifndef __GPSM_H__
#define __GPSM_H__

#include "gpsm_registers.h"
#include "node.h"
#include "una.h"

#ifdef GPSM

/*** GPSM macros ***/

#define NODE_BOARD_ID		        UNA_BOARD_ID_GPSM
#define NODE_REGISTER_ADDRESS_LAST	GPSM_REGISTER_ADDRESS_LAST
#define NODE_REGISTER_ACCESS        GPSM_REGISTER_ACCESS

/*** GPSM functions ***/

/*!******************************************************************
 * \fn NODE_status_t GPSM_init_registers(void)
 * \brief Init GPSM registers to their default value.
 * \param[in]  	none
 * \param[out] 	none
 * \retval		Function execution status.
 *******************************************************************/
NODE_status_t GPSM_init_registers(void);

/*!******************************************************************
 * \fn NODE_status_t GPSM_update_register(uint8_t reg_addr)
 * \brief Update GPSM register.
 * \param[in]  	reg_addr: Address of the register to update.
 * \param[out] 	none
 * \retval		Function execution status.
 *******************************************************************/
NODE_status_t GPSM_update_register(uint8_t reg_addr);

/*!******************************************************************
 * \fn NODE_status_t GPSM_check_register(uint8_t reg_addr)
 * \brief Check GPSM register.
 * \param[in]  	reg_addr: Address of the register to check.
 * \param[in]	reg_mask: Mask of the bits to check.
 * \param[out] 	none
 * \retval		Function execution status.
 *******************************************************************/
NODE_status_t GPSM_check_register(uint8_t reg_addr, uint32_t reg_mask);

/*!******************************************************************
 * \fn NODE_status_t GPSM_mtrg_callback(void)
 * \brief GPSM measurements callback.
 * \param[in]  	none
 * \param[out] 	none
 * \retval		Function execution status.
 *******************************************************************/
NODE_status_t GPSM_mtrg_callback(void);

#endif /* GPSM */

#endif /* __GPSM_H__ */

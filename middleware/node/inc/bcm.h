/*
 * bcm.h
 *
 *  Created on: 27 mar. 2025
 *      Author: Ludo
 */

#ifndef __BCM_H__
#define __BCM_H__

#include "bcm_registers.h"
#include "node.h"
#include "una.h"

#ifdef BCM

/*** BCM macros ***/

#define NODE_BOARD_ID               UNA_BOARD_ID_BCM
#define NODE_REGISTER_ADDRESS_LAST  BCM_REGISTER_ADDRESS_LAST
#define NODE_REGISTER_ACCESS        BCM_REGISTER_ACCESS
#define NODE_REGISTER_ERROR_VALUE   BCM_REGISTER_ERROR_VALUE

/*** BCM functions ***/

/*!******************************************************************
 * \fn NODE_status_t BCM_init_registers(void)
 * \brief Init BCM registers to their default value.
 * \param[in]   none
 * \param[out]  none
 * \retval      Function execution status.
 *******************************************************************/
NODE_status_t BCM_init_registers(void);

/*!******************************************************************
 * \fn NODE_status_t BCM_update_register(uint8_t reg_addr)
 * \brief Update BCM register.
 * \param[in]   reg_addr: Address of the register to update.
 * \param[out]  none
 * \retval      Function execution status.
 *******************************************************************/
NODE_status_t BCM_update_register(uint8_t reg_addr);

/*!******************************************************************
 * \fn NODE_status_t BCM_check_register(uint8_t reg_addr)
 * \brief Check BCM register.
 * \param[in]   reg_addr: Address of the register to check.
 * \param[in]   reg_mask: Mask of the bits to check.
 * \param[out]  none
 * \retval      Function execution status.
 *******************************************************************/
NODE_status_t BCM_check_register(uint8_t reg_addr, uint32_t reg_mask);

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

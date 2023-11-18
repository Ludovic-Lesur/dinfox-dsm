/*
 * rrm.h
 *
 *  Created on: 10 jun. 2023
 *      Author: Ludo
 */

#ifndef __RRM_H__
#define __RRM_H__

#include "common.h"
#include "common_reg.h"
#include "dinfox.h"
#include "node.h"
#include "rrm_reg.h"

/*** RRM macros ***/

#ifdef RRM
#define NODE_BOARD_ID		DINFOX_BOARD_ID_RRM
#define NODE_REG_ADDR_LAST	RRM_REG_ADDR_LAST
#endif

/*** RRM global variables ***/

#ifdef RRM
static const DINFOX_register_access_t NODE_REG_ACCESS[RRM_REG_ADDR_LAST] = {
	COMMON_REG_ACCESS
	DINFOX_REG_ACCESS_READ_ONLY,
	DINFOX_REG_ACCESS_READ_WRITE,
	DINFOX_REG_ACCESS_READ_ONLY,
	DINFOX_REG_ACCESS_READ_ONLY
};
#endif

/*** RRM functions ***/

#ifdef RRM
/*!******************************************************************
 * \fn void RRM_init_registers(void)
 * \brief Init RRM registers to their default value.
 * \param[in]  	none
 * \param[out] 	none
 * \retval		none
 *******************************************************************/
void RRM_init_registers(void);
#endif

#ifdef RRM
/*!******************************************************************
 * \fn NODE_status_t RRM_update_register(uint8_t reg_addr)
 * \brief Update RRM register.
 * \param[in]  	reg_addr: Address of the register to update.
 * \param[out] 	none
 * \retval		Function execution status.
 *******************************************************************/
NODE_status_t RRM_update_register(uint8_t reg_addr);
#endif

#ifdef RRM
/*!******************************************************************
 * \fn NODE_status_t RRM_check_register(uint8_t reg_addr)
 * \brief Check RRM register.
 * \param[in]  	reg_addr: Address of the register to check.
 * \param[in]	reg_mask: Mask of the bits to check.
 * \param[out] 	none
 * \retval		Function execution status.
 *******************************************************************/
NODE_status_t RRM_check_register(uint8_t reg_addr, uint32_t reg_mask);
#endif

#ifdef RRM
/*!******************************************************************
 * \fn NODE_status_t RRM_mtrg_callback(ADC_status_t* adc_status)
 * \brief RRM measurements callback.
 * \param[in]  	none
 * \param[out] 	adc_status: Pointer to the resulting ADC status.
 * \retval		Function execution status.
 *******************************************************************/
NODE_status_t RRM_mtrg_callback(ADC_status_t* adc_status);
#endif

#endif /* INC_NODES_RRM_H_ */

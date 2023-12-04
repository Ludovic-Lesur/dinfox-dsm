/*
 * lvrm.h
 *
 *  Created on: 04 jun. 2023
 *      Author: Ludo
 */

#ifndef __LVRM_H__
#define __LVRM_H__

#include "common.h"
#include "common_reg.h"
#include "dinfox.h"
#include "lvrm_reg.h"
#include "node.h"

/*** LVRM macros ***/

#ifdef LVRM
#define NODE_BOARD_ID		DINFOX_BOARD_ID_LVRM
#define NODE_REG_ADDR_LAST	LVRM_REG_ADDR_LAST
#endif

/*** LVRM global variables ***/

#ifdef LVRM
static const DINFOX_register_access_t NODE_REG_ACCESS[LVRM_REG_ADDR_LAST] = {
	COMMON_REG_ACCESS
	DINFOX_REG_ACCESS_READ_ONLY,
	DINFOX_REG_ACCESS_READ_ONLY,
	DINFOX_REG_ACCESS_READ_ONLY,
	DINFOX_REG_ACCESS_READ_WRITE,
	DINFOX_REG_ACCESS_READ_ONLY,
	DINFOX_REG_ACCESS_READ_ONLY
};
#endif

/*** LVRM functions ***/

#ifdef LVRM
/*!******************************************************************
 * \fn void LVRM_init_registers(void)
 * \brief Init LVRM registers to their default value.
 * \param[in]  	none
 * \param[out] 	none
 * \retval		none
 *******************************************************************/
void LVRM_init_registers(void);
#endif

#ifdef LVRM
/*!******************************************************************
 * \fn NODE_status_t LVRM_update_register(uint8_t reg_addr)
 * \brief Update LVRM register.
 * \param[in]  	reg_addr: Address of the register to update.
 * \param[out] 	none
 * \retval		Function execution status.
 *******************************************************************/
NODE_status_t LVRM_update_register(uint8_t reg_addr);
#endif

#ifdef LVRM
/*!******************************************************************
 * \fn NODE_status_t LVRM_check_register(uint8_t reg_addr)
 * \brief Check LVRM register.
 * \param[in]  	reg_addr: Address of the register to check.
 * \param[in]	reg_mask: Mask of the bits to check.
 * \param[out] 	none
 * \retval		Function execution status.
 *******************************************************************/
NODE_status_t LVRM_check_register(uint8_t reg_addr, uint32_t reg_mask);
#endif

#ifdef LVRM
/*!******************************************************************
 * \fn NODE_status_t LVRM_mtrg_callback(ADC_status_t* adc_status)
 * \brief LVRM measurements callback.
 * \param[in]  	none
 * \param[out] 	adc_status: Pointer to the resulting ADC status.
 * \retval		Function execution status.
 *******************************************************************/
NODE_status_t LVRM_mtrg_callback(ADC_status_t* adc_status);
#endif

#if (defined LVRM) && (defined LVRM_MODE_BMS)
/*!******************************************************************
 * \fn NODE_status_t LVRM_bms_process(NODE_address_t lvrm_node_addr)
 * \brief BMS function.
 * \param[in]  	none
 * \param[out] 	none
 * \retval		Function execution status.
 *******************************************************************/
NODE_status_t LVRM_bms_process(void);
#endif

#endif /* __LVRM_H__ */

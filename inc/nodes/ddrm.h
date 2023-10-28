/*
 * ddrm.h
 *
 *  Created on: 04 jun. 2023
 *      Author: Ludo
 */

#ifndef __DDRM_H__
#define __DDRM_H__

#include "ddrm_reg.h"
#include "common.h"
#include "common_reg.h"
#include "dinfox.h"
#include "node.h"

/*** DDRM macros ***/

#ifdef DDRM
#define NODE_BOARD_ID		DINFOX_BOARD_ID_DDRM
#define NODE_REG_ADDR_LAST	DDRM_REG_ADDR_LAST
#endif

/*** DDRM global variables ***/

#ifdef DDRM
static const DINFOX_register_access_t NODE_REG_ACCESS[DDRM_REG_ADDR_LAST] = {
	COMMON_REG_ACCESS
	DINFOX_REG_ACCESS_READ_WRITE,
	DINFOX_REG_ACCESS_READ_ONLY,
	DINFOX_REG_ACCESS_READ_ONLY,
	DINFOX_REG_ACCESS_READ_ONLY
};
#endif

/*** DDRM functions ***/

#ifdef DDRM
/*!******************************************************************
 * \fn void DDRM_init_registers(void)
 * \brief Init DDRM registers to their default value.
 * \param[in]  	none
 * \param[out] 	none
 * \retval		none
 *******************************************************************/
void DDRM_init_registers(void);
#endif

#ifdef DDRM
/*!******************************************************************
 * \fn NODE_status_t DDRM_update_register(uint8_t reg_addr)
 * \brief Update DDRM register.
 * \param[in]  	reg_addr: Address of the register to update.
 * \param[out] 	none
 * \retval		Function execution status.
 *******************************************************************/
NODE_status_t DDRM_update_register(uint8_t reg_addr);
#endif

#ifdef DDRM
/*!******************************************************************
 * \fn NODE_status_t DDRM_check_register(uint8_t reg_addr)
 * \brief Check DDRM register.
 * \param[in]  	reg_addr: Address of the register to check.
 * \param[in]	reg_mask: Mask of the bits to check.
 * \param[out] 	none
 * \retval		Function execution status.
 *******************************************************************/
NODE_status_t DDRM_check_register(uint8_t reg_addr, uint32_t reg_mask);
#endif

#ifdef DDRM
/*!******************************************************************
 * \fn NODE_status_t DDRM_mtrg_callback(ADC_status_t* adc_status)
 * \brief DDRM measurements callback.
 * \param[in]  	none
 * \param[out] 	adc_status: Pointer to the resulting ADC status.
 * \retval		Function execution status.
 *******************************************************************/
NODE_status_t DDRM_mtrg_callback(ADC_status_t* adc_status);
#endif

#endif /* __DDRM_H__ */

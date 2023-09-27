/*
 * bpsm.h
 *
 *  Created on: 04 jun. 2023
 *      Author: Ludo
 */

#ifndef __BPSM_H__
#define __BPSM_H__

#include "bpsm_reg.h"
#include "common.h"
#include "common_reg.h"
#include "dinfox.h"
#include "node.h"

/*** BPSM macros ***/

#ifdef BPSM
#define NODE_BOARD_ID		DINFOX_BOARD_ID_BPSM
#define NODE_REG_ADDR_LAST	BPSM_REG_ADDR_LAST
#endif

/*** BPSM global variables ***/

#ifdef BPSM
static const DINFOX_register_access_t NODE_REG_ACCESS[BPSM_REG_ADDR_LAST] = {
	COMMON_REG_ACCESS
	DINFOX_REG_ACCESS_READ_WRITE,
	DINFOX_REG_ACCESS_READ_ONLY,
	DINFOX_REG_ACCESS_READ_ONLY
};
#endif

/*** BPSM functions ***/

#ifdef BPSM
/*!******************************************************************
 * \fn void BPSM_init_registers(void)
 * \brief Init BPSM registers to their default value.
 * \param[in]  	none
 * \param[out] 	none
 * \retval		none
 *******************************************************************/
void BPSM_init_registers(void);
#endif

#ifdef BPSM
/*!******************************************************************
 * \fn NODE_status_t BPSM_update_register(uint8_t reg_addr)
 * \brief Update BPSM register.
 * \param[in]  	reg_addr: Address of the register to update.
 * \param[out] 	none
 * \retval		Function execution status.
 *******************************************************************/
NODE_status_t BPSM_update_register(uint8_t reg_addr);
#endif

#ifdef BPSM
/*!******************************************************************
 * \fn NODE_status_t BPSM_check_register(uint8_t reg_addr)
 * \brief Check BPSM register.
 * \param[in]  	reg_addr: Address of the register to check.
 * \param[out] 	none
 * \retval		Function execution status.
 *******************************************************************/
NODE_status_t BPSM_check_register(uint8_t reg_addr);
#endif

#ifdef BPSM
/*!******************************************************************
 * \fn NODE_status_t BPSM_mtrg_callback(ADC_status_t* adc_status)
 * \brief BPSM measurements callback.
 * \param[in]  	none
 * \param[out] 	adc_status: Pointer to the resulting ADC status.
 * \retval		Function execution status.
 *******************************************************************/
NODE_status_t BPSM_mtrg_callback(ADC_status_t* adc_status);
#endif

#endif /* __BPSM_H__ */

/*
 * sm.h
 *
 *  Created on: 10 jun. 2023
 *      Author: Ludo
 */

#ifndef __SM_H__
#define __SM_H__

#include "adc.h"
#include "dinfox.h"
#include "sm_reg.h"
#include "node.h"

/*** SM macros ***/

#ifdef SM
#define NODE_BOARD_ID		DINFOX_BOARD_ID_SM
#define NODE_REG_ADDR_LAST	SM_REG_ADDR_LAST
#endif

/*** SM global variables ***/

#ifdef SM
extern const DINFOX_register_access_t NODE_REG_ACCESS[SM_REG_ADDR_LAST];
#endif

/*** SM functions ***/

#ifdef SM
/*!******************************************************************
 * \fn void SM_init_registers(void)
 * \brief Init SM registers to their default value.
 * \param[in]  	none
 * \param[out] 	none
 * \retval		none
 *******************************************************************/
void SM_init_registers(void);
#endif

#ifdef SM
/*!******************************************************************
 * \fn NODE_status_t SM_update_register(uint8_t reg_addr)
 * \brief Update SM register.
 * \param[in]  	reg_addr: Address of the register to update.
 * \param[out] 	none
 * \retval		Function execution status.
 *******************************************************************/
NODE_status_t SM_update_register(uint8_t reg_addr);
#endif

#ifdef SM
/*!******************************************************************
 * \fn NODE_status_t SM_check_register(uint8_t reg_addr)
 * \brief Check SM register.
 * \param[in]  	reg_addr: Address of the register to check.
 * \param[in]	reg_mask: Mask of the bits to check.
 * \param[out] 	none
 * \retval		Function execution status.
 *******************************************************************/
NODE_status_t SM_check_register(uint8_t reg_addr, uint32_t reg_mask);
#endif

#ifdef SM
/*!******************************************************************
 * \fn NODE_status_t SM_mtrg_callback(ADC_status_t* adc_status)
 * \brief SM measurements callback.
 * \param[in]  	none
 * \param[out] 	adc_status: Pointer to the resulting ADC status.
 * \retval		Function execution status.
 *******************************************************************/
NODE_status_t SM_mtrg_callback(ADC_status_t* adc_status);
#endif

#endif /* __SM_H__ */

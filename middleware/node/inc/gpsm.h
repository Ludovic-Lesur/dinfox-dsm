/*
 * gpsm.h
 *
 *  Created on: 8 jun. 2023
 *      Author: Ludo
 */

#ifndef __GPSM_H__
#define __GPSM_H__

#include "adc.h"
#include "dinfox.h"
#include "gpsm_reg.h"
#include "node.h"

/*** GPSM macros ***/

#ifdef GPSM
#define NODE_BOARD_ID		DINFOX_BOARD_ID_GPSM
#define NODE_REG_ADDR_LAST	GPSM_REG_ADDR_LAST
#endif

/*** GPSM global variables ***/

#ifdef GPSM
extern const DINFOX_register_access_t NODE_REG_ACCESS[GPSM_REG_ADDR_LAST];
#endif

/*** GPSM functions ***/

#ifdef GPSM
/*!******************************************************************
 * \fn void GPSM_init_registers(void)
 * \brief Init GPSM registers to their default value.
 * \param[in]  	none
 * \param[out] 	none
 * \retval		none
 *******************************************************************/
void GPSM_init_registers(void);
#endif

#ifdef GPSM
/*!******************************************************************
 * \fn NODE_status_t GPSM_update_register(uint8_t reg_addr)
 * \brief Update GPSM register.
 * \param[in]  	reg_addr: Address of the register to update.
 * \param[out] 	none
 * \retval		Function execution status.
 *******************************************************************/
NODE_status_t GPSM_update_register(uint8_t reg_addr);
#endif

#ifdef GPSM
/*!******************************************************************
 * \fn NODE_status_t GPSM_check_register(uint8_t reg_addr)
 * \brief Check GPSM register.
 * \param[in]  	reg_addr: Address of the register to check.
 * \param[in]	reg_mask: Mask of the bits to check.
 * \param[out] 	none
 * \retval		Function execution status.
 *******************************************************************/
NODE_status_t GPSM_check_register(uint8_t reg_addr, uint32_t reg_mask);
#endif

#ifdef GPSM
/*!******************************************************************
 * \fn NODE_status_t GPSM_mtrg_callback(ADC_status_t* adc_status)
 * \brief GPSM measurements callback.
 * \param[in]  	none
 * \param[out] 	adc_status: Pointer to the resulting ADC status.
 * \retval		Function execution status.
 *******************************************************************/
NODE_status_t GPSM_mtrg_callback(ADC_status_t* adc_status);
#endif

#endif /* __GPSM_H__ */

/*
 * uhfm.h
 *
 *  Created on: 4 jun. 2023
 *      Author: Ludo
 */

#ifndef __UHFM_H__
#define __UHFM_H__

#include "common.h"
#include "common_reg.h"
#include "dinfox.h"
#include "node.h"
#include "uhfm_reg.h"

/*** UHFM macros ***/

#ifdef UHFM
#define NODE_BOARD_ID		DINFOX_BOARD_ID_UHFM
#define NODE_REG_ADDR_LAST	UHFM_REG_ADDR_LAST
#endif

/*** UHFM global variables ***/

#ifdef UHFM
static const DINFOX_register_access_t NODE_REG_ACCESS[UHFM_REG_ADDR_LAST] = {
	COMMON_REG_ACCESS
	DINFOX_REG_ACCESS_READ_WRITE,
	DINFOX_REG_ACCESS_READ_ONLY,
	DINFOX_REG_ACCESS_READ_ONLY,
	DINFOX_REG_ACCESS_READ_ONLY,
	DINFOX_REG_ACCESS_READ_ONLY,
	DINFOX_REG_ACCESS_READ_ONLY,
	DINFOX_REG_ACCESS_READ_ONLY,
	DINFOX_REG_ACCESS_READ_WRITE,
	DINFOX_REG_ACCESS_READ_WRITE,
	DINFOX_REG_ACCESS_READ_WRITE,
	DINFOX_REG_ACCESS_READ_WRITE,
	DINFOX_REG_ACCESS_READ_WRITE,
	DINFOX_REG_ACCESS_READ_WRITE,
	DINFOX_REG_ACCESS_READ_ONLY,
	DINFOX_REG_ACCESS_READ_ONLY,
	DINFOX_REG_ACCESS_READ_ONLY,
	DINFOX_REG_ACCESS_READ_ONLY,
	DINFOX_REG_ACCESS_READ_ONLY,
	DINFOX_REG_ACCESS_READ_ONLY,
	DINFOX_REG_ACCESS_READ_WRITE,
	DINFOX_REG_ACCESS_READ_WRITE
};
#endif

/*** UHFM functions ***/

#ifdef UHFM
/*!******************************************************************
 * \fn void UHFM_init_registers(void)
 * \brief Init UHFM registers to their default value.
 * \param[in]  	none
 * \param[out] 	none
 * \retval		none
 *******************************************************************/
void UHFM_init_registers(void);
#endif

#ifdef UHFM
/*!******************************************************************
 * \fn NODE_status_t UHFM_update_register(uint8_t reg_addr)
 * \brief Update UHFM register.
 * \param[in]  	reg_addr: Address of the register to update.
 * \param[out] 	none
 * \retval		Function execution status.
 *******************************************************************/
NODE_status_t UHFM_update_register(uint8_t reg_addr);
#endif

#ifdef UHFM
/*!******************************************************************
 * \fn NODE_status_t UHFM_check_register(uint8_t reg_addr)
 * \brief Check UHFM register.
 * \param[in]  	reg_addr: Address of the register to check.
 * \param[out] 	none
 * \retval		Function execution status.
 *******************************************************************/
NODE_status_t UHFM_check_register(uint8_t reg_addr);
#endif

#ifdef UHFM
/*!******************************************************************
 * \fn NODE_status_t UHFM_mtrg_callback(ADC_status_t* adc_status)
 * \brief UHFM measurements callback.
 * \param[in]  	none
 * \param[out] 	adc_status: Pointer to the resulting ADC status.
 * \retval		Function execution status.
 *******************************************************************/
NODE_status_t UHFM_mtrg_callback(ADC_status_t* adc_status);
#endif

#endif /* __UHFM_H__ */

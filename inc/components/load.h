/*
 * load.h
 *
 *  Created on: 17 feb. 2023
 *      Author: Ludo
 */

#ifndef __LOAD_H__
#define __LOAD_H__

#include "lptim.h"
#include "types.h"

/*** LOAD structures ***/

/*!******************************************************************
 * \enum LOAD_status_t
 * \brief LOAD driver error codes.
 *******************************************************************/
typedef enum {
	LOAD_SUCCESS = 0,
	LOAD_ERROR_STATE_UNKNOWN,
	LOAD_ERROR_BASE_LPTIM = 0x0100,
	LOAD_ERROR_BASE_LAST = (LOAD_ERROR_BASE_LPTIM + LPTIM_ERROR_BASE_LAST)
} LOAD_status_t;

/*** LOAD functions ***/

#if (defined LVRM) || (defined BPSM) || (defined DDRM) || (defined RRM)
/*!******************************************************************
 * \fn void LOAD_init(void)
 * \brief Init load interface.
 * \param[in]  	none
 * \param[out] 	none
 * \retval		none
 *******************************************************************/
void LOAD_init(void);
#endif

#if (defined LVRM) || (defined BPSM) || (defined DDRM) || (defined RRM)
/*!******************************************************************
 * \fn LOAD_status_t LOAD_set_output_state(uint8_t state)
 * \brief Set load output state.
 * \param[in]  	state: New state to set.
 * \param[out] 	none
 * \retval		Function execution status.
 *******************************************************************/
LOAD_status_t LOAD_set_output_state(uint8_t state);
#endif

#if (defined LVRM) || (defined BPSM) || (defined DDRM) || (defined RRM)
/*!******************************************************************
 * \fn LOAD_status_t LOAD_get_output_state(uint8_t* state)
 * \brief Read load output state.
 * \param[in]  	none
 * \param[out] 	state: Pointer to boolean that will contain load output state.
 * \retval		Function execution status.
 *******************************************************************/
LOAD_status_t LOAD_get_output_state(uint8_t* state);
#endif

#ifdef BPSM
/*!******************************************************************
 * \fn void LOAD_set_charge_state(uint8_t state)
 * \brief Set charge enable state.
 * \param[in]  	state: New state to set.
 * \param[out] 	none
 * \retval		none
 *******************************************************************/
void LOAD_set_charge_state(uint8_t state);
#endif

#ifdef BPSM
/*!******************************************************************
 * \fn uint8_t LOAD_get_charge_state(void)
 * \brief Read charge enable state.
 * \param[in]  	none
 * \param[out] 	none
 * \retval		Charge enable state.
 *******************************************************************/
uint8_t LOAD_get_charge_state(void);
#endif

#ifdef BPSM
/*!******************************************************************
 * \fn uint8_t LOAD_get_charge_status(void)
 * \brief Read charge status.
 * \param[in]  	none
 * \param[out] 	none
 * \retval		Charge status.
 *******************************************************************/
uint8_t LOAD_get_charge_status(void);
#endif

/*******************************************************************/
#define LOAD_check_status(error_base) { if (load_status != LOAD_SUCCESS) { status = error_base + load_status; goto errors; } }

/*******************************************************************/
#define LOAD_stack_error() { ERROR_stack_error(load_status, LOAD_SUCCESS, ERROR_BASE_LOAD); }

/*******************************************************************/
#define LOAD_print_error() { ERROR_print_error(load_status, LOAD_SUCCESS, ERROR_BASE_LOAD); }

#endif /* __LOAD_H__ */

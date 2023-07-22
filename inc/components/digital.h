/*
 * digital.h
 *
 *  Created on: 17 feb. 2023
 *      Author: Ludo
 */

#ifndef __DIGITAL_H__
#define __DIGITAL_H__

#include "lptim.h"
#include "types.h"

/*** DIGITAL structures ***/

/*!******************************************************************
 * \enum DIGITAL_status_t
 * \brief DIGITAL driver error codes.
 *******************************************************************/
typedef enum {
	DIGITAL_SUCCESS = 0,
	DIGITAL_ERROR_DATA_INDEX,
	DIGITAL_ERROR_BASE_LPTIM = 0x100,
	DIGITAL_ERROR_BASE_LAST = (DIGITAL_ERROR_BASE_LPTIM + LPTIM_ERROR_BASE_LAST)
} DIGITAL_status_t;

/*!******************************************************************
 * \enum DIGITAL_data_index_t
 * \brief Digital data indexes.
 *******************************************************************/
typedef enum {
	DIGITAL_DATA_INDEX_DIO0 = 0,
	DIGITAL_DATA_INDEX_DIO1,
	DIGITAL_DATA_INDEX_DIO2,
	DIGITAL_DATA_INDEX_DIO3,
	DIGITAL_DATA_INDEX_LAST
} DIGITAL_data_index_t;

/*** DIGITAL functions ***/

#ifdef SM
/*!******************************************************************
 * \fn void DIGITAL_init(void)
 * \brief Init DIGITAL interface.
 * \param[in]  	none
 * \param[out] 	none
 * \retval		none
 *******************************************************************/
void DIGITAL_init(void);
#endif

#ifdef SM
/*!******************************************************************
 * \fn DIGITAL_status_t DIGITAL_perform_measurements(void)
 * \brief Perform all digital channels measurement.
 * \param[in]  	none
 * \param[out] 	none
 * \retval		Function execution status.
 *******************************************************************/
DIGITAL_status_t DIGITAL_perform_measurements(void);
#endif

#ifdef SM
/*!******************************************************************
 * \fn DIGITAL_status_t DIGITAL_read(DIGITAL_data_index_t data_idx, uint8_t* state)
 * \brief Read digital data.
 * \param[in]  	data_idx: Data to read.
 * \param[out] 	state: Pointer to boolean that will contain the result.
 * \retval		Function execution status.
 *******************************************************************/
DIGITAL_status_t DIGITAL_read(DIGITAL_data_index_t data_idx, uint8_t* state);
#endif

/*******************************************************************/
#define DIGITAL_check_status(error_base) { if (digital_status != DIGITAL_SUCCESS) { status = error_base + digital_status; goto errors; } }

/*******************************************************************/
#define DIGITAL_stack_error() { ERROR_stack_error(digital_status, DIGITAL_SUCCESS, ERROR_BASE_DIGITAL); }

/*******************************************************************/
#define DIGITAL_print_error() { ERROR_print_error(digital_status, DIGITAL_SUCCESS, ERROR_BASE_DIGITAL); }

#endif /* __DIGITAL_H__ */

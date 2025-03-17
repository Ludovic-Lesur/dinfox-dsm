/*
 * digital.h
 *
 *  Created on: 12 jan. 2025
 *      Author: Ludo
 */

#ifndef __DIGITAL_H__
#define __DIGITAL_H__

#include "error.h"
#include "types.h"

/*** DIGITAL structures ***/

/*!******************************************************************
 * \enum DIGITAL_status_t
 * \brief DIGITAL driver error codes.
 *******************************************************************/
typedef enum {
    // Driver errors.
    DIGITAL_SUCCESS = 0,
    DIGITAL_ERROR_NULL_PARAMETER,
    DIGITAL_ERROR_CHANNEL,
    // Last base value.
    DIGITAL_ERROR_BASE_LAST = ERROR_BASE_STEP
} DIGITAL_status_t;

#ifdef SM

/*!******************************************************************
 * \enum DIGITAL_channel_t
 * \brief DIGITAL channels list.
 *******************************************************************/
typedef enum {
    DIGITAL_CHANNEL_DIO0 = 0,
    DIGITAL_CHANNEL_DIO1,
    DIGITAL_CHANNEL_DIO2,
    DIGITAL_CHANNEL_DIO3,
    DIGITAL_CHANNEL_LAST
} DIGITAL_channel_t;

/*** DIGITAL functions ***/

/*!******************************************************************
 * \fn DIGITAL_status_t DIGITAL_init(void)
 * \brief Init DIGITAL driver.
 * \param[in]   none
 * \param[out]  none
 * \retval      Function execution status.
 *******************************************************************/
DIGITAL_status_t DIGITAL_init(void);

/*!******************************************************************
 * \fn DIGITAL_status_t DIGITAL_de_init(void)
 * \brief Release DIGITAL driver.
 * \param[in]   none
 * \param[out]  none
 * \retval      Function execution status.
 *******************************************************************/
DIGITAL_status_t DIGITAL_de_init(void);

/*!******************************************************************
 * \fn DIGITAL_status_t DIGITAL_read_channel(DIGITAL_channel_t channel, uint8_t* state)
 * \brief Read a digital channel.
 * \param[in]   channel: Channel to read.
 * \param[out]  state: Pointer to byte that will contain the digital input state.
 * \retval      Function execution status.
 *******************************************************************/
DIGITAL_status_t DIGITAL_read_channel(DIGITAL_channel_t channel, uint8_t* state);

/*******************************************************************/
#define DIGITAL_exit_error(base) { ERROR_check_exit(digital_status, DIGITAL_SUCCESS, base) }

/*******************************************************************/
#define DIGITAL_stack_error(base) { ERROR_check_stack(digital_status, DIGITAL_SUCCESS, base) }

/*******************************************************************/
#define DIGITAL_stack_exit_error(base, code) { ERROR_check_stack_exit(digital_status, DIGITAL_SUCCESS, base, code) }

#endif /* SM */

#endif /* __DIGITAL_H__ */

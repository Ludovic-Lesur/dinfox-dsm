/*
 * usart.h
 *
 *  Created on: 15 apr. 2023
 *      Author: Ludo
 */

#ifndef __USART_H__
#define __USART_H__

#include "lptim.h"
#include "types.h"

/*** USART structures ***/

/*!******************************************************************
 * \enum USART_status_t
 * \brief USART driver error codes.
 *******************************************************************/
typedef enum {
	USART_SUCCESS = 0,
	USART_ERROR_TX_TIMEOUT,
	USART_ERROR_BASE_LPTIM = 0x0100,
	USART_ERROR_BASE_LAST = (USART_ERROR_BASE_LPTIM + LPTIM_ERROR_BASE_LAST)
} USART_status_t;

/*!******************************************************************
 * \fn USART_character_match_irq_cb
 * \brief USART character match interrupt callback.
 *******************************************************************/
typedef void (*USART_character_match_irq_cb)(uint8_t line_end_flag);

/*** USART functions ***/

#ifdef GPSM
/*!******************************************************************
 * \fn void USART2_init(void)
 * \brief Init USART2 peripheral.
 * \param[in]  	none
 * \param[out] 	none
 * \retval		none
 *******************************************************************/
void USART2_init(void);
#endif

#ifdef GPSM
/*!******************************************************************
 * \fn void USART2_set_character_match_callback(USART_character_match_irq_cb irq_callback
 * \brief Set USART character match interrupt callback.
 * \param[in]  	irq_callback: Function to call on interrupt.
 * \param[out] 	none
 * \retval		none
 *******************************************************************/
void USART2_set_character_match_callback(USART_character_match_irq_cb irq_callback);
#endif

#ifdef GPSM
/*!******************************************************************
 * \fn USART_status_t USART2_power_on(void)
 * \brief Power on all modules connected to the USART2 link.
 * \param[in]  	none
 * \param[out] 	none
 * \retval		Function execution status.
 *******************************************************************/
USART_status_t USART2_power_on(void);
#endif

#ifdef GPSM
/*!******************************************************************
 * \fn void USART2_power_off(void)
 * \brief Power off all modules connected to the USART2 link.
 * \param[in]  	none
 * \param[out] 	none
 * \retval		none
 *******************************************************************/
void USART2_power_off(void);
#endif

#ifdef GPSM
/*!******************************************************************
 * \fn USART_status_t USART2_send_byte(uint8_t data)
 * \brief Send byte over USART2
 * \param[in]	data: Byte to send.
 * \param[out] 	none
 * \retval		Function execution status.
 *******************************************************************/
USART_status_t USART2_send_byte(uint8_t data);
#endif

/*******************************************************************/
#define USART2_check_status(error_base) { if (usart2_status != USART_SUCCESS) { status = error_base + usart2_status; goto errors; } }

/*******************************************************************/
#define USART2_stack_error() { ERROR_stack_error(usart2_status, USART_SUCCESS, ERROR_BASE_USART2); }

/*******************************************************************/
#define USART2_print_error() { ERROR_print_error(usart2_status, USART_SUCCESS, ERROR_BASE_USART2); }

#endif /* __USART_H__ */

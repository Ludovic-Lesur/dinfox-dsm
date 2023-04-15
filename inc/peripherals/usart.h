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

typedef enum {
	USART_SUCCESS = 0,
	USART_ERROR_TX_TIMEOUT,
	USART_ERROR_BASE_LPTIM = 0x0100,
	USART_ERROR_BASE_LAST = (USART_ERROR_BASE_LPTIM + LPTIM_ERROR_BASE_LAST)
} USART_status_t;

#ifdef GPSM

/*** USART functions ***/

void USART2_init(void);
USART_status_t USART2_power_on(void);
void USART2_power_off(void);
USART_status_t USART2_send_byte(uint8_t tx_byte);

#define USART2_status_check(error_base) { if (usart2_status != USART_SUCCESS) { status = error_base + usart2_status; goto errors; }}
#define USART2_error_check() { ERROR_status_check(usart2_status, USART_SUCCESS, ERROR_BASE_USART2); }
#define USART2_error_check_print() { ERROR_status_check_print(usart2_status, USART_SUCCESS, ERROR_BASE_USART2); }

#endif /* GPSM */

#endif /* __USART_H__ */

/*
 * lpuart.h
 *
 *  Created on: 9 jul. 2019
 *      Author: Ludo
 */

#ifndef __LPUART_H__
#define __LPUART_H__

#include "mode.h"
#include "types.h"

/*** LPUART functions ***/

#ifdef AM
void LPUART1_init(uint8_t node_address);
#else
void LPUART1_init(void);
#endif
void LPUART1_enable_rx(void);
void LPUART1_disable_rx(void);
void LPUART1_send_string(char_t* tx_string);

#endif /* __LPUART_H__ */

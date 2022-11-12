/*
 * rs485.h
 *
 *  Created on: 18 apr. 2020
 *      Author: Ludo
 */

#ifndef __RS485_H__
#define __RS485_H__

#include "types.h"

/*** RS485 macros ***/

#define RS485_ADDRESS_MASK	0x7F

/*** RS485 functions ***/

void RS485_init(void);
void RS485_task(void);
void RS485_fill_rx_buffer(uint8_t rx_byte);

#endif /* __RS485_H__ */

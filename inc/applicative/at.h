/*
 * at.h
 *
 *  Created on: 18 apr. 2020
 *      Author: Ludo
 */

#ifndef __AT_H__
#define __AT_H__

#include "types.h"

/*** AT functions ***/

void AT_init(void);
void AT_task(void);
void AT_fill_rx_buffer(uint8_t rx_byte);
void AT_print_test_result(uint8_t status, int16_t rssi);

#endif /* __AT_H__ */

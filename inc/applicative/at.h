/*
 * at.h
 *
 *  Created on: 18 apr. 2020
 *      Author: Ludo
 */

#ifndef AT_H
#define AT_H

/*** AT functions ***/

void AT_init(void);
void AT_task(void);
void AT_fill_rx_buffer(unsigned char rx_byte);

#endif /* AT_H */

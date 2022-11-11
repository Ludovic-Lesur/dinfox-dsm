/*
 * relay.h
 *
 *  Created on: Mar 03, 2022
 *      Author: Ludo
 */

#ifndef RELAY_H
#define RELAY_H

#include "types.h"

/**** RELAY functions ***/

void RELAY_init(void);
void RELAY_set_state(uint8_t enable);

#endif /* RELAY_H */

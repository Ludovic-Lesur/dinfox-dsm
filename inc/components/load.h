/*
 * load.h
 *
 *  Created on: 17 feb. 2023
 *      Author: Ludo
 */

#ifndef __LOAD_H__
#define __LOAD_H__

#include "types.h"

#if (defined LVRM) || (defined BPSM) || (defined DDRM) || (defined RRM)

/*** LOAD functions ***/

void LOAD_init(void);
void LOAD_set_output_state(uint8_t state);
uint8_t LOAD_get_output_state(void);
#ifdef BPSM
void LOAD_set_charge_state(uint8_t state);
uint8_t LOAD_get_charge_state(void);
uint8_t LOAD_get_charge_status(void);
#endif

#endif

#endif /* __LOAD_H__ */

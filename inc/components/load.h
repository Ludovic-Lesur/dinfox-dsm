/*
 * load.h
 *
 *  Created on: 17 feb. 2023
 *      Author: Ludo
 */

#ifndef __LOAD_H__
#define __LOAD_H__

#include "lptim.h"
#include "types.h"

/*** LOAD structures ***/

typedef enum {
	LOAD_SUCCESS = 0,
	LOAD_ERROR_STATE_UNKNOWN,
	LOAD_ERROR_BASE_LPTIM = 0x0100,
	LOAD_ERROR_BASE_LAST = (LOAD_ERROR_BASE_LPTIM + LPTIM_ERROR_BASE_LAST)
} LOAD_status_t;

#if (defined LVRM) || (defined BPSM) || (defined DDRM) || (defined RRM)

/*** LOAD functions ***/

void LOAD_init(void);
LOAD_status_t LOAD_set_output_state(uint8_t state);
LOAD_status_t LOAD_get_output_state(uint8_t* state);
#ifdef BPSM
void LOAD_set_charge_state(uint8_t state);
uint8_t LOAD_get_charge_state(void);
uint8_t LOAD_get_charge_status(void);
#endif

#define LOAD_check_status(error_base) { if (load_status != LOAD_SUCCESS) { status = error_base + load_status; goto errors; }}
#define LOAD_stack_error() { ERROR_stack_error(load_status, LOAD_SUCCESS, ERROR_BASE_LOAD); }
#define LOAD_print_error() { ERROR_print_error(load_status, LOAD_SUCCESS, ERROR_BASE_LOAD); }

#endif

#endif /* __LOAD_H__ */

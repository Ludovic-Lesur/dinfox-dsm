/*
 * load.c
 *
 *  Created on: 17 feb. 2023
 *      Author: Ludo
 */

#include "load.h"

#include "error.h"
#include "gpio.h"
#include "gpio_mapping.h"
#include "lptim.h"
#include "mode.h"
#include "types.h"

#ifdef XM_LOAD_CONTROL

/*** LOAD local macros ***/

#define LOAD_DC_DC_DELAY_MS				100
#define LOAD_VCOIL_DELAY_MS				100
#define LOAD_RELAY_CONTROL_DURATION_MS	1000

/*** LOAD local global variables ***/

static uint8_t load_state = 0xFF;

/*** LOAD functions ***/

/*******************************************************************/
void LOAD_init(void) {
	// Output control.
#if (defined LVRM) && (defined HW2_0)
	GPIO_configure(&GPIO_DC_DC_POWER_ENABLE, GPIO_MODE_OUTPUT, GPIO_TYPE_PUSH_PULL, GPIO_SPEED_LOW, GPIO_PULL_NONE);
	GPIO_configure(&GPIO_COIL_POWER_ENABLE, GPIO_MODE_OUTPUT, GPIO_TYPE_PUSH_PULL, GPIO_SPEED_LOW, GPIO_PULL_NONE);
	GPIO_configure(&GPIO_OUT_SELECT, GPIO_MODE_OUTPUT, GPIO_TYPE_PUSH_PULL, GPIO_SPEED_LOW, GPIO_PULL_NONE);
	GPIO_configure(&GPIO_OUT_CONTROL, GPIO_MODE_OUTPUT, GPIO_TYPE_PUSH_PULL, GPIO_SPEED_LOW, GPIO_PULL_NONE);
#else
	GPIO_configure(&GPIO_OUT_EN, GPIO_MODE_OUTPUT, GPIO_TYPE_PUSH_PULL, GPIO_SPEED_LOW, GPIO_PULL_NONE);
#endif
#ifdef BPSM
	// Charger interface.
	GPIO_configure(&GPIO_CHRG_EN, GPIO_MODE_OUTPUT, GPIO_TYPE_PUSH_PULL, GPIO_SPEED_LOW, GPIO_PULL_NONE);
	GPIO_configure(&GPIO_CHRG_ST, GPIO_MODE_INPUT, GPIO_TYPE_PUSH_PULL, GPIO_SPEED_LOW, GPIO_PULL_NONE);
#endif
	// Open load by default.
	LOAD_set_output_state(0);
}

/*******************************************************************/
LOAD_status_t LOAD_set_output_state(uint8_t state) {
	// Local variables.
	LOAD_status_t status = LOAD_SUCCESS;
	// Directly exit with success if state is already set.
	if (state == load_state) goto errors;
#if (defined LVRM) && (defined HW2_0)
	LPTIM_status_t lptim_status = LPTIM_SUCCESS;
	// Enable DC-DC.
	GPIO_write(&GPIO_DC_DC_POWER_ENABLE, 1);
	lptim_status = LPTIM_delay_milliseconds(LOAD_DC_DC_DELAY_MS, LPTIM_DELAY_MODE_STOP);
	LPTIM_exit_error(LOAD_ERROR_BASE_LPTIM);
	// Enable COIL voltage.
	GPIO_write(&GPIO_COIL_POWER_ENABLE, 1);
	lptim_status = LPTIM_delay_milliseconds(LOAD_VCOIL_DELAY_MS, LPTIM_DELAY_MODE_STOP);
	LPTIM_exit_error(LOAD_ERROR_BASE_LPTIM);
	// Select coil.
	GPIO_write(&GPIO_OUT_SELECT, state);
	// Set relay state.
	GPIO_write(&GPIO_OUT_CONTROL, 1);
	lptim_status = LPTIM_delay_milliseconds(LOAD_RELAY_CONTROL_DURATION_MS, LPTIM_DELAY_MODE_STOP);
	LPTIM_exit_error(LOAD_ERROR_BASE_LPTIM);
#else
	// Set GPIO.
	GPIO_write(&GPIO_OUT_EN, state);
#endif
	// Update state.
	load_state = state;
errors:
#if (defined LVRM) && (defined HW2_0)
	// Turn all GPIOs off.
	GPIO_write(&GPIO_OUT_CONTROL, 0);
	GPIO_write(&GPIO_OUT_SELECT, 0);
	GPIO_write(&GPIO_COIL_POWER_ENABLE, 0);
	GPIO_write(&GPIO_DC_DC_POWER_ENABLE, 0);
#endif
	return status;
}

/*******************************************************************/
uint8_t LOAD_get_output_state(void) {
	// Read current state.
	return load_state;
}

#ifdef BPSM
/*******************************************************************/
void LOAD_set_charge_state(uint8_t state) {
	// Set GPIO.
	GPIO_write(&GPIO_CHRG_EN, state);
}
#endif

#ifdef BPSM
/*******************************************************************/
uint8_t LOAD_get_charge_state(void) {
	// Read GPIO.
	return (GPIO_read(&GPIO_CHRG_EN));
}
#endif

#ifdef BPSM
/*******************************************************************/
uint8_t LOAD_get_charge_status(void) {
	// Read GPIO.
	return (GPIO_read(&GPIO_CHRG_ST));
}
#endif

#endif /* XM_LOAD_CONTROL */

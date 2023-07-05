/*
 * load.c
 *
 *  Created on: 17 feb. 2023
 *      Author: Ludo
 */

#include "load.h"

#include "gpio.h"
#include "lptim.h"
#include "mapping.h"
#include "types.h"

#if (defined LVRM) || (defined BPSM) || (defined DDRM) || (defined RRM)

/*** LOAD local macros ***/

#define LOAD_STATE_UNKNOWN_VALUE		0xFF
#if (defined LVRM) && (defined HW2_0)
#define LOAD_DC_DC_DELAY_MS				100
#define LOAD_VCOIL_DELAY_MS				100
#define LOAD_RELAY_CONTROL_DURATION_MS	1000
#endif

/*** LOAD local global variables ***/

static uint8_t load_state = LOAD_STATE_UNKNOWN_VALUE;

/*** LOAD functions ***/

/* INIT POWER LOAD INTERFACE.
 * @param:	None.
 * @return:	None.
 */
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
	// Open relay by default.
	LOAD_set_output_state(0);
}

/* ENABLE OR DISABLE LOAD POWER PATH.
 * @param state:	State to apply (0 to turn off, non-zero to turn on).
 * @return status:	Function executions status.
 */
LOAD_status_t LOAD_set_output_state(uint8_t state) {
	// Local variables.
	LOAD_status_t status = LOAD_SUCCESS;
	// Directly exit with success if state is already set.
	if ((state == load_state) && (load_state != LOAD_STATE_UNKNOWN_VALUE)) goto errors;
#if (defined LVRM) && (defined HW2_0)
	LPTIM_status_t lptim1_status = LPTIM_SUCCESS;
	// Enable DC-DC.
	GPIO_write(&GPIO_COIL_POWER_ENABLE, 1);
	lptim1_status = LPTIM1_delay_milliseconds(LOAD_DC_DC_DELAY_MS, LPTIM_DELAY_MODE_STOP);
	LPTIM1_status_check(LOAD_ERROR_BASE_LPTIM);
	// Enable COIL voltage.
	GPIO_write(&GPIO_COIL_POWER_ENABLE, 1);
	lptim1_status = LPTIM1_delay_milliseconds(LOAD_VCOIL_DELAY_MS, LPTIM_DELAY_MODE_STOP);
	LPTIM1_status_check(LOAD_ERROR_BASE_LPTIM);
	// Select coil.
	GPIO_write(&GPIO_OUT_SELECT, state);
	// Set relay state.
	GPIO_write(&GPIO_OUT_CONTROL, 1);
	lptim1_status = LPTIM1_delay_milliseconds(LOAD_RELAY_CONTROL_DURATION_MS, LPTIM_DELAY_MODE_STOP);
	LPTIM1_status_check(LOAD_ERROR_BASE_LPTIM);
	// Turn all GPIOs off.
	GPIO_write(&GPIO_OUT_CONTROL, 0);
	GPIO_write(&GPIO_OUT_SELECT, 0);
	GPIO_write(&GPIO_COIL_POWER_ENABLE, 0);
	GPIO_write(&GPIO_DC_DC_POWER_ENABLE, 0);
#else
	// Set GPIO.
	GPIO_write(&GPIO_OUT_EN, state);
#endif
	// Update state.
	load_state = state;
errors:
	return status;
}

/* READ LOAD POWER PATH STATE.
 * @param state:	Pointer that will contain the current load state;
 * @return status:	Function executions status.
 */
LOAD_status_t LOAD_get_output_state(uint8_t* state) {
	// Local variables.
	LOAD_status_t status = LOAD_SUCCESS;
	// Check state.
	if (load_state == LOAD_STATE_UNKNOWN_VALUE) {
		status = LOAD_ERROR_STATE_UNKNOWN;
		goto errors;
	}
	(*state) = load_state;
errors:
	return status;
}

#ifdef BPSM
/* ENABLE OR DISABLE CHARGER.
 * @param state:	State to apply (0 to turn off, non-zero to turn on).
 * @return:			None.
 */
void LOAD_set_charge_state(uint8_t state) {
	// Set GPIO.
	GPIO_write(&GPIO_CHRG_EN, state);
}
#endif

#ifdef BPSM
/* READ CHARGE STATE.
 * @param:	None.
 * @return:	Charge status indicator.
 */
uint8_t LOAD_get_charge_state(void) {
	// Read GPIO.
	return (GPIO_read(&GPIO_CHRG_EN));
}
#endif

#ifdef BPSM
/* READ CHARGE STATUS INDICATOR.
 * @param:	None.
 * @return:	Charge status indicator.
 */
uint8_t LOAD_get_charge_status(void) {
	// Read GPIO.
	return (GPIO_read(&GPIO_CHRG_ST));
}
#endif

#endif

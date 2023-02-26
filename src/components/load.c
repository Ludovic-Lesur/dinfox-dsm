/*
 * load.c
 *
 *  Created on: 17 feb. 2023
 *      Author: Ludo
 */

#include "load.h"

#include "gpio.h"
#include "mapping.h"
#include "types.h"

#if (defined LVRM) || (defined BPSM) || (defined DDRM) || (defined RRM)

/*** LOAD functions ***/

/* INIT POWER LOAD INTERFACE.
 * @param:	None.
 * @return:	None.
 */
void LOAD_init(void) {
	// Output enable.
	GPIO_configure(&GPIO_OUT_EN, GPIO_MODE_OUTPUT, GPIO_TYPE_PUSH_PULL, GPIO_SPEED_LOW, GPIO_PULL_NONE);
#ifdef BPSM
	// Charger interface.
	GPIO_configure(&GPIO_CHRG_EN, GPIO_MODE_OUTPUT, GPIO_TYPE_PUSH_PULL, GPIO_SPEED_LOW, GPIO_PULL_NONE);
	GPIO_configure(&GPIO_CHRG_ST, GPIO_MODE_INPUT, GPIO_TYPE_PUSH_PULL, GPIO_SPEED_LOW, GPIO_PULL_NONE);
#endif
}

/* ENABLE OR DISABLE LOAD POWER PATH.
 * @param state:	State to apply (0 to turn off, non-zero to turn on).
 * @return:			None.
 */
void LOAD_set_output_state(uint8_t state) {
	// Set GPIO.
	GPIO_write(&GPIO_OUT_EN, state);
}

/* READ LOAD POWER PATH STATE.
 * @param:	None.
 * @return:	Load state.
 */
uint8_t LOAD_get_output_state(void) {
	// Read GPIO.
	return (GPIO_read(&GPIO_OUT_EN));
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

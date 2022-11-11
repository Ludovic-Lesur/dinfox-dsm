/*
 * relay.c
 *
 *  Created on: Mar 03, 2022
 *      Author: Ludo
 */

#include "relay.h"

#include "gpio.h"
#include "mapping.h"
#include "types.h"

/**** RELAY functions ***/

/* INIT RELAY INTERFACE.
 * @param:	None.
 * @return:	None.
 */
void RELAY_init(void) {
	// Init GPIO.
	GPIO_configure(&GPIO_OUT_EN, GPIO_MODE_OUTPUT, GPIO_TYPE_PUSH_PULL, GPIO_SPEED_LOW, GPIO_PULL_NONE);
}

/* SET RELAY STATE.
 * @param enable:	Enable or disable the relay.
 * @return:			None.
 */
void RELAY_set_state(uint8_t enable) {
	// Set GPIO.
	GPIO_write(&GPIO_OUT_EN, enable);
}

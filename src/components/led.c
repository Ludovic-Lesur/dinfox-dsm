/*
 * led.c
 *
 *  Created on: 22 aug. 2020
 *      Author: Ludo
 */

#include "led.h"

#include "gpio.h"
#include "mapping.h"
#include "tim.h"

/*** LED local functions ***/

/* TURN LED OFF.
 * @param:	None.
 * @return:	None.
 */
static void LED_Off(void) {
	// Configure pins as output high.
	GPIO_configure(&GPIO_LED_RED, GPIO_MODE_OUTPUT, GPIO_TYPE_PUSH_PULL, GPIO_SPEED_LOW, GPIO_PULL_NONE);
	GPIO_configure(&GPIO_LED_GREEN, GPIO_MODE_OUTPUT, GPIO_TYPE_PUSH_PULL, GPIO_SPEED_LOW, GPIO_PULL_NONE);
	GPIO_configure(&GPIO_LED_BLUE, GPIO_MODE_OUTPUT, GPIO_TYPE_PUSH_PULL, GPIO_SPEED_LOW, GPIO_PULL_NONE);
	GPIO_write(&GPIO_LED_RED, 1);
	GPIO_write(&GPIO_LED_GREEN, 1);
	GPIO_write(&GPIO_LED_BLUE, 1);
}

/*** LED functions ***/

/* INIT LED.
 * @param:	None.
 * @return:	None.
 */
void LED_init(void) {
	LED_Off();
}

/* SET LED COLOR.
 * @param blink_period_ms:	Blink duration in ms.
 * @param led_color:		Color to set.
 * @return:					None.
 */
void LED_SingleBlink(unsigned int blink_duration_ms, TIM2_channel_mask_t color) {
	// Init required peripheral.
	TIM2_init();
	TIM21_init(blink_duration_ms);
	// Set color according to thresholds.
	TIM2_set_color_mask(color);
	// Start blink.
	TIM2_start();
	TIM21_Start();
	// Wait the end of blink.
	while (TIM21_IsSingleBlinkDone() == 0);
	// Stop timers.
	TIM2_stop();
	TIM21_Stop();
	// Turn peripherals off.
	TIM2_disable();
	TIM21_disable();
	// Turn LED off.
	LED_Off();
}


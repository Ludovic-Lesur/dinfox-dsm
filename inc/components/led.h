/*
 * led.h
 *
 *  Created on: 22 aug 2020
 *      Author: Ludo
 */

#ifndef LED_H
#define LED_H

#include "tim.h"
#include "types.h"

/*** LED structures ***/

typedef enum {
	LED_SUCCESS,
	LED_ERROR_NULL_DURATION,
	LED_ERROR_COLOR,
	LED_ERROR_BASE_LAST = 0x0100
} LED_status_t;

#if (defined LVRM) || (defined DDRM) || (defined RRM)

typedef enum {
	LED_COLOR_OFF = TIM2_CHANNEL_MASK_OFF,
	LED_COLOR_RED = TIM2_CHANNEL_MASK_RED,
	LED_COLOR_GREEN = TIM2_CHANNEL_MASK_GREEN,
	LED_COLOR_YELLOW = TIM2_CHANNEL_MASK_YELLOW,
	LED_COLOR_BLUE = TIM2_CHANNEL_MASK_BLUE,
	LED_COLOR_MAGENTA = TIM2_CHANNEL_MASK_MAGENTA,
	LED_COLOR_CYAN = TIM2_CHANNEL_MASK_CYAN,
	LED_COLOR_WHITE = TIM2_CHANNEL_MASK_WHITE,
	LED_COLOR_LAST
} LED_color_t;

/*** LED functions ***/

void LED_init(void);
LED_status_t LED_start_single_blink(uint32_t blink_duration_ms, LED_color_t color);
uint8_t LED_is_single_blink_done(void);
void LED_stop_blink(void);

#define LED_status_check(error_base) { if (led_status != LED_SUCCESS) { status = error_base + led_status; goto errors; }}
#define LED_error_check() { ERROR_status_check(led_status, LED_SUCCESS, ERROR_BASE_LED); }
#define LED_error_check_print() { ERROR_status_check_print(led_status, LED_SUCCESS, ERROR_BASE_LED); }

#endif /* LVRM or DDRM or RRM */

#endif /* LED_H */

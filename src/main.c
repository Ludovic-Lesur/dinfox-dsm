/*
 * main.c
 *
 *  Created on: Feb 05, 2022
 *      Author: Ludo
 */

#include "adc.h"
#include "exti.h"
#include "gpio.h"
#include "iwdg.h"
#include "led.h"
#include "lpuart.h"
#include "mapping.h"
#include "mode.h"
#include "nvic.h"
#include "pwr.h"
#include "rcc.h"
#include "relay.h"
#include "rtc.h"
#include "rs485.h"
#include "tim.h"

/*** MAIN local macros ***/

#define LVRM_NUMBER_OF_IOUT_THRESHOLD	6

/*** MAIN structures ***/

#ifdef RSM
typedef struct {
	TIM2_channel_mask_t led_color;
	uint32_t iout_ua;
} LVRM_context_t;
#endif

/*** MAIN local global variables ***/

#ifdef RSM
static const uint32_t lvrm_iout_threshold_ua[LVRM_NUMBER_OF_IOUT_THRESHOLD] = {
	50000,
	500000,
	1000000,
	2000000,
	3000000,
	4000000
};
static const TIM2_channel_mask_t lvrm_iout_led_color[LVRM_NUMBER_OF_IOUT_THRESHOLD + 1] = {
	TIM2_CHANNEL_MASK_GREEN,
	TIM2_CHANNEL_MASK_YELLOW,
	TIM2_CHANNEL_MASK_RED,
	TIM2_CHANNEL_MASK_MAGENTA,
	TIM2_CHANNEL_MASK_BLUE,
	TIM2_CHANNEL_MASK_CYAN,
	TIM2_CHANNEL_MASK_WHITE
};
static LVRM_context_t lvrm_ctx;
#endif

/*** MAIN local functions ***/

/* COMMON INIT FUNCTION FOR PERIPHERALS AND COMPONENTS.
 * @param:	None.
 * @return:	None.
 */
static void _LVRM_init_hw(void) {
	// Init memory.
	NVIC_init();
	// Init power and clock modules.
	PWR_init();
	RCC_init();
	RCC_enable_lsi();
	// Init watchdog.
#ifndef DEBUG
	IWDG_init();
#endif
	// Init GPIOs.
	GPIO_init();
	EXTI_init();
	// Init RTC.
	RTC_reset();
	RCC_enable_lse();
	RTC_init();
	// Init peripherals.
	LPUART1_init();
	ADC1_init();
	// Init components.
	LED_init();
	RELAY_init();
	// Init AT interface.
	RS485_init();
}

#ifdef RSM
/* UPDATE LED COLOR ACCORDING TO OUTPUT CURRENT VALUE.
 * @param:	None.
 * @return:	None.
 */
static void _LVRM_update_led_color(void) {
	// Local variables.
	uint8_t idx = 0;
	// Default is maximum.
	lvrm_ctx.led_color = lvrm_iout_led_color[LVRM_NUMBER_OF_IOUT_THRESHOLD];
	// Check thresholds.
	for (idx=0 ; idx<LVRM_NUMBER_OF_IOUT_THRESHOLD ; idx++) {
		if (lvrm_ctx.iout_ua < lvrm_iout_threshold_ua[idx]) {
			lvrm_ctx.led_color = lvrm_iout_led_color[idx];
			break;
		}
	}
}
#endif

/*** MAIN function ***/

/* MAIN FUNCTION.
 * @param:	None.
 * @return:	None.
 */
int main(void) {
	// Init board.
	_LVRM_init_hw();
	// Start periodic wakeup timer.
	RTC_start_wakeup_timer(RTC_WAKEUP_PERIOD_SECONDS);
	// Main loop.
	while (1) {
#ifdef RSM
		// Enter sleep or stop mode depending on LED state.
		if (TIM21_is_single_blink_done() != 0) {
			LED_stop_blink();
			PWR_enter_stop_mode();
		}
		else {
			PWR_enter_sleep_mode();
		}
		// Blink LED according to output current.
		if (RTC_get_wakeup_timer_flag() != 0) {
			// Wake-up by RTC: clear flag and blink LED.
			RTC_clear_wakeup_timer_flag();
			// Perform analog measurements.
			ADC1_perform_measurements();
			ADC1_get_data(ADC_DATA_INDEX_IOUT_UA, &lvrm_ctx.iout_ua);
			// Compute LED color according to output current.
			_LVRM_update_led_color();
			// Blink LED.
			LED_start_blink(2000, lvrm_ctx.led_color);
		}
#endif
#ifdef ATM
		// Enter stop mode.
		PWR_enter_stop_mode();
#endif
		// Perform command task.
		RS485_task();
		// Reload watchdog.
		IWDG_reload();
	}
}

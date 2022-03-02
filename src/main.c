/*
 * main.c
 *
 *  Created on: Feb 05, 2022
 *      Author: Ludo
 */

#include "adc.h"
#include "at.h"
#include "gpio.h"
#include "iwdg.h"
#include "led.h"
#include "lptim.h"
#include "lpuart.h"
#include "nvic.h"
#include "pwr.h"
#include "rcc.h"
#include "rtc.h"
#include "tim.h"

/*** MAIN function ***/

/* MAIN FUNCTION.
 * @param:	None.
 * @return:	None.
 */
int main(void) {
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
	// Init RTC.
	RTC_reset();
	RCC_enable_lse();
	RTC_init();
	// Init peripherals.
	LPTIM1_init();
	ADC1_init();
	LPUART1_init();
	// Init components.
	LED_init();
	// Init AT interface.
	AT_init();
	// Start periodic wakeup timer.
	RTC_start_wakeup_timer(RTC_WAKEUP_PERIOD_SECONDS);
	// Main loop.
	while (1) {
		// Enter stop mode.
		PWR_enter_stop_mode();
		// Wake-up.
		if (RTC_get_wakeup_timer_flag) {
			// Blink LED and clear flag.
			LED_SingleBlink(2000, TIM2_CHANNEL_MASK_GREEN);
			RTC_clear_wakeup_timer_flag();
		}
		// Execute AT commands task.
		AT_task();
		// Clear watchdog.
		IWDG_reload();
	}
}

/*
 * main.c
 *
 *  Created on: Feb 05, 2022
 *      Author: Ludo
 */

#include "adc.h"
#include "exti.h"
#include "error.h"
#include "gpio.h"
#include "iwdg.h"
#include "led.h"
#include "lpuart.h"
#include "mapping.h"
#include "mode.h"
#include "nvic.h"
#include "nvm.h"
#include "pwr.h"
#include "rcc.h"
#include "rtc.h"
#include "rs485.h"
#include "tim.h"

/*** MAIN local macros ***/

#define LVRM_IOUT_INDICATOR_RANGE	7

/*** MAIN structures ***/

typedef struct {
	uint32_t threshold_ua;
	TIM2_channel_mask_t led_color;
} LVRM_iout_indicator_t;

typedef struct {
	TIM2_channel_mask_t led_color;
	uint32_t iout_ua;
} LVRM_context_t;

/*** MAIN local global variables ***/

static const LVRM_iout_indicator_t LVRM_IOUT_INDICATOR[LVRM_IOUT_INDICATOR_RANGE] = {
	{0, TIM2_CHANNEL_MASK_GREEN},
	{50000, TIM2_CHANNEL_MASK_YELLOW},
	{500000, TIM2_CHANNEL_MASK_RED},
	{1000000, TIM2_CHANNEL_MASK_MAGENTA},
	{2000000, TIM2_CHANNEL_MASK_BLUE},
	{3000000, TIM2_CHANNEL_MASK_CYAN},
	{4000000, TIM2_CHANNEL_MASK_WHITE}
};

static LVRM_context_t lvrm_ctx;

/*** MAIN local functions ***/

/* COMMON INIT FUNCTION FOR PERIPHERALS AND COMPONENTS.
 * @param:	None.
 * @return:	None.
 */
static void _LVRM_init_hw(void) {
	// Local variables.
	ADC_status_t adc1_status = ADC_SUCCESS;
	RTC_status_t rtc_status = RTC_SUCCESS;
#ifdef AM
	NVM_status_t nvm_status = NVM_SUCCESS;
	uint8_t node_address;
#endif
	// Init error stack
	ERROR_stack_init();
	// Init memory.
	NVIC_init();
	NVM_init();
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
	rtc_status = RTC_init();
	RTC_error_check();
#ifdef AM
	// Read RS485 address in NVM.
	nvm_status = NVM_read_byte(NVM_ADDRESS_RS485_ADDRESS, &node_address);
	NVM_error_check();
#endif
	// Init peripherals.
	adc1_status = ADC1_init();
	ADC1_error_check();
	TIM2_init();
	TIM21_init();
#ifdef AM
	LPUART1_init(node_address);
#else
	LPUART1_init();
#endif
	// Init components.
	LED_init();
	// Init AT interface.
	RS485_init();
}

/* UPDATE LED COLOR ACCORDING TO OUTPUT CURRENT VALUE.
 * @param:	None.
 * @return:	None.
 */
static void _LVRM_update_led_color(void) {
	// Local variables.
	uint8_t idx = LVRM_IOUT_INDICATOR_RANGE;
	// Get range and corresponding color.
	do {
		idx--;
		lvrm_ctx.led_color = LVRM_IOUT_INDICATOR[idx].led_color;
		if (lvrm_ctx.iout_ua >= LVRM_IOUT_INDICATOR[idx].threshold_ua) break;
	}
	while (idx > 0);
}

/*** MAIN function ***/

/* MAIN FUNCTION.
 * @param:	None.
 * @return:	None.
 */
int main(void) {
	// Init board.
	_LVRM_init_hw();
	// Local variables.
	ADC_status_t adc1_status = ADC_SUCCESS;
	RTC_status_t rtc_status = RTC_SUCCESS;
	// Start periodic wakeup timer.
	rtc_status = RTC_start_wakeup_timer(RTC_WAKEUP_PERIOD_SECONDS);
	RTC_error_check();
	// Main loop.
	while (1) {
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
			adc1_status = ADC1_perform_measurements();
			ADC1_error_check();
			adc1_status = ADC1_get_data(ADC_DATA_INDEX_IOUT_UA, &lvrm_ctx.iout_ua);
			ADC1_error_check();
			// Compute LED color according to output current.
			_LVRM_update_led_color();
			// Blink LED.
			LED_start_blink(2000, lvrm_ctx.led_color);
		}
		// Perform command task.
		RS485_task();
		// Reload watchdog.
		IWDG_reload();
	}
}

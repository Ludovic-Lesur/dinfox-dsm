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
#include "lptim.h"
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

#if (defined LVRM) || (defined DDRM) || (defined RRM)
#define XM_IOUT_INDICATOR_RANGE					7
#define XM_IOUT_INDICATOR_PERIOD_THRESHOLD_MV	3600
#define XM_IOUT_INDICATOR_PERIOD_HIGH_SECONDS	5
#define XM_IOUT_INDICATOR_PERIOD_LOW_SECONDS	60
#endif

/*** MAIN structures ***/

#if (defined LVRM) || (defined DDRM) || (defined RRM)
typedef struct {
	uint32_t threshold_ua;
	TIM2_channel_mask_t led_color;
} XM_iout_indicator_t;
#endif

#if (defined LVRM) || (defined DDRM) || (defined RRM)
typedef struct {
	TIM2_channel_mask_t led_color;
	uint32_t input_voltage_mv;
	uint32_t iout_ua;
	uint32_t iout_indicator_seconds_count;
	uint32_t iout_indicator_period_seconds;
} XM_context_t;
#endif

/*** MAIN local global variables ***/

#if (defined LVRM) || (defined DDRM) || (defined RRM)
static const XM_iout_indicator_t LVRM_IOUT_INDICATOR[XM_IOUT_INDICATOR_RANGE] = {
	{0, TIM2_CHANNEL_MASK_GREEN},
	{50000, TIM2_CHANNEL_MASK_YELLOW},
	{500000, TIM2_CHANNEL_MASK_RED},
	{1000000, TIM2_CHANNEL_MASK_MAGENTA},
	{2000000, TIM2_CHANNEL_MASK_BLUE},
	{3000000, TIM2_CHANNEL_MASK_CYAN},
	{4000000, TIM2_CHANNEL_MASK_WHITE}
};
static XM_context_t xm_ctx;
#endif

/*** MAIN local functions ***/

#if (defined LVRM) || (defined DDRM) || (defined RRM)
/* COMMON INIT FUNCTION FOR MAIN CONTEXT.
 * @param:	None.
 * @return:	None.
 */
static void _XM_init_context(void) {
	xm_ctx.led_color = TIM2_CHANNEL_MASK_OFF;
	xm_ctx.input_voltage_mv = 0;
	xm_ctx.iout_ua = 0;
	xm_ctx.iout_indicator_seconds_count = 0;
	xm_ctx.iout_indicator_period_seconds = XM_IOUT_INDICATOR_PERIOD_LOW_SECONDS;
}
#endif

/* COMMON INIT FUNCTION FOR PERIPHERALS AND COMPONENTS.
 * @param:	None.
 * @return:	None.
 */
static void _XM_init_hw(void) {
	// Local variables.
	ADC_status_t adc1_status = ADC_SUCCESS;
	RTC_status_t rtc_status = RTC_SUCCESS;
#ifdef AM
	LPUART_status_t lpuart1_status = LPUART_SUCCESS;
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
	LPTIM1_init();
	adc1_status = ADC1_init();
	ADC1_error_check();
#if (defined LVRM) || (defined DDRM) || (defined RRM)
	TIM2_init();
	TIM21_init();
#endif
#ifdef AM
	lpuart1_status = LPUART1_init(node_address);
	LPUART1_error_check();
#else
	LPUART1_init();
#endif
	// Init components.
#if (defined LVRM) || (defined DDRM) || (defined RRM)
	LED_init();
#endif
	// Init AT interface.
	RS485_init();
}

#if (defined LVRM) || (defined DDRM) || (defined RRM)
/* UPDATE LED COLOR ACCORDING TO OUTPUT CURRENT VALUE.
 * @param:	None.
 * @return:	None.
 */
static void _XM_update_led_color(void) {
	// Local variables.
	uint8_t idx = XM_IOUT_INDICATOR_RANGE;
	// Get range and corresponding color.
	do {
		idx--;
		xm_ctx.led_color = LVRM_IOUT_INDICATOR[idx].led_color;
		if (xm_ctx.iout_ua >= LVRM_IOUT_INDICATOR[idx].threshold_ua) break;
	}
	while (idx > 0);
}
#endif

/*** MAIN function ***/

/* MAIN FUNCTION.
 * @param:	None.
 * @return:	None.
 */
int main(void) {
	// Init board.
#if (defined LVRM) || (defined DDRM) || (defined RRM)
	_XM_init_context();
#endif
	_XM_init_hw();
	// Local variables.
#if (defined LVRM) || (defined DDRM) || (defined RRM)
	ADC_status_t adc1_status = ADC_SUCCESS;
#endif
	RTC_status_t rtc_status = RTC_SUCCESS;
	// Start periodic wakeup timer.
	rtc_status = RTC_start_wakeup_timer(RTC_WAKEUP_PERIOD_SECONDS);
	RTC_error_check();
	// Main loop.
	while (1) {
#if (defined LVRM) || (defined DDRM) || (defined RRM)
		// Enter sleep or stop mode depending on LED state.
		if (TIM21_is_single_blink_done() != 0) {
			LED_stop_blink();
			PWR_enter_stop_mode();
		}
		else {
			PWR_enter_sleep_mode();
		}
#else
		// Enter stop mode.
		PWR_enter_stop_mode();
#endif
		// Check RTC flag.
		if (RTC_get_wakeup_timer_flag() != 0) {
			// Clear flag.
			RTC_clear_wakeup_timer_flag();
#if (defined LVRM) || (defined DDRM) || (defined RRM)
			// Increment time.
			xm_ctx.iout_indicator_seconds_count += RTC_WAKEUP_PERIOD_SECONDS;
			// Check period.
			if (xm_ctx.iout_indicator_seconds_count >= xm_ctx.iout_indicator_period_seconds) {
				// Reset count.
				xm_ctx.iout_indicator_seconds_count = 0;
				// Perform analog measurements.
				adc1_status = ADC1_perform_measurements();
				ADC1_error_check();
				// Read data.
				adc1_status = ADC1_get_data(ADC_DATA_INDEX_IOUT_UA, &xm_ctx.iout_ua);
				ADC1_error_check();
#ifdef LVRM
				adc1_status = ADC1_get_data(ADC_DATA_INDEX_VCOM_MV, &xm_ctx.input_voltage_mv);
#endif
#if (defined DDRM) || (defined RRM)
				adc1_status = ADC1_get_data(ADC_DATA_INDEX_VIN_MV, &xm_ctx.input_voltage_mv);
#endif
				ADC1_error_check();
				// Compute LED color according to output current.
				_XM_update_led_color();
				// Update period according to input voltage.
				xm_ctx.iout_indicator_period_seconds = (xm_ctx.input_voltage_mv > XM_IOUT_INDICATOR_PERIOD_THRESHOLD_MV) ? XM_IOUT_INDICATOR_PERIOD_HIGH_SECONDS : XM_IOUT_INDICATOR_PERIOD_LOW_SECONDS;
				// Blink LED.
				LED_start_blink(2000, xm_ctx.led_color);
			}
#endif
		}
		// Perform command task.
		RS485_task();
		// Reload watchdog.
		IWDG_reload();
	}
}

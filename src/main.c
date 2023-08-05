/*
 * main.c
 *
 *  Created on: 05 feb. 2022
 *      Author: Ludo
 */

// Peripherals.
#include "exti.h"
#include "gpio.h"
#include "iwdg.h"
#include "lptim.h"
#include "mode.h"
#include "nvic.h"
#include "pwr.h"
#include "rcc.h"
#include "rtc.h"
// Utils.
#include "types.h"
// Components.
#include "led.h"
#include "load.h"
#include "power.h"
// Applicative.
#include "at_bus.h"
#include "error.h"

/*** MAIN local macros ***/

#if (defined LVRM) || (defined DDRM) || (defined RRM)
#define XM_STATIC_MEASUREMENTS_PERIOD_SECONDS	60
#define XM_IOUT_INDICATOR_PERIOD_SECONDS		10
#define XM_IOUT_INDICATOR_RANGE					7
#define XM_HIGH_PERIOD_THRESHOLD_MV				6000
#endif

/*** MAIN local structures ***/

#if (defined LVRM) || (defined DDRM) || (defined RRM)
/*******************************************************************/
typedef struct {
	uint32_t threshold_ua;
	LED_color_t led_color;
} XM_iout_indicator_t;
#endif

#if (defined LVRM) || (defined DDRM) || (defined RRM)
/*******************************************************************/
typedef struct {
	uint32_t static_measurements_seconds_count;
	uint32_t iout_indicator_seconds_count;
	uint8_t iout_indicator_enable;
} XM_context_t;
#endif

/*** MAIN local global variables ***/

#if (defined LVRM) || (defined DDRM) || (defined RRM)
static const XM_iout_indicator_t LVRM_IOUT_INDICATOR[XM_IOUT_INDICATOR_RANGE] = {
	{0, LED_COLOR_GREEN},
	{50000, LED_COLOR_YELLOW},
	{500000, LED_COLOR_RED},
	{1000000, LED_COLOR_MAGENTA},
	{2000000, LED_COLOR_BLUE},
	{3000000, LED_COLOR_CYAN},
	{4000000, LED_COLOR_WHITE}
};
static XM_context_t xm_ctx;
#endif

/*** MAIN local functions ***/

#if (defined LVRM) || (defined DDRM) || (defined RRM)
/*******************************************************************/
static void _XM_init_context(void) {
	// Init context.
	xm_ctx.static_measurements_seconds_count = 0;
	xm_ctx.iout_indicator_seconds_count = XM_IOUT_INDICATOR_PERIOD_SECONDS;
	xm_ctx.iout_indicator_enable = 0;
}
#endif

/*******************************************************************/
static void _XM_init_hw(void) {
	// Local variables.
	RCC_status_t rcc_status = RCC_SUCCESS;
	RTC_status_t rtc_status = RTC_SUCCESS;
#ifndef DEBUG
	IWDG_status_t iwdg_status = IWDG_SUCCESS;
#endif
	// Init error stack
	ERROR_stack_init();
	// Init memory.
	NVIC_init();
	// Init power module and clock tree.
	PWR_init();
	RCC_init();
	// Init GPIOs.
	GPIO_init();
	EXTI_init();
#ifndef DEBUG
	// Start independent watchdog.
	iwdg_status = IWDG_init();
	IWDG_stack_error();
	IWDG_reload();
#endif
	// High speed oscillator.
	rcc_status = RCC_switch_to_hsi();
	RCC_stack_error();
	// Init RTC.
	rtc_status = RTC_init();
	RTC_stack_error();
	// Init delay timer.
	LPTIM1_init();
	// Init components.
	POWER_init();
#if (defined LVRM) || (defined BPSM) || (defined DDRM) || (defined RRM)
	LOAD_init();
#endif
#if (defined LVRM) || (defined DDRM) || (defined RRM) || (defined GPSM)
	LED_init();
#endif
	// Init AT BUS layer.
	AT_BUS_init();
}

#if (defined LVRM) || (defined DDRM) || (defined RRM)
/*******************************************************************/
static void _XM_static_measurements(void) {
	// Local variables.
	POWER_status_t power_status = POWER_SUCCESS;
	ADC_status_t adc1_status = ADC_SUCCESS;
	uint32_t input_voltage_mv = 0;
	// Perform analog measurements.
	power_status = POWER_enable(POWER_DOMAIN_ANALOG, LPTIM_DELAY_MODE_SLEEP);
	POWER_stack_error();
	adc1_status = ADC1_perform_measurements();
	ADC1_stack_error();
	power_status = POWER_disable(POWER_DOMAIN_ANALOG);
	POWER_stack_error();
	// Check input voltage.
#ifdef LVRM
	adc1_status = ADC1_get_data(ADC_DATA_INDEX_VCOM_MV, &input_voltage_mv);
#endif
#ifdef BPSM
	adc1_status = ADC1_get_data(ADC_DATA_INDEX_VSRC_MV, &input_voltage_mv);
#endif
#if (defined DDRM) || (defined RRM)
	adc1_status = ADC1_get_data(ADC_DATA_INDEX_VIN_MV, &input_voltage_mv);
#endif
	ADC1_stack_error();
	// Enable RGB LED only if power input supplies the board.
	xm_ctx.iout_indicator_enable = (input_voltage_mv > XM_HIGH_PERIOD_THRESHOLD_MV) ? 1 : 0;
}
#endif

#if (defined LVRM) || (defined DDRM) || (defined RRM)
/*******************************************************************/
static void _XM_iout_indicator(void) {
	// Local variables.
	ADC_status_t adc1_status = ADC_SUCCESS;
	LED_status_t led_status = LED_SUCCESS;
	LED_color_t led_color;
	uint32_t iout_ua;
	uint8_t idx = XM_IOUT_INDICATOR_RANGE;
	// Read data.
	adc1_status = ADC1_get_data(ADC_DATA_INDEX_IOUT_UA, &iout_ua);
	ADC1_stack_error();
	// Compute LED color according to output current..
	do {
		idx--;
		// Get range and corresponding color.
		led_color = LVRM_IOUT_INDICATOR[idx].led_color;
		if (iout_ua >= LVRM_IOUT_INDICATOR[idx].threshold_ua) break;
	}
	while (idx > 0);
	// Blink LED.
	led_status = LED_start_single_blink(2000, led_color);
	LED_stack_error();
}
#endif

/*** MAIN function ***/

/*******************************************************************/
int main(void) {
	// Init board.
#if (defined LVRM) || (defined DDRM) || (defined RRM)
	_XM_init_context();
#endif
	_XM_init_hw();
#if (defined LVRM) || (defined DDRM) || (defined RRM)
	_XM_static_measurements();
#endif
	// Main loop.
	while (1) {
		// Reload watchdog.
		IWDG_reload();
#if (defined LVRM) || (defined DDRM) || (defined RRM)
		// Enter sleep or stop mode depending on LED state.
		if (LED_is_single_blink_done() != 0) {
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
		// Reload watchdog.
		IWDG_reload();
		// Check RTC flag.
		if (RTC_get_wakeup_timer_flag() != 0) {
			// Clear flag.
			RTC_clear_wakeup_timer_flag();
#if (defined LVRM) || (defined DDRM) || (defined RRM)
			// Increment seconds count.
			xm_ctx.static_measurements_seconds_count += RTC_WAKEUP_PERIOD_SECONDS;
			// Check ADC period.
			if (xm_ctx.static_measurements_seconds_count >= XM_STATIC_MEASUREMENTS_PERIOD_SECONDS) {
				// Reset count.
				xm_ctx.static_measurements_seconds_count = 0;
				_XM_static_measurements();
			}
			// Increment seconds count.
			xm_ctx.iout_indicator_seconds_count += RTC_WAKEUP_PERIOD_SECONDS;
			// Check Iout indicator period.
			if (xm_ctx.iout_indicator_seconds_count >= XM_IOUT_INDICATOR_PERIOD_SECONDS) {
				// Reset count.
				xm_ctx.iout_indicator_seconds_count = 0;
				// Check enable flag.
				if (xm_ctx.iout_indicator_enable != 0) {
					_XM_iout_indicator();
				}
			}
#endif
		}
		// Reload watchdog.
		IWDG_reload();
		// Perform command task.
		AT_BUS_task();
	}
}

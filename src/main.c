/*
 * main.c
 *
 *  Created on: 05 feb. 2022
 *      Author: Ludo
 */

// Peripherals.
#include "adc.h"
#include "aes.h"
#include "dma.h"
#include "exti.h"
#include "gpio.h"
#include "i2c.h"
#include "iwdg.h"
#include "lpuart.h"
#include "lptim.h"
#include "mapping.h"
#include "mode.h"
#include "nvic.h"
#include "nvm.h"
#include "pwr.h"
#include "rcc.h"
#include "rtc.h"
#include "spi.h"
#include "tim.h"
#include "usart.h"
// Components.
#include "led.h"
#include "load.h"
#include "neom8n.h"
// Nodes.
#include "lbus.h"
#include "node.h"
// Applicative.
#include "at_bus.h"
#include "error.h"

/*** MAIN local macros ***/

#define XM_MEASUREMENTS_PERIOD_HIGH_SECONDS		10
#define XM_MEASUREMENTS_PERIOD_LOW_SECONDS		60
#if (defined LVRM) || (defined DDRM) || (defined RRM)
#define XM_IOUT_INDICATOR_PERIOD_SECONDS		60
#define XM_IOUT_INDICATOR_RANGE					7
#endif
#define XM_HIGH_PERIOD_THRESHOLD_MV				6000

/*** MAIN local structures ***/

#if (defined LVRM) || (defined DDRM) || (defined RRM)
typedef struct {
	uint32_t threshold_ua;
	LED_color_t led_color;
} XM_iout_indicator_t;
#endif


typedef struct {
	uint32_t measurements_seconds_count;
	uint32_t measurements_period_seconds;
#if (defined LVRM) || (defined DDRM) || (defined RRM)
	uint32_t iout_indicator_seconds_count;
	uint8_t iout_indicator_enable;
#endif
} XM_context_t;

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
#endif
static XM_context_t xm_ctx;

/*** MAIN local functions ***/

/* COMMON INIT FUNCTION FOR MAIN CONTEXT.
 * @param:	None.
 * @return:	None.
 */
static void _XM_init_context(void) {
	// Init context.
	xm_ctx.measurements_seconds_count = 0;
	xm_ctx.measurements_period_seconds = XM_MEASUREMENTS_PERIOD_LOW_SECONDS;
#if (defined LVRM) || (defined DDRM) || (defined RRM)
	xm_ctx.iout_indicator_seconds_count = XM_IOUT_INDICATOR_PERIOD_SECONDS;
	xm_ctx.iout_indicator_enable = 0;
#endif
}

/* COMMON INIT FUNCTION FOR PERIPHERALS AND COMPONENTS.
 * @param:	None.
 * @return:	None.
 */
static void _XM_init_hw(void) {
	// Local variables.
	ADC_status_t adc1_status = ADC_SUCCESS;
	RCC_status_t rcc_status = RCC_SUCCESS;
	RTC_status_t rtc_status = RTC_SUCCESS;
	LPUART_status_t lpuart1_status = LPUART_SUCCESS;
	NVM_status_t nvm_status = NVM_SUCCESS;
	NODE_address_t self_address;
#ifndef DEBUG
	IWDG_status_t iwdg_status = IWDG_SUCCESS;
#endif
	// Init error stack
	ERROR_stack_init();
	// Init memory.
	NVIC_init();
	NVM_init();
	// Init GPIOs.
	GPIO_init();
	EXTI_init();
	// Init power and clock modules.
	PWR_init();
	RCC_init();
	RCC_enable_lsi();
	// Init watchdog.
#ifndef DEBUG
	iwdg_status = IWDG_init();
	IWDG_error_check();
#endif
	// High speed oscillator.
	IWDG_reload();
	rcc_status = RCC_switch_to_hsi();
	RCC_error_check();
	// Init RTC.
	RTC_reset();
	RCC_enable_lse();
	rtc_status = RTC_init();
	RTC_error_check();
	// Read self address in NVM.
	nvm_status = NVM_read_byte(NVM_ADDRESS_SELF_ADDRESS, &self_address);
	NVM_error_check();
	// Init peripherals.
	LPTIM1_init();
	adc1_status = ADC1_init();
	ADC1_error_check();
#if (defined LVRM) || (defined DDRM) || (defined RRM)
	TIM2_init();
	TIM21_init();
#endif
	lpuart1_status = LPUART1_init(self_address);
	LPUART1_error_check();
#ifdef SM
	I2C1_init();
#endif
#ifdef UHFM
	AES_init();
	DMA1_init_channel3();
	SPI1_init();
#endif
#ifdef GPSM
	USART2_init();
	DMA1_init_channel6();
#endif
	// Init components.
#ifdef SM
	DIGITAL_init();
#endif
#if (defined LVRM) || (defined BPSM) || (defined DDRM) || (defined RRM)
	LOAD_init();
#endif
#if (defined LVRM) || (defined DDRM) || (defined RRM) || (defined GPSM)
	LED_init();
#endif
#ifdef UHFM
	S2LP_init();
#endif
#ifdef GPSM
	NEOM8N_init();
#endif
	// Init AT interface.
	AT_BUS_init(self_address);
}

/* PERFORM EXTERNAL MEASUREMENTS.
 * @param:	None.
 * @return:	None.
 */
static void _XM_perform_measurements(void) {
	// Local variables.
	ADC_status_t adc1_status = ADC_SUCCESS;
#ifdef SM
#ifdef SM_DIO_ENABLE
	DIGITAL_status_t digital_status = DIGITAL_SUCCESS;
#endif
#ifdef SM_DIGITAL_SENSORS_ENABLE
	I2C_status_t i2c1_status = I2C_SUCCESS;
	SHT3X_status_t sht3x_status = SHT3X_SUCCESS;
#endif
#endif /* SM */
	// Perform analog measurements.
	adc1_status = ADC1_perform_measurements();
	ADC1_error_check();
#ifdef SM
#ifdef SM_DIO_ENABLE
	// Perform digital measurements.
	digital_status = DIGITAL_perform_measurements();
	DIGITAL_error_check();
#endif
#ifdef SM_DIGITAL_SENSORS_ENABLE
	// Perform temperature / humidity measurements.
	i2c1_status = I2C1_power_on();
	I2C1_error_check();
	sht3x_status = SHT3X_perform_measurements(SHT3X_I2C_ADDRESS);
	SHT3X_error_check();
	I2C1_power_off();
#endif
#endif /* SM */
}

#if (defined LVRM) || (defined BPSM) || (defined DDRM) || (defined RRM)
/* UPDATE MEASUREMENTS PERIOD ACCORDING TO INPUT VOLTAGE.
 * @param:	None.
 * @return:	None.
 */
static void _XM_update_measurements_period(void) {
	// Local variables.
	ADC_status_t adc1_status = ADC_SUCCESS;
	uint32_t input_voltage_mv = 0;
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
	ADC1_error_check();
	// Update periods according to input voltage.
	xm_ctx.measurements_period_seconds = (input_voltage_mv > XM_HIGH_PERIOD_THRESHOLD_MV) ? XM_MEASUREMENTS_PERIOD_HIGH_SECONDS : XM_MEASUREMENTS_PERIOD_LOW_SECONDS;
#if (defined LVRM) || (defined DDRM) || (defined RRM)
	xm_ctx.iout_indicator_enable = (input_voltage_mv > XM_HIGH_PERIOD_THRESHOLD_MV) ? 1 : 0;
#endif
}
#endif

#if (defined LVRM) || (defined DDRM) || (defined RRM)
/* MAKE IOUT INDICATOR BLINK.
 * @param:	None.
 * @return:	None.
 */
static void _XM_iout_indicator(void) {
	// Local variables.
	ADC_status_t adc1_status = ADC_SUCCESS;
	LED_status_t led_status = LED_SUCCESS;
	LED_color_t led_color;
	uint32_t iout_ua;
	uint8_t idx = XM_IOUT_INDICATOR_RANGE;
	// Read data.
	adc1_status = ADC1_get_data(ADC_DATA_INDEX_IOUT_UA, &iout_ua);
	ADC1_error_check();
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
	LED_error_check();
}
#endif

/*** MAIN function ***/

/* MAIN FUNCTION.
 * @param:	None.
 * @return:	None.
 */
int main(void) {
	// Init board.
	_XM_init_context();
	_XM_init_hw();
	// Local variables.
	RTC_status_t rtc_status = RTC_SUCCESS;
	// Perform first measurements.
	_XM_perform_measurements();
#if (defined LVRM) || (defined BPSM) || (defined DDRM) || (defined RRM)
	_XM_update_measurements_period();
#endif
	// Main loop.
	while (1) {
		// Start periodic wakeup timer.
		rtc_status = RTC_start_wakeup_timer(RTC_WAKEUP_PERIOD_SECONDS);
		RTC_error_check();
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
		// Wake-up: stop wakeup timer.
		rtc_status = RTC_stop_wakeup_timer();
		RTC_error_check();
		// Check RTC flag.
		if (RTC_get_wakeup_timer_flag() != 0) {
			// Clear flag.
			RTC_clear_wakeup_timer_flag();
			// Increment seconds count.
			xm_ctx.measurements_seconds_count += RTC_WAKEUP_PERIOD_SECONDS;
			// Check ADC period.
			if (xm_ctx.measurements_seconds_count >= xm_ctx.measurements_period_seconds) {
				// Reset count.
				xm_ctx.measurements_seconds_count = 0;
				_XM_perform_measurements();
#if (defined LVRM) || (defined BPSM) || (defined DDRM) || (defined RRM)
				_XM_update_measurements_period();
#endif
			}
#if (defined LVRM) || (defined DDRM) || (defined RRM)
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
		// Perform command task.
		AT_BUS_task();
		// Reload watchdog.
		IWDG_reload();
	}
}

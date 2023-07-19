/*
 * rtc.c
 *
 *  Created on: 16 aug. 2020
 *      Author: Ludo
 */

#include "rtc.h"

#include "exti.h"
#include "nvic.h"
#include "rcc_reg.h"
#include "rtc_reg.h"
#include "types.h"

/*** RTC local macros ***/

#define RTC_INIT_TIMEOUT_COUNT		1000

/*** RTC local global variables ***/

static volatile uint8_t rtc_wakeup_timer_flag = 0;

/*** RTC local functions ***/

/*******************************************************************/
static inline void _RTC_clear_wakeup_timer_flags(void) {
	// Clear RTC and EXTI flags.
	RTC -> ISR &= ~(0b1 << 10); // WUTF='0'.
	EXTI_clear_flag(EXTI_LINE_RTC_WAKEUP_TIMER);
}

/*******************************************************************/
void __attribute__((optimize("-O0"))) RTC_IRQHandler(void) {
	// Wake-up timer interrupt.
	if (((RTC -> ISR) & (0b1 << 10)) != 0) {
		// Set local flag.
		if (((RTC -> CR) & (0b1 << 14)) != 0) {
			rtc_wakeup_timer_flag = 1;
		}
		// Clear flags.
		_RTC_clear_wakeup_timer_flags();
	}
}

/*******************************************************************/
static RTC_status_t __attribute__((optimize("-O0"))) _RTC_enter_initialization_mode(void) {
	// Local variables.
	RTC_status_t status = RTC_SUCCESS;
	uint32_t loop_count = 0;
	// Enter key.
	RTC -> WPR = 0xCA;
	RTC -> WPR = 0x53;
	RTC -> ISR |= (0b1 << 7); // INIT='1'.
	// Wait for initialization mode.
	while (((RTC -> ISR) & (0b1 << 6)) == 0) {
		// Wait for INITF='1' or timeout.
		loop_count++;
		if (loop_count > RTC_INIT_TIMEOUT_COUNT) {
			status = RTC_ERROR_INITIALIZATION_MODE;
			break;
		}
	}
	return status;
}

/*******************************************************************/
static void __attribute__((optimize("-O0"))) _RTC_exit_initialization_mode(void) {
	RTC -> ISR &= ~(0b1 << 7); // INIT='0'.
}

/*** RTC functions ***/

/*******************************************************************/
void __attribute__((optimize("-O0"))) RTC_reset(void) {
	// Local variables.
	uint8_t j = 0;
	// Reset RTC peripheral.
	RCC -> CSR |= (0b1 << 19); // RTCRST='1'.
	for (j=0 ; j<100 ; j++);
	RCC -> CSR &= ~(0b1 << 19); // RTCRST='0'.
}

/*******************************************************************/
RTC_status_t RTC_init(void) {
	// Local variables.
	RTC_status_t status = RTC_SUCCESS;
	// Use LSE.
	RCC -> CSR |= (0b01 << 16); // RTCSEL='01'.
	// Enable RTC and register access.
	RCC -> CSR |= (0b1 << 18); // RTCEN='1'.
	// Switch to LSI if RTC failed to enter initialization mode.
	status = _RTC_enter_initialization_mode();
	if (status != RTC_SUCCESS) {
		RTC_reset();
		// Use LSI.
		RCC -> CSR &= ~(0b11 << 16); // Reset bits.
		RCC -> CSR |= (0b10 << 16); // RTCSEL='10'.
		// Enable RTC and register access.
		RCC -> CSR |= (0b1 << 18); // RTCEN='1'.
		status = _RTC_enter_initialization_mode();
		if (status != RTC_SUCCESS) goto errors;
	}
	// Compute prescaler for 32.768kHz quartz.
	RTC -> PRER = (127 << 16) | (255 << 0);
	// Force registers reset.
	RTC -> CR = 0;
	RTC -> ALRMAR = 0;
	RTC -> ALRMBR = 0;
	// Bypass shadow registers.
	RTC -> CR |= (0b1 << 5); // BYPSHAD='1'.
	// Configure wake-up timer.
	RTC -> CR |= (0b100 << 0); // Wake-up timer clocked by RTC clock (1Hz).
	_RTC_exit_initialization_mode();
	// Enable interrupt.
	RTC -> ISR &= 0xFFFE0000;
	EXTI_configure_line(EXTI_LINE_RTC_WAKEUP_TIMER, EXTI_TRIGGER_RISING_EDGE);
	NVIC_enable_interrupt(NVIC_INTERRUPT_RTC, NVIC_PRIORITY_RTC);
errors:
	return status;
}

/*******************************************************************/
RTC_status_t RTC_start_wakeup_timer(uint16_t period_seconds) {
	// Local variables.
	RTC_status_t status = RTC_SUCCESS;
	uint32_t loop_count = 0;
	// Check if timer is not already running.
	if (((RTC -> CR) & (0b1 << 10)) != 0) {
		status = RTC_ERROR_WAKEUP_TIMER_RUNNING;
		goto errors;
	}
	// Enable RTC and register access.
	status = _RTC_enter_initialization_mode();
	if (status != RTC_SUCCESS) goto errors;
	// Disable interrupt.
	RTC -> CR &= ~(0b1 << 14); // WUTE='0'.
	// Poll WUTWF flag before accessing reload register.
	while (((RTC -> ISR) & (0b1 << 2)) == 0) {
		// Wait for WUTWF='1' or timeout.
		loop_count++;
		if (loop_count > RTC_INIT_TIMEOUT_COUNT) {
			status = RTC_ERROR_WAKEUP_TIMER_REGISTER_ACCESS;
			goto errors;
		}
	}
	// Configure wake-up timer.
	RTC -> WUTR = (uint32_t) (period_seconds - 1);
	// Clear flags.
	_RTC_clear_wakeup_timer_flags();
	rtc_wakeup_timer_flag = 0;
	// Enable interrupt.
	RTC -> CR |= (0b1 << 14); // WUTE='1'.
	// Start timer.
	RTC -> CR |= (0b1 << 10); // Enable wake-up timer.
errors:
	_RTC_exit_initialization_mode();
	return status;
}

/*******************************************************************/
RTC_status_t RTC_stop_wakeup_timer(void) {
	// Local variables.
	RTC_status_t status = RTC_SUCCESS;
	// Enable RTC and register access.
	status = _RTC_enter_initialization_mode();
	if (status != RTC_SUCCESS) goto errors;
	// Disable wake-up timer.
	RTC -> CR &= ~(0b1 << 10);
	_RTC_exit_initialization_mode();
	// Disable interrupt.
	RTC -> CR &= ~(0b1 << 14); // WUTE='0'.
errors:
	return status;
}

/*******************************************************************/
volatile uint8_t RTC_get_wakeup_timer_flag(void) {
	return rtc_wakeup_timer_flag;
}

/*******************************************************************/
void RTC_clear_wakeup_timer_flag(void) {
	// Clear flag.
	rtc_wakeup_timer_flag = 0;
}

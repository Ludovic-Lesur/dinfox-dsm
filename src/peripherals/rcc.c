/*
 * rcc.c
 *
 *  Created on: 18 apr. 2020
 *      Author: Ludo
 */

#include "rcc.h"

#include "flash.h"
#include "nvic.h"
#include "pwr.h"
#include "rcc_reg.h"
#include "types.h"

/*** RCC local macros ***/

#define RCC_TIMEOUT_COUNT	1000000

/*** RCC local global variables ***/

static const uint32_t msi_range_frequency_khz[RCC_ERROR_MSI_RANGE] = {65, 131, 262, 524, 1048, 2097, 4194};
static uint32_t rcc_sysclk_khz;

/*** RCC local functions ***/

/*******************************************************************/
void RCC_IRQHandler(void) {
	// Clear all flags.
	RCC -> CICR |= (0b11 << 0);
}

/*** RCC functions ***/

/*******************************************************************/
void RCC_init(void) {
	// Default prescalers (HCLK, PCLK1 and PCLK2 must not exceed 32MHz).
	// HCLK = SYSCLK = 16MHz (HPRE='0000').
	// PCLK1 = HCLK = 16MHz (PPRE1='000').
	// PCLK2 = HCLK = 16MHz (PPRE2='000').
	// All peripherals clocked via the corresponding APBx line.
	// Reset clock is MSI 2.1MHz.
	rcc_sysclk_khz = msi_range_frequency_khz[RCC_MSI_RANGE_5_2MHZ];
}

/*******************************************************************/
RCC_status_t RCC_switch_to_hsi(void) {
	// Local variables.
	RCC_status_t status = RCC_SUCCESS;
	FLASH_status_t flash_status = FLASH_SUCCESS;
	uint32_t loop_count = 0;
	// Set flash latency.
	flash_status = FLASH_set_latency(1);
	FLASH_check_status(RCC_ERROR_BASE_FLASH);
	// Enable HSI.
	RCC -> CR |= (0b1 << 0); // Enable HSI (HSI16ON='1').
	// Wait for HSI to be stable.
	while (((RCC -> CR) & (0b1 << 2)) == 0) {
		// Wait for HSIRDYF='1' or timeout.
		loop_count++;
		if (loop_count > RCC_TIMEOUT_COUNT) {
			status = RCC_ERROR_HSI_READY;
			goto errors;
		}
	}
	// Switch SYSCLK.
	RCC -> CFGR &= ~(0b11 << 0); // Reset bits 0-1.
	RCC -> CFGR |= (0b01 << 0); // Use HSI as system clock (SW='01').
	// Wait for clock switch.
	loop_count = 0;
	while (((RCC -> CFGR) & (0b11 << 2)) != (0b01 << 2)) {
		// Wait for SWS='01' or timeout.
		loop_count++;
		if (loop_count > RCC_TIMEOUT_COUNT) {
			status = RCC_ERROR_HSI_SWITCH;
			goto errors;
		}
	}
	// Disable MSI.
	RCC -> CR &= ~(0b1 << 8); // MSION='0'.
	// Update frequency.
	rcc_sysclk_khz = RCC_HSI_FREQUENCY_KHZ;
errors:
	return status;
}

/*******************************************************************/
RCC_status_t RCC_switch_to_msi(RCC_msi_range_t msi_range) {
	// Local variables.
	RCC_status_t status = RCC_SUCCESS;
	FLASH_status_t flash_status = FLASH_SUCCESS;
	uint32_t loop_count = 0;
	// Check parameter.
	if (msi_range >= RCC_MSI_RANGE_LAST) {
		status = RCC_ERROR_MSI_RANGE;
		goto errors;
	}
	// Set frequency.
	RCC -> ICSCR &= ~(0b111 << 13);
	RCC -> ICSCR |= (msi_range << 13);
	// Enable MSI.
	RCC -> CR |= (0b1 << 8); // MSION='1'.
	// Wait for MSI to be stable.
	while (((RCC -> CR) & (0b1 << 9)) == 0) {
		// Wait for MSIRDYF='1' or timeout.
		loop_count++;
		if (loop_count > RCC_TIMEOUT_COUNT) {
			status = RCC_ERROR_MSI_READY;
			goto errors;
		}
	}
	// Switch SYSCLK.
	RCC -> CFGR &= ~(0b11 << 0); // Use MSI as system clock (SW='00').
	// Wait for clock switch.
	while (((RCC -> CFGR) & (0b11 << 2)) != (0b00 << 2)) {
		// Wait for SWS='00' or timeout.
		loop_count++;
		if (loop_count > RCC_TIMEOUT_COUNT) {
			status = RCC_ERROR_MSI_SWITCH;
			goto errors;
		}
	}
	// Set flash latency.
	flash_status = FLASH_set_latency(0);
	FLASH_check_status(RCC_ERROR_BASE_FLASH);
	// Disable HSI.
	RCC -> CR &= ~(0b1 << 0); // HSI16ON='0'.
	// Update frequency.
	rcc_sysclk_khz = msi_range_frequency_khz[msi_range];
errors:
	return status;
}

/*******************************************************************/
uint32_t RCC_get_sysclk_khz(void) {
	return rcc_sysclk_khz;
}

/*******************************************************************/
void RCC_enable_lsi(void) {
	// Enable LSI.
	RCC -> CSR |= (0b1 << 0); // LSION='1'.
	// Enable interrupt.
	RCC -> CIER |= (0b1 << 0);
	NVIC_enable_interrupt(NVIC_INTERRUPT_RCC_CRS, NVIC_PRIORTY_RCC_CRS);
	// Wait for LSI to be stable.
	while (((RCC -> CSR) & (0b1 << 1)) == 0) {
		PWR_enter_sleep_mode();
	}
	NVIC_disable_interrupt(NVIC_INTERRUPT_RCC_CRS);
}

/*******************************************************************/
void RCC_enable_lse(void) {
	// Enable LSE (32.768kHz crystal).
	RCC -> CSR |= (0b1 << 8); // LSEON='1'.
	// Enable interrupt.
	RCC -> CIER |= (0b1 << 1);
	NVIC_enable_interrupt(NVIC_INTERRUPT_RCC_CRS, NVIC_PRIORTY_RCC_CRS);
	// Wait for LSE to be stable.
	while (((RCC -> CSR) & (0b1 << 9)) == 0) {
		PWR_enter_sleep_mode();
	}
	NVIC_disable_interrupt(NVIC_INTERRUPT_RCC_CRS);
}

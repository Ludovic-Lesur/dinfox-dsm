/*
 * mapping.h
 *
 *  Created on: 18 apr. 2020
 *      Author: Ludo
 */

#ifndef __MAPPING_H__
#define __MAPPING_H__

#include "gpio.h"
#include "gpio_reg.h"
#include "mode.h"

// ADC inputs.
#if (defined LVRM) || (defined BPSM) || (defined DDRM)
static const GPIO_pin_t GPIO_ADC1_IN0 =		(GPIO_pin_t) {GPIOA, 0, 0, 0};
#endif
static const GPIO_pin_t GPIO_ADC1_IN4 =		(GPIO_pin_t) {GPIOA, 0, 4, 0};
#if (defined LVRM) || (defined BPSM) || (defined RRM)
static const GPIO_pin_t GPIO_ADC1_IN6 =		(GPIO_pin_t) {GPIOA, 0, 6, 0};
#endif
#if (defined DDRM) || (defined RRM)
static const GPIO_pin_t GPIO_ADC1_IN7 =		(GPIO_pin_t) {GPIOA, 0, 7, 0};
#endif
#ifdef BPSM
// Monitoring enable.
static const GPIO_pin_t GPIO_MNTR_EN =		(GPIO_pin_t) {GPIOA, 0, 1, 0};
// Charge control and status.
static const GPIO_pin_t GPIO_CHRG_ST=		(GPIO_pin_t) {GPIOA, 0, 8, 0};
static const GPIO_pin_t GPIO_CHRG_EN =		(GPIO_pin_t) {GPIOA, 0, 9, 0};
#endif
// Output enable.
#ifdef LVRM
static const GPIO_pin_t GPIO_OUT_EN =		(GPIO_pin_t) {GPIOA, 0, 7, 0};
#endif
#ifdef BPSM
static const GPIO_pin_t GPIO_OUT_EN =		(GPIO_pin_t) {GPIOA, 0, 5, 0};
#endif
#ifdef DDRM
static const GPIO_pin_t GPIO_OUT_EN =		(GPIO_pin_t) {GPIOA, 0, 6, 0};
#endif
#ifdef RRM
static const GPIO_pin_t GPIO_OUT_EN =		(GPIO_pin_t) {GPIOA, 0, 0, 0};
#endif
// LPUART1 (RS485).
static const GPIO_pin_t GPIO_LPUART1_TX =	(GPIO_pin_t) {GPIOB, 1, 6, 6}; // AF6 = LPUART1_TX.
static const GPIO_pin_t GPIO_LPUART1_RX =	(GPIO_pin_t) {GPIOB, 1, 7, 6}; // AF6 = LPUART1_RX.
static const GPIO_pin_t GPIO_LPUART1_DE =	(GPIO_pin_t) {GPIOB, 1, 1, 4}; // AF4 = LPUART1_DE.
#if (defined LVRM) || (defined DDRM) || (defined RRM)
static const GPIO_pin_t GPIO_LPUART1_NRE =	(GPIO_pin_t) {GPIOA, 0, 9, 0};
#endif
#ifdef BPSM
static const GPIO_pin_t GPIO_LPUART1_NRE =	(GPIO_pin_t) {GPIOA, 0, 10, 0};
#endif
// RGB LED.
#ifdef LVRM
static const GPIO_pin_t GPIO_LED_RED =		(GPIO_pin_t) {GPIOA, 0, 10, 5}; // AF5 = TIM2_CH3.
static const GPIO_pin_t GPIO_LED_GREEN =	(GPIO_pin_t) {GPIOA, 0, 1, 2}; // AF2 = TIM2_CH2.
static const GPIO_pin_t GPIO_LED_BLUE =		(GPIO_pin_t) {GPIOA, 0, 5, 5}; // AF5 = TIM2_CH1.
#endif
#ifdef DDRM
static const GPIO_pin_t GPIO_LED_RED =		(GPIO_pin_t) {GPIOA, 0, 10, 5}; // AF5 = TIM2_CH3.
static const GPIO_pin_t GPIO_LED_GREEN =	(GPIO_pin_t) {GPIOA, 0, 5, 5}; // AF5 = TIM2_CH1.
static const GPIO_pin_t GPIO_LED_BLUE =		(GPIO_pin_t) {GPIOA, 0, 1, 2}; // AF2 = TIM2_CH2.
#endif
#ifdef RRM
static const GPIO_pin_t GPIO_LED_RED =		(GPIO_pin_t) {GPIOA, 0, 1, 2}; // AF2 = TIM2_CH2.
static const GPIO_pin_t GPIO_LED_GREEN =	(GPIO_pin_t) {GPIOA, 0, 10, 5}; // AF5 = TIM2_CH3.
static const GPIO_pin_t GPIO_LED_BLUE =		(GPIO_pin_t) {GPIOA, 0, 5, 5}; // AF5 = TIM2_CH1.
#endif
// Programming.
static const GPIO_pin_t GPIO_SWDIO =		(GPIO_pin_t) {GPIOA, 0, 13, 0};
static const GPIO_pin_t GPIO_SWCLK =		(GPIO_pin_t) {GPIOA, 0, 14, 0};

#endif /* __MAPPING_H__ */

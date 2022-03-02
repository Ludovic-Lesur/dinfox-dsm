/*
 * mapping.h
 *
 *  Created on: 18 apr. 2020
 *      Author: Ludo
 */

#ifndef MAPPING_H
#define MAPPING_H

#include "gpio.h"
#include "gpio_reg.h"

// ADC inputs.
static const GPIO GPIO_ADC1_IN0 =		(GPIO) {GPIOA, 0, 0, 0};
static const GPIO GPIO_ADC1_IN4 =		(GPIO) {GPIOA, 0, 4, 0};
static const GPIO GPIO_ADC1_IN6 =		(GPIO) {GPIOA, 0, 6, 0};
// DC-DC enable.
static const GPIO GPIO_OUT_EN =			(GPIO) {GPIOA, 0, 7, 0};
// LPUART1 (RS485).
static const GPIO GPIO_LPUART1_TX =		(GPIO) {GPIOB, 1, 6, 6}; // AF6 = LPUART1_TX.
static const GPIO GPIO_LPUART1_RX =		(GPIO) {GPIOB, 1, 7, 6}; // AF6 = LPUART1_RX.
static const GPIO GPIO_LPUART1_DE =		(GPIO) {GPIOB, 1, 1, 4}; // AF4 = LPUART1_DE.
static const GPIO GPIO_LPUART1_NRE =	(GPIO) {GPIOA, 0, 9, 0};
// RGB LED.
static const GPIO GPIO_LED_RED =		(GPIO) {GPIOA, 0, 10, 5}; // AF5 = TIM2_CH3.
static const GPIO GPIO_LED_GREEN =		(GPIO) {GPIOA, 0, 1, 2}; // AF2 = TIM2_CH2.
static const GPIO GPIO_LED_BLUE =		(GPIO) {GPIOA, 0, 5, 5}; // AF5 = TIM2_CH1.
// Programming.
static const GPIO GPIO_SWDIO =			(GPIO) {GPIOA, 0, 13, 0};
static const GPIO GPIO_SWCLK =			(GPIO) {GPIOA, 0, 14, 0};

#endif /* MAPPING_H */

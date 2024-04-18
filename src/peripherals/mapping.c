/*
 * mapping.c
 *
 *  Created on: 18 apr. 2024
 *      Author: Ludo
 */

#include "mapping.h"

#include "gpio.h"
#include "gpio_reg.h"

// ADC inputs.
#if (defined LVRM) || (defined BPSM) || (defined DDRM) || (defined GPSM)
const GPIO_pin_t GPIO_ADC1_IN0 =			(GPIO_pin_t) {GPIOA, 0, 0, 0};
#endif
#if ((defined LVRM) && (defined HW2_0)) || (defined GPSM)
const GPIO_pin_t GPIO_ADC1_IN1 =			(GPIO_pin_t) {GPIOA, 0, 1, 0};
#endif
#if ((defined LVRM) && (defined HW1_0)) || (defined BPSM) || (defined DDRM) || (defined RRM)
const GPIO_pin_t GPIO_ADC1_IN4 =			(GPIO_pin_t) {GPIOA, 0, 4, 0};
#endif
#if (defined LVRM) || (defined BPSM) || (defined RRM)
const GPIO_pin_t GPIO_ADC1_IN6 =			(GPIO_pin_t) {GPIOA, 0, 6, 0};
#endif
#if (defined DDRM) || (defined RRM) || (defined UHFM)
const GPIO_pin_t GPIO_ADC1_IN7 =			(GPIO_pin_t) {GPIOA, 0, 7, 0};
#endif
// Monitoring enable.
#if (defined LVRM) && (defined HW2_0)
const GPIO_pin_t GPIO_MNTR_EN =				(GPIO_pin_t) {GPIOB, 1, 7, 0};
#endif
// Charge control and status.
#ifdef BPSM
const GPIO_pin_t GPIO_MNTR_EN =				(GPIO_pin_t) {GPIOA, 0, 1, 0};
const GPIO_pin_t GPIO_CHRG_ST =				(GPIO_pin_t) {GPIOA, 0, 8, 0};
const GPIO_pin_t GPIO_CHRG_EN =				(GPIO_pin_t) {GPIOA, 0, 9, 0};
#endif
// Analog front-end.
#ifdef SM
const GPIO_pin_t GPIO_ANA_POWER_ENABLE =	(GPIO_pin_t) {GPIOA, 0, 8, 0};
const GPIO_pin_t GPIO_AIN0 =				(GPIO_pin_t) {GPIOA, 0, 5, 0};
const GPIO_pin_t GPIO_AIN1 =				(GPIO_pin_t) {GPIOA, 0, 6, 0};
const GPIO_pin_t GPIO_AIN2 =				(GPIO_pin_t) {GPIOA, 0, 7, 0};
const GPIO_pin_t GPIO_AIN3 =				(GPIO_pin_t) {GPIOB, 1, 0, 0};
#endif
// Digital front-end.
#ifdef SM
const GPIO_pin_t GPIO_DIG_POWER_ENABLE =	(GPIO_pin_t) {GPIOB, 1, 3, 0};
const GPIO_pin_t GPIO_DIO0 =				(GPIO_pin_t) {GPIOA, 0, 0, 0};
const GPIO_pin_t GPIO_DIO1 =				(GPIO_pin_t) {GPIOA, 0, 1, 0};
const GPIO_pin_t GPIO_DIO2 =				(GPIO_pin_t) {GPIOA, 0, 9, 0};
const GPIO_pin_t GPIO_DIO3 =				(GPIO_pin_t) {GPIOA, 0, 10, 0};
#endif
// Digital sensors.
#ifdef SM
const GPIO_pin_t GPIO_SEN_POWER_ENABLE =	(GPIO_pin_t) {GPIOA, 0, 15, 0};
const GPIO_pin_t GPIO_I2C1_SCL =			(GPIO_pin_t) {GPIOB, 1, 6, 1}; // AF1 = I2C1_SCL.
const GPIO_pin_t GPIO_I2C1_SDA =			(GPIO_pin_t) {GPIOB, 1, 7, 1}; // AF1 = I2C1_SDA.
#endif
// Load control.
#if (defined LVRM) && (defined HW1_0)
const GPIO_pin_t GPIO_OUT_EN =				(GPIO_pin_t) {GPIOA, 0, 7, 0};
#endif
#if (defined LVRM) && (defined HW2_0)
const GPIO_pin_t GPIO_DC_DC_POWER_ENABLE =	(GPIO_pin_t) {GPIOB, 1, 6, 0};
const GPIO_pin_t GPIO_COIL_POWER_ENABLE =	(GPIO_pin_t) {GPIOA, 0, 9, 0};
const GPIO_pin_t GPIO_OUT_SELECT =			(GPIO_pin_t) {GPIOA, 0, 10, 0};
const GPIO_pin_t GPIO_OUT_CONTROL =			(GPIO_pin_t) {GPIOA, 0, 15, 0};
#endif
#ifdef BPSM
const GPIO_pin_t GPIO_OUT_EN =				(GPIO_pin_t) {GPIOA, 0, 5, 0};
#endif
#ifdef DDRM
const GPIO_pin_t GPIO_OUT_EN =				(GPIO_pin_t) {GPIOA, 0, 6, 0};
#endif
#ifdef RRM
const GPIO_pin_t GPIO_OUT_EN =				(GPIO_pin_t) {GPIOA, 0, 0, 0};
#endif
// Radio.
#ifdef UHFM
const GPIO_pin_t GPIO_SPI1_SCK = 			(GPIO_pin_t) {GPIOB, 1, 3, 0};
const GPIO_pin_t GPIO_SPI1_MISO = 			(GPIO_pin_t) {GPIOB, 1, 4, 0};
const GPIO_pin_t GPIO_SPI1_MOSI = 			(GPIO_pin_t) {GPIOB, 1, 5, 0};
const GPIO_pin_t GPIO_S2LP_CS = 			(GPIO_pin_t) {GPIOA, 0, 15, 0};
// RF power enable.
const GPIO_pin_t GPIO_RF_POWER_ENABLE =		(GPIO_pin_t) {GPIOB, 1, 8, 0};
const GPIO_pin_t GPIO_RF_TX_ENABLE =		(GPIO_pin_t) {GPIOB, 1, 7, 0};
const GPIO_pin_t GPIO_RF_RX_ENABLE =		(GPIO_pin_t) {GPIOB, 1, 6, 0};
// TCXO power control.
const GPIO_pin_t GPIO_TCXO_POWER_ENABLE =	(GPIO_pin_t) {GPIOA, 0, 8, 0};
// S2LP GPIOs.
const GPIO_pin_t GPIO_S2LP_SDN =			(GPIO_pin_t) {GPIOA, 0, 9, 0};
const GPIO_pin_t GPIO_S2LP_GPIO0 =			(GPIO_pin_t) {GPIOA, 0, 11, 0};
#endif
// GPS.
#ifdef GPSM
const GPIO_pin_t GPIO_GPS_POWER_ENABLE =	(GPIO_pin_t) {GPIOB, 1, 7, 0};
const GPIO_pin_t GPIO_GPS_RESET =			(GPIO_pin_t) {GPIOB, 1, 3, 0};
const GPIO_pin_t GPIO_GPS_VBCKP =			(GPIO_pin_t) {GPIOA, 0, 8, 0};
const GPIO_pin_t GPIO_GPS_TIMEPULSE =		(GPIO_pin_t) {GPIOA, 0, 15, 0};
const GPIO_pin_t GPIO_ANT_POWER_ENABLE =	(GPIO_pin_t) {GPIOB, 1, 6, 0};
const GPIO_pin_t GPIO_USART2_TX =			(GPIO_pin_t) {GPIOA, 0, 9, 4}; // AF4 = USART2_TX.
const GPIO_pin_t GPIO_USART2_RX =			(GPIO_pin_t) {GPIOA, 0, 10, 4}; // AF4 = USART2_RX.
#endif
// LPUART1 (RS485).
#if ((defined LVRM) && (defined HW1_0)) || (defined BPSM) || (defined DDRM) || (defined RRM)
const GPIO_pin_t GPIO_LPUART1_TX =			(GPIO_pin_t) {GPIOB, 1, 6, 6}; // AF6 = LPUART1_TX.
const GPIO_pin_t GPIO_LPUART1_RX =			(GPIO_pin_t) {GPIOB, 1, 7, 6}; // AF6 = LPUART1_RX.

#endif
#if ((defined LVRM) && (defined HW2_0)) || (defined SM) || (defined UHFM) || (defined GPSM)
const GPIO_pin_t GPIO_LPUART1_TX =			(GPIO_pin_t) {GPIOA, 0, 2, 6}; // AF6 = LPUART1_TX.
const GPIO_pin_t GPIO_LPUART1_RX =			(GPIO_pin_t) {GPIOA, 0, 3, 6}; // AF6 = LPUART1_RX.
#endif
const GPIO_pin_t GPIO_LPUART1_DE =			(GPIO_pin_t) {GPIOB, 1, 1, 4}; // AF4 = LPUART1_DE.
#if ((defined LVRM) && (defined HW1_0)) || (defined DDRM) || (defined RRM)
const GPIO_pin_t GPIO_LPUART1_NRE =			(GPIO_pin_t) {GPIOA, 0, 9, 0};
#endif
#ifdef BPSM
const GPIO_pin_t GPIO_LPUART1_NRE =			(GPIO_pin_t) {GPIOA, 0, 10, 0};
#endif
#if (defined SM) || (defined GPSM)
const GPIO_pin_t GPIO_LPUART1_NRE =			(GPIO_pin_t) {GPIOA, 0, 4, 0};
#endif
#ifdef UHFM
const GPIO_pin_t GPIO_LPUART1_NRE =			(GPIO_pin_t) {GPIOB, 1, 2, 0};
#endif
#if (defined LVRM) && (defined HW2_0)
const GPIO_pin_t GPIO_LPUART1_NRE =			(GPIO_pin_t) {GPIOA, 0, 7, 0};
#endif
// RGB LED.
#if (defined LVRM) && (defined HW1_0)
const GPIO_pin_t GPIO_LED_RED =				(GPIO_pin_t) {GPIOA, 0, 10, 5}; // AF5 = TIM2_CH3.
const GPIO_pin_t GPIO_LED_GREEN =			(GPIO_pin_t) {GPIOA, 0, 1, 2}; // AF2 = TIM2_CH2.
const GPIO_pin_t GPIO_LED_BLUE =			(GPIO_pin_t) {GPIOA, 0, 5, 5}; // AF5 = TIM2_CH1.
#endif
#if (defined LVRM) && (defined HW2_0)
const GPIO_pin_t GPIO_LED_RED =				(GPIO_pin_t) {GPIOB, 1, 3, 2}; // AF2 = TIM2_CH2.
const GPIO_pin_t GPIO_LED_GREEN =			(GPIO_pin_t) {GPIOB, 1, 0, 5}; // AF5 = TIM2_CH3.
const GPIO_pin_t GPIO_LED_BLUE =			(GPIO_pin_t) {GPIOA, 0, 8, 5}; // AF5 = TIM2_CH1.
#endif
#ifdef DDRM
const GPIO_pin_t GPIO_LED_RED =				(GPIO_pin_t) {GPIOA, 0, 10, 5}; // AF5 = TIM2_CH3.
const GPIO_pin_t GPIO_LED_GREEN =			(GPIO_pin_t) {GPIOA, 0, 5, 5}; // AF5 = TIM2_CH1.
const GPIO_pin_t GPIO_LED_BLUE =			(GPIO_pin_t) {GPIOA, 0, 1, 2}; // AF2 = TIM2_CH2.
#endif
#ifdef RRM
const GPIO_pin_t GPIO_LED_RED =				(GPIO_pin_t) {GPIOA, 0, 1, 2}; // AF2 = TIM2_CH2.
const GPIO_pin_t GPIO_LED_GREEN =			(GPIO_pin_t) {GPIOA, 0, 10, 5}; // AF5 = TIM2_CH3.
const GPIO_pin_t GPIO_LED_BLUE =			(GPIO_pin_t) {GPIOA, 0, 5, 5}; // AF5 = TIM2_CH1.
#endif
#ifdef GPSM
const GPIO_pin_t GPIO_LED_RED =				(GPIO_pin_t) {GPIOA, 0, 6, 2}; // AF5 = TIM22_CH1.
const GPIO_pin_t GPIO_LED_GREEN =			(GPIO_pin_t) {GPIOA, 0, 7, 5}; // AF5 = TIM22_CH2.
const GPIO_pin_t GPIO_LED_BLUE =			(GPIO_pin_t) {GPIOB, 1, 0, 5}; // AF5 = TIM2_CH3.
#endif
// MUX control.
#if (defined LVRM) && (defined HW2_0)
const GPIO_pin_t GPIO_MUX_SEL =				(GPIO_pin_t) {GPIOA, 0, 4, 0};
const GPIO_pin_t GPIO_MUX_EN =				(GPIO_pin_t) {GPIOA, 0, 5, 0};
#endif
// Test points.
#ifdef UHFM
const GPIO_pin_t GPIO_TP1 =					(GPIO_pin_t) {GPIOA, 0, 0, 0};
const GPIO_pin_t GPIO_TP2 =					(GPIO_pin_t) {GPIOA, 0, 5, 0};
const GPIO_pin_t GPIO_TP3 =					(GPIO_pin_t) {GPIOA, 0, 12, 0};
#endif
#ifdef GPSM
const GPIO_pin_t GPIO_TP1 =					(GPIO_pin_t) {GPIOA, 0, 5, 0};
#endif
// Programming.
const GPIO_pin_t GPIO_SWDIO =				(GPIO_pin_t) {GPIOA, 0, 13, 0};
const GPIO_pin_t GPIO_SWCLK =				(GPIO_pin_t) {GPIOA, 0, 14, 0};

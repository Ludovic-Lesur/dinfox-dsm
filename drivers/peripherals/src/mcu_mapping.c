/*
 * mcu_mapping.c
 *
 *  Created on: 18 apr. 2024
 *      Author: Ludo
 */

#include "mcu_mapping.h"

#include "adc.h"
#include "dsm_flags.h"
#include "gpio.h"
#include "gpio_registers.h"
#ifndef MPMCM
#include "i2c.h"
#endif
#include "lpuart.h"
#ifndef MPMCM
#include "spi.h"
#endif
#include "tim.h"
#include "usart.h"

/*** MCU MAPPING local global variables ***/

// LPUART1.
#if (((defined LVRM) && (defined HW1_0)) || (defined BPSM) || (defined DDRM) || (defined RRM))
static const GPIO_pin_t GPIO_LPUART1_TX = { GPIOB, 1, 6, 6 };
static const GPIO_pin_t GPIO_LPUART1_RX = { GPIOB, 1, 7, 6 };
#endif
#if (((defined LVRM) && (defined HW2_0)) || (defined BCM) || (defined SM) || (defined UHFM) || (defined GPSM))
static const GPIO_pin_t GPIO_LPUART1_TX = { GPIOA, 0, 2, 6 };
static const GPIO_pin_t GPIO_LPUART1_RX = { GPIOA, 0, 3, 6 };
#endif
#ifdef MPMCM
static const GPIO_pin_t GPIO_LPUART1_TX = { GPIOB, 1, 11, 8 };
static const GPIO_pin_t GPIO_LPUART1_RX = { GPIOB, 1, 10, 8 };
#endif
#ifdef MPMCM
static const GPIO_pin_t GPIO_LPUART1_DE = { GPIOB, 1, 12, 8 };
#else
static const GPIO_pin_t GPIO_LPUART1_DE = { GPIOB, 1, 1, 4 };
#endif
#if (((defined LVRM) && (defined HW1_0)) || (defined DDRM) || (defined RRM))
static const GPIO_pin_t GPIO_LPUART1_NRE = { GPIOA, 0, 9, 0 };
#endif
#if ((defined LVRM) && (defined HW2_0))
static const GPIO_pin_t GPIO_LPUART1_NRE = { GPIOA, 0, 7, 0 };
#endif
#ifdef BPSM
static const GPIO_pin_t GPIO_LPUART1_NRE = { GPIOA, 0, 10, 0 };
#endif
#ifdef UHFM
static const GPIO_pin_t GPIO_LPUART1_NRE = { GPIOB, 1, 2, 0 };
#endif
#if ((defined BCM) || (defined SM) || (defined GPSM))
static const GPIO_pin_t GPIO_LPUART1_NRE = { GPIOA, 0, 4, 0 };
#endif
#ifdef MPMCM
static const GPIO_pin_t GPIO_LPUART1_NRE = { GPIOB, 1, 13, 0 };
#endif
// ADC.
#if ((defined LVRM) || (defined DDRM) || (defined RRM))
#if ((defined LVRM) && (defined HW1_0))
static const GPIO_pin_t GPIO_ADC_VIN_MEASURE =  { GPIOA, 0, 6, 0 };
static const GPIO_pin_t GPIO_ADC_VOUT_MEASURE = { GPIOA, 0, 4, 0 };
static const GPIO_pin_t GPIO_ADC_IOUT_MEASURE = { GPIOA, 0, 0, 0 };
#endif
#if ((defined LVRM) && (defined HW2_0))
static const GPIO_pin_t GPIO_ADC_VIN_MEASURE =  { GPIOA, 0, 0, 0 };
static const GPIO_pin_t GPIO_ADC_VOUT_MEASURE = { GPIOA, 0, 6, 0 };
static const GPIO_pin_t GPIO_ADC_IOUT_MEASURE = { GPIOA, 0, 1, 0 };
#endif
#ifdef DDRM
static const GPIO_pin_t GPIO_ADC_VIN_MEASURE =  { GPIOA, 0, 7, 0 };
static const GPIO_pin_t GPIO_ADC_VOUT_MEASURE = { GPIOA, 0, 4, 0 };
static const GPIO_pin_t GPIO_ADC_IOUT_MEASURE = { GPIOA, 0, 0, 0 };
#endif
#ifdef RRM
static const GPIO_pin_t GPIO_ADC_VIN_MEASURE =  { GPIOA, 0, 7, 0 };
static const GPIO_pin_t GPIO_ADC_VOUT_MEASURE = { GPIOA, 0, 6, 0 };
static const GPIO_pin_t GPIO_ADC_IOUT_MEASURE = { GPIOA, 0, 4, 0 };
#endif
static const GPIO_pin_t* const GPIO_ADC_PINS_LIST[ADC_CHANNEL_INDEX_LAST] = { &GPIO_ADC_VIN_MEASURE, &GPIO_ADC_VOUT_MEASURE, &GPIO_ADC_IOUT_MEASURE };
#endif
#ifdef BPSM
static const GPIO_pin_t GPIO_ADC_VSRC_MEASURE = { GPIOA, 0, 6, 0 };
static const GPIO_pin_t GPIO_ADC_VSTR_MEASURE = { GPIOA, 0, 4, 0 };
static const GPIO_pin_t GPIO_ADC_VBKP_MEASURE = { GPIOA, 0, 0, 0 };
static const GPIO_pin_t* const GPIO_ADC_PINS_LIST[ADC_CHANNEL_INDEX_LAST] = { &GPIO_ADC_VSRC_MEASURE, &GPIO_ADC_VSTR_MEASURE, &GPIO_ADC_VBKP_MEASURE };
#endif
#ifdef UHFM
static const GPIO_pin_t GPIO_ADC_VRF_MEASURE = { GPIOA, 0, 7, 0 };
static const GPIO_pin_t* const GPIO_ADC_PINS_LIST[ADC_CHANNEL_INDEX_LAST] = { &GPIO_ADC_VRF_MEASURE };
#endif
#ifdef GPSM
static const GPIO_pin_t GPIO_ADC_VGPS_MEASURE = { GPIOA, 0, 0, 0 };
static const GPIO_pin_t GPIO_ADC_VANT_MEASURE = { GPIOA, 0, 1, 0 };
static const GPIO_pin_t* const GPIO_ADC_PINS_LIST[ADC_CHANNEL_INDEX_LAST] = { &GPIO_ADC_VGPS_MEASURE, &GPIO_ADC_VANT_MEASURE };
#endif
#ifdef SM
static const GPIO_pin_t GPIO_ADC_AIN0 = { GPIOA, 0, 5, 0 };
static const GPIO_pin_t GPIO_ADC_AIN1 = { GPIOA, 0, 6, 0 };
static const GPIO_pin_t GPIO_ADC_AIN2 = { GPIOA, 0, 7, 0 };
static const GPIO_pin_t GPIO_ADC_AIN3 = { GPIOB, 1, 0, 0 };
static const GPIO_pin_t* const GPIO_ADC_PINS_LIST[ADC_CHANNEL_INDEX_LAST] = { &GPIO_ADC_AIN0, &GPIO_ADC_AIN1, &GPIO_ADC_AIN2, &GPIO_ADC_AIN3 };
#endif
#ifdef MPMCM
static const GPIO_pin_t GPIO_ACV_SAMPLING = { GPIOA, 0, 0, 0 };
static const GPIO_pin_t GPIO_ACI1_SAMPLING = { GPIOA, 0, 1, 0 };
static const GPIO_pin_t GPIO_ACI2_SAMPLING = { GPIOA, 0, 4, 0 };
static const GPIO_pin_t GPIO_ACI3_SAMPLING = { GPIOA, 0, 6, 0 };
static const GPIO_pin_t GPIO_ACI4_SAMPLING = { GPIOA, 0, 7, 0 };
static const GPIO_pin_t* const GPIO_ADC_PINS_LIST[ADC_CHANNEL_INDEX_LAST] = { &GPIO_ACV_SAMPLING, &GPIO_ACI1_SAMPLING, &GPIO_ACI2_SAMPLING, &GPIO_ACI3_SAMPLING, &GPIO_ACI4_SAMPLING };
#endif
#ifdef BCM
static const GPIO_pin_t GPIO_ADC_VSRC_MEASURE = { GPIOB, 1, 0, 0 };
static const GPIO_pin_t GPIO_ADC_VSTR_MEASURE = { GPIOA, 0, 7, 0 };
static const GPIO_pin_t GPIO_ADC_ISTR_MEASURE = { GPIOA, 0, 1, 0 };
static const GPIO_pin_t GPIO_ADC_VBKP_MEASURE = { GPIOA, 0, 6, 0 };
static const GPIO_pin_t* const GPIO_ADC_PINS_LIST[ADC_CHANNEL_INDEX_LAST] = { &GPIO_ADC_VSRC_MEASURE, &GPIO_ADC_VSTR_MEASURE, &GPIO_ADC_ISTR_MEASURE, &GPIO_ADC_VBKP_MEASURE };
#endif
#ifdef DSM_RGB_LED
// RGB LED.
#if (defined LVRM) && (defined HW1_0)
static const GPIO_pin_t GPIO_LED_RED =   { GPIOA, 0, 10, 5 };
static const GPIO_pin_t GPIO_LED_GREEN = { GPIOA, 0, 1, 2 };
static const GPIO_pin_t GPIO_LED_BLUE =  { GPIOA, 0, 5, 5 };
#endif
#if (defined LVRM) && (defined HW2_0)
static const GPIO_pin_t GPIO_LED_RED =   { GPIOB, 1, 3, 2 };
static const GPIO_pin_t GPIO_LED_GREEN = { GPIOB, 1, 0, 5 };
static const GPIO_pin_t GPIO_LED_BLUE =  { GPIOA, 0, 8, 5 };
#endif
#ifdef DDRM
static const GPIO_pin_t GPIO_LED_RED =   { GPIOA, 0, 10, 5 };
static const GPIO_pin_t GPIO_LED_GREEN = { GPIOA, 0, 5, 5 };
static const GPIO_pin_t GPIO_LED_BLUE =  { GPIOA, 0, 1, 2 };
#endif
#ifdef GPSM
static const GPIO_pin_t GPIO_LED_RED =   { GPIOA, 0, 6, 5 };
static const GPIO_pin_t GPIO_LED_GREEN = { GPIOA, 0, 7, 5 };
static const GPIO_pin_t GPIO_LED_BLUE =  { GPIOB, 1, 0, 5 };
#endif
#ifdef RRM
static const GPIO_pin_t GPIO_LED_RED =   { GPIOA, 0, 1, 2 };
static const GPIO_pin_t GPIO_LED_GREEN = { GPIOA, 0, 10, 5 };
static const GPIO_pin_t GPIO_LED_BLUE =  { GPIOA, 0, 5, 5 };
#endif
#ifdef MPMCM
static const GPIO_pin_t GPIO_LED_RED =   { GPIOB, 1, 6, 2 };
static const GPIO_pin_t GPIO_LED_GREEN = { GPIOB, 1, 7, 2 };
static const GPIO_pin_t GPIO_LED_BLUE =  { GPIOB, 1, 9, 2 };
#endif
#ifdef BCM
static const GPIO_pin_t GPIO_LED_RED =   { GPIOA, 0, 15, 5 };
static const GPIO_pin_t GPIO_LED_GREEN = { GPIOB, 1, 3, 2 };
static const GPIO_pin_t GPIO_LED_BLUE =  { GPIOB, 1, 7, 5 };
#endif
// Timer channels.
static const TIM_channel_gpio_t TIM_CHANNEL_GPIO_LED_RED = { TIM_CHANNEL_LED_RED, &GPIO_LED_RED, TIM_POLARITY_ACTIVE_LOW };
static const TIM_channel_gpio_t TIM_CHANNEL_GPIO_LED_GREEN = { TIM_CHANNEL_LED_GREEN, &GPIO_LED_GREEN, TIM_POLARITY_ACTIVE_LOW };
static const TIM_channel_gpio_t TIM_CHANNEL_GPIO_LED_BLUE = { TIM_CHANNEL_LED_BLUE, &GPIO_LED_BLUE, TIM_POLARITY_ACTIVE_LOW };
// Timer pins list.
#ifdef GPSM
static const TIM_channel_gpio_t* const TIM_CHANNEL_GPIO_LIST_LED_RG[TIM_CHANNEL_INDEX_LED_RG_LAST] = { &TIM_CHANNEL_GPIO_LED_RED, &TIM_CHANNEL_GPIO_LED_GREEN };
static const TIM_channel_gpio_t* const TIM_CHANNEL_GPIO_LIST_LED_B[TIM_CHANNEL_INDEX_LED_B_LAST] = { &TIM_CHANNEL_GPIO_LED_BLUE };
#else
static const TIM_channel_gpio_t* const TIM_CHANNEL_GPIO_LIST_LED[TIM_CHANNEL_INDEX_LED_LAST] = { &TIM_CHANNEL_GPIO_LED_RED, &TIM_CHANNEL_GPIO_LED_GREEN, &TIM_CHANNEL_GPIO_LED_BLUE };
#endif
#endif
#ifdef UHFM
// SPI1.
static const GPIO_pin_t GPIO_SPI1_SCK =  { GPIOB, 1, 3, 0 };
static const GPIO_pin_t GPIO_SPI1_MISO = { GPIOB, 1, 4, 0 };
static const GPIO_pin_t GPIO_SPI1_MOSI = { GPIOB, 1, 5, 0 };
#endif
#ifdef GPSM
// USART2.
static const GPIO_pin_t GPIO_USART2_TX = { GPIOA, 0, 9, 4 };
static const GPIO_pin_t GPIO_USART2_RX = { GPIOA, 0, 10, 4 };
#endif
#ifdef SM
// I2C1.
const GPIO_pin_t GPIO_I2C1_SCL = { GPIOB, 1, 6, 1 };
const GPIO_pin_t GPIO_I2C1_SDA = { GPIOB, 1, 7, 1 };
#endif
#ifdef MPMCM
// Frequency measure.
static const GPIO_pin_t GPIO_ACV_FREQUENCY = { GPIOA, 0, 5, 1 };
static const TIM_channel_gpio_t TIM_CHANNEL_GPIO_ACV_FREQUENCY = { TIM_CHANNEL_ACV_FREQUENCY, &GPIO_ACV_FREQUENCY, TIM_POLARITY_ACTIVE_HIGH };
static const TIM_channel_gpio_t* const TIM_CHANNEL_GPIO_LIST_ACV_FREQUENCY[TIM_CHANNEL_INDEX_ACV_FREQUENCY_LAST] = { &TIM_CHANNEL_GPIO_ACV_FREQUENCY };
// USART2.
static const GPIO_pin_t GPIO_USART2_TX = { GPIOB, 1, 3, 7 };
static const GPIO_pin_t GPIO_USART2_RX = { GPIOA, 0, 15, 7 };
#endif

/*** MCU MAPPING global variables ***/

// RS485.
const LPUART_gpio_t LPUART_GPIO_RS485 = { &GPIO_LPUART1_TX, &GPIO_LPUART1_RX, &GPIO_LPUART1_DE, &GPIO_LPUART1_NRE };
// Analog inputs.
#ifdef BCM
const GPIO_pin_t GPIO_MNTR_EN = { GPIOA, 0, 5, 0 };
#endif
#ifdef BPSM
const GPIO_pin_t GPIO_MNTR_EN = { GPIOA, 0, 1, 0 };
#endif
#if ((defined LVRM) && (defined HW2_0))
const GPIO_pin_t GPIO_MNTR_EN = { GPIOB, 1, 7, 0 };
#endif
#ifdef SM
const GPIO_pin_t GPIO_ANALOG_POWER_ENABLE = { GPIOA, 0, 8, 0 };
#endif
#ifdef MPMCM
const GPIO_pin_t GPIO_ANALOG_POWER_ENABLE = { GPIOB, 1, 15, 0 };
#endif
const ADC_gpio_t ADC_GPIO = { (const GPIO_pin_t**) &GPIO_ADC_PINS_LIST, ADC_CHANNEL_INDEX_LAST };
#ifdef DSM_RGB_LED
// RGB LED.
#ifdef GPSM
const TIM_gpio_t TIM_GPIO_LED_RG = { (const TIM_channel_gpio_t**) &TIM_CHANNEL_GPIO_LIST_LED_RG, TIM_CHANNEL_INDEX_LED_RG_LAST };
const TIM_gpio_t TIM_GPIO_LED_B = { (const TIM_channel_gpio_t**) &TIM_CHANNEL_GPIO_LIST_LED_B, TIM_CHANNEL_INDEX_LED_B_LAST };
#else
const TIM_gpio_t TIM_GPIO_LED = { (const TIM_channel_gpio_t**) &TIM_CHANNEL_GPIO_LIST_LED, TIM_CHANNEL_INDEX_LED_LAST };
#endif
#endif
#if (defined LVRM) && (defined HW1_0)
const GPIO_pin_t GPIO_OUT_EN = { GPIOA, 0, 7, 0 };
#endif
#ifdef DDRM
const GPIO_pin_t GPIO_OUT_EN = { GPIOA, 0, 6, 0 };
#endif
#ifdef BPSM
const GPIO_pin_t GPIO_OUT_EN = { GPIOA, 0, 5, 0 };
#endif
#ifdef RRM
const GPIO_pin_t GPIO_OUT_EN = { GPIOA, 0, 0, 0 };
#endif
#ifdef BCM
const GPIO_pin_t GPIO_OUT_EN = { GPIOA, 0, 10, 0 };
#endif
#if (defined LVRM) && (defined HW2_0)
// Bistable relay control.
const GPIO_pin_t GPIO_DC_DC_POWER_ENABLE = { GPIOB, 1, 6, 0 };
const GPIO_pin_t GPIO_COIL_POWER_ENABLE = { GPIOA, 0, 9, 0 };
const GPIO_pin_t GPIO_OUT_SELECT = { GPIOA, 0, 10, 0 };
const GPIO_pin_t GPIO_OUT_CONTROL = { GPIOA, 0, 15, 0 };
const GPIO_pin_t GPIO_MUX_SEL = { GPIOA, 0, 4, 0 };
const GPIO_pin_t GPIO_MUX_EN = { GPIOA, 0, 5, 0 };
#endif
#ifdef BPSM
// Charge control and status.
const GPIO_pin_t GPIO_CHRG_ST = { GPIOA, 0, 8, 0 };
const GPIO_pin_t GPIO_CHRG_EN = { GPIOA, 0, 9, 0 };
#endif
#ifdef UHFM
// RF power enable.
const GPIO_pin_t GPIO_RF_POWER_ENABLE = { GPIOB, 1, 8, 0 };
const GPIO_pin_t GPIO_RF_TX_ENABLE = { GPIOB, 1, 7, 0 };
const GPIO_pin_t GPIO_RF_RX_ENABLE = { GPIOB, 1, 6, 0 };
// TCXO power control.
const GPIO_pin_t GPIO_TCXO_POWER_ENABLE = { GPIOA, 0, 8, 0 };
// Radio SPI.
const GPIO_pin_t GPIO_S2LP_CS = { GPIOA, 0, 15, 0 };
const SPI_gpio_t SPI_GPIO_S2LP = { &GPIO_SPI1_SCK, &GPIO_SPI1_MOSI, &GPIO_SPI1_MISO };
// S2LP GPIOs.
const GPIO_pin_t GPIO_S2LP_SDN = { GPIOA, 0, 9, 0 };
const GPIO_pin_t GPIO_S2LP_GPIO0 = { GPIOA, 0, 11, 0 };
// Test points.
const GPIO_pin_t GPIO_TP1 = { GPIOA, 0, 0, 0 };
const GPIO_pin_t GPIO_TP2 = { GPIOA, 0, 5, 0 };
const GPIO_pin_t GPIO_TP3 = { GPIOA, 0, 12, 0 };
#endif
#ifdef GPSM
// GPS.
const GPIO_pin_t GPIO_GPS_POWER_ENABLE = { GPIOB, 1, 7, 0 };
const GPIO_pin_t GPIO_GPS_RESET = { GPIOB, 1, 3, 0 };
const GPIO_pin_t GPIO_GPS_VBCKP = { GPIOA, 0, 8, 0 };
const GPIO_pin_t GPIO_GPS_TIMEPULSE = { GPIOA, 0, 15, 0 };
const GPIO_pin_t GPIO_ANT_POWER_ENABLE = { GPIOB, 1, 6, 0 };
// USART.
const USART_gpio_t USART_GPIO_GPS = { &GPIO_USART2_TX, &GPIO_USART2_RX };
// Test point.
const GPIO_pin_t GPIO_TP1 = { GPIOA, 0, 5, 0 };
#endif
#ifdef SM
// Digital front-end.
const GPIO_pin_t GPIO_DIGITAL_POWER_ENABLE = { GPIOB, 1, 3, 0 };
const GPIO_pin_t GPIO_DIO0 = { GPIOA, 0, 0, 0 };
const GPIO_pin_t GPIO_DIO1 = { GPIOA, 0, 1, 0 };
const GPIO_pin_t GPIO_DIO2 = { GPIOA, 0, 9, 0 };
const GPIO_pin_t GPIO_DIO3 = { GPIOA, 0, 10, 0 };
// Digital sensors.
const GPIO_pin_t GPIO_SENSORS_POWER_ENABLE = { GPIOA, 0, 15, 0 };
const I2C_gpio_t I2C_GPIO_SENSORS = { &GPIO_I2C1_SCL, &GPIO_I2C1_SDA };
#endif
#ifdef MPMCM
// TCXO.
const GPIO_pin_t GPIO_TCXO_POWER_ENABLE = { GPIOC, 2, 13, 0 };
// Current sensors detectors.
const GPIO_pin_t GPIO_ACI1_DETECT = { GPIOB, 1, 0, 0 };
const GPIO_pin_t GPIO_ACI2_DETECT = { GPIOB, 1, 1, 0 };
const GPIO_pin_t GPIO_ACI3_DETECT = { GPIOB, 1, 2, 0 };
const GPIO_pin_t GPIO_ACI4_DETECT = { GPIOB, 1, 14, 0 };
// Zero cross detector.
const GPIO_pin_t GPIO_ZERO_CROSS_RAW = { GPIOA, 0, 3, 0 };
const GPIO_pin_t GPIO_ZERO_CROSS_PULSE = { GPIOA, 0, 2, 0 };
// Frequency measure.
const TIM_gpio_t TIM_GPIO_ACV_FREQUENCY = { (const TIM_channel_gpio_t**) &TIM_CHANNEL_GPIO_LIST_ACV_FREQUENCY, TIM_CHANNEL_INDEX_ACV_FREQUENCY_LAST };
// TIC interface.
const GPIO_pin_t GPIO_TIC_POWER_ENABLE = { GPIOB, 1, 4, 0 };
const USART_gpio_t USART_GPIO_TIC = { &GPIO_USART2_TX, &GPIO_USART2_RX };
// Test points.
const GPIO_pin_t GPIO_TP1 = { GPIOB, 1, 5, 0 };
const GPIO_pin_t GPIO_MCO = { GPIOA, 0, 8, 0 };
#endif
#ifdef BCM
// Charge control and status.
const GPIO_pin_t GPIO_CHRG_ST0 = { GPIOB, 1, 5, 0 };
const GPIO_pin_t GPIO_CHRG_ST1 = { GPIOB, 1, 4, 0 };
const GPIO_pin_t GPIO_CHRG_EN = { GPIOB, 1, 6, 0 };
#endif

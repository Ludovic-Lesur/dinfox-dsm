/*
 * gpio_mapping.c
 *
 *  Created on: 18 apr. 2024
 *      Author: Ludo
 */

#include "gpio_mapping.h"

#include "adc.h"
#include "gpio.h"
#include "gpio_registers.h"
#include "i2c.h"
#include "lpuart.h"
#include "mode.h"
#include "spi.h"
#include "tim.h"
#include "usart.h"

/*** GPIO MAPPING local macros ***/

#if ((defined DDRM) || (defined LVRM) || (defined RRM) || (defined GPSM))
#if (defined LVRM) && (defined HW1_0)
#define GPIO_TIM_CHANNEL_LED_RED    TIM_CHANNEL_3
#define GPIO_TIM_CHANNEL_LED_GREEN  TIM_CHANNEL_2
#define GPIO_TIM_CHANNEL_LED_BLUE   TIM_CHANNEL_1
#endif
#if (defined LVRM) && (defined HW2_0)
#define GPIO_TIM_CHANNEL_LED_RED    TIM_CHANNEL_2
#define GPIO_TIM_CHANNEL_LED_GREEN  TIM_CHANNEL_3
#define GPIO_TIM_CHANNEL_LED_BLUE   TIM_CHANNEL_1
#endif
#ifdef DDRM
#define GPIO_TIM_CHANNEL_LED_RED    TIM_CHANNEL_3
#define GPIO_TIM_CHANNEL_LED_GREEN  TIM_CHANNEL_1
#define GPIO_TIM_CHANNEL_LED_BLUE   TIM_CHANNEL_2
#endif
#ifdef RRM
#define GPIO_TIM_CHANNEL_LED_RED    TIM_CHANNEL_2
#define GPIO_TIM_CHANNEL_LED_GREEN  TIM_CHANNEL_3
#define GPIO_TIM_CHANNEL_LED_BLUE   TIM_CHANNEL_1
#endif
#ifdef GPSM
#define GPIO_TIM_CHANNEL_LED_RED    TIM_CHANNEL_1
#define GPIO_TIM_CHANNEL_LED_GREEN  TIM_CHANNEL_2
#define GPIO_TIM_CHANNEL_LED_BLUE   TIM_CHANNEL_3
#endif
#endif

/*** GPIO MAPPING local structures ***/

/*******************************************************************/
typedef enum {
#ifdef BPSM
    GPIO_ADC_CHANNEL_INDEX_VSRC_MEASURE = 0,
    GPIO_ADC_CHANNEL_INDEX_VSTR_MEASURE,
    GPIO_ADC_CHANNEL_INDEX_VBKP_MEASURE,
#endif
#if ((defined DDRM) || (defined LVRM) || (defined RRM))
    GPIO_ADC_CHANNEL_INDEX_VIN_MEASURE = 0,
    GPIO_ADC_CHANNEL_INDEX_VOUT_MEASURE,
    GPIO_ADC_CHANNEL_INDEX_IOUT_MEASURE,
#endif
#ifdef GPSM
    GPIO_ADC_CHANNEL_INDEX_VGPS_MEASURE = 0,
    GPIO_ADC_CHANNEL_INDEX_VANT_MEASURE,
#endif
#ifdef SM
    GPIO_ADC_CHANNEL_INDEX_AIN0 = 0,
    GPIO_ADC_CHANNEL_INDEX_AIN1,
    GPIO_ADC_CHANNEL_INDEX_AIN2,
    GPIO_ADC_CHANNEL_INDEX_AIN3,
#endif
#ifdef UHFM
    GPIO_ADC_CHANNEL_INDEX_VRF = 0,
#endif
    GPIO_ADC_CHANNEL_INDEX_LAST
} GPIO_adc_channel_index_t;

/*** GPIO MAPPING local global variables ***/

// LPUART1.
#if ((defined LVRM) && (defined HW1_0)) || (defined BPSM) || (defined DDRM) || (defined RRM)
static const GPIO_pin_t GPIO_LPUART1_TX = (GPIO_pin_t) { GPIOB, 1, 6, 6 };
static const GPIO_pin_t GPIO_LPUART1_RX = (GPIO_pin_t) { GPIOB, 1, 7, 6 };
#endif
#if ((defined LVRM) && (defined HW2_0)) || (defined SM) || (defined UHFM) || (defined GPSM)
static const GPIO_pin_t GPIO_LPUART1_TX = (GPIO_pin_t) { GPIOA, 0, 2, 6 };
static const GPIO_pin_t GPIO_LPUART1_RX = (GPIO_pin_t) { GPIOA, 0, 3, 6 };
#endif
static const GPIO_pin_t GPIO_LPUART1_DE = (GPIO_pin_t) { GPIOB, 1, 1, 4 };
#if ((defined LVRM) && (defined HW1_0)) || (defined DDRM) || (defined RRM)
static const GPIO_pin_t GPIO_LPUART1_NRE = (GPIO_pin_t) { GPIOA, 0, 9, 0 };
#endif
#ifdef BPSM
static const GPIO_pin_t GPIO_LPUART1_NRE = (GPIO_pin_t) { GPIOA, 0, 10, 0 };
#endif
#if (defined SM) || (defined GPSM)
static const GPIO_pin_t GPIO_LPUART1_NRE = (GPIO_pin_t) { GPIOA, 0, 4, 0 };
#endif
#ifdef UHFM
static const GPIO_pin_t GPIO_LPUART1_NRE = (GPIO_pin_t) { GPIOB, 1, 2, 0 };
#endif
#if (defined LVRM) && (defined HW2_0)
static const GPIO_pin_t GPIO_LPUART1_NRE = (GPIO_pin_t) { GPIOA, 0, 7, 0 };
#endif
// ADC.
#ifdef BPSM
static const GPIO_pin_t GPIO_ADC_VSRC_MEASURE = (GPIO_pin_t) { GPIOA, 0, 6, 0 };
static const GPIO_pin_t GPIO_ADC_VSTR_MEASURE = (GPIO_pin_t) { GPIOA, 0, 4, 0 };
static const GPIO_pin_t GPIO_ADC_VBKP_MEASURE = (GPIO_pin_t) { GPIOA, 0, 0, 0 };
static const GPIO_pin_t* GPIO_ADC_PINS_LIST[GPIO_ADC_CHANNEL_INDEX_LAST] = { &GPIO_ADC_VSRC_MEASURE, &GPIO_ADC_VSTR_MEASURE, &GPIO_ADC_VBKP_MEASURE };
#endif
#if ((defined DDRM) || (defined LVRM) || (defined RRM))
#ifdef DDRM
static const GPIO_pin_t GPIO_ADC_VIN_MEASURE = (GPIO_pin_t) { GPIOA, 0, 7, 0 };
static const GPIO_pin_t GPIO_ADC_VOUT_MEASURE = (GPIO_pin_t) { GPIOA, 0, 4, 0 };
static const GPIO_pin_t GPIO_ADC_IOUT_MEASURE = (GPIO_pin_t) { GPIOA, 0, 0, 0 };
#endif
#if ((defined LVRM) && (defined HW1_0))
static const GPIO_pin_t GPIO_ADC_VIN_MEASURE = (GPIO_pin_t) { GPIOA, 0, 6, 0 };
static const GPIO_pin_t GPIO_ADC_VOUT_MEASURE = (GPIO_pin_t) { GPIOA, 0, 4, 0 };
static const GPIO_pin_t GPIO_ADC_IOUT_MEASURE = (GPIO_pin_t) { GPIOA, 0, 0, 0 };
#endif
#if ((defined LVRM) && (defined HW2_0))
static const GPIO_pin_t GPIO_ADC_VIN_MEASURE = (GPIO_pin_t) { GPIOA, 0, 0, 0 };
static const GPIO_pin_t GPIO_ADC_VOUT_MEASURE = (GPIO_pin_t) { GPIOA, 0, 6, 0 };
static const GPIO_pin_t GPIO_ADC_IOUT_MEASURE = (GPIO_pin_t) { GPIOA, 0, 1, 0 };
#endif
#ifdef RRM
static const GPIO_pin_t GPIO_ADC_VIN_MEASURE = (GPIO_pin_t) { GPIOA, 0, 7, 0 };
static const GPIO_pin_t GPIO_ADC_VOUT_MEASURE = (GPIO_pin_t) { GPIOA, 0, 6, 0 };
static const GPIO_pin_t GPIO_ADC_IOUT_MEASURE = (GPIO_pin_t) { GPIOA, 0, 4, 0 };
#endif
static const GPIO_pin_t* GPIO_ADC_PINS_LIST[GPIO_ADC_CHANNEL_INDEX_LAST] = { &GPIO_ADC_VIN_MEASURE, &GPIO_ADC_VOUT_MEASURE, &GPIO_ADC_IOUT_MEASURE };
#endif
#ifdef GPSM
static const GPIO_pin_t GPIO_ADC_VGPS_MEASURE = (GPIO_pin_t) { GPIOA, 0, 0, 0 };
static const GPIO_pin_t GPIO_ADC_VANT_MEASURE = (GPIO_pin_t) { GPIOA, 0, 1, 0 };
static const GPIO_pin_t* GPIO_ADC_PINS_LIST[GPIO_ADC_CHANNEL_INDEX_LAST] = { &GPIO_ADC_VGPS_MEASURE, &GPIO_ADC_VANT_MEASURE };
#endif
#ifdef SM
static const GPIO_pin_t GPIO_ADC_AIN0 = (GPIO_pin_t) { GPIOA, 0, 5, 0 };
static const GPIO_pin_t GPIO_ADC_AIN1 = (GPIO_pin_t) { GPIOA, 0, 6, 0 };
static const GPIO_pin_t GPIO_ADC_AIN2 = (GPIO_pin_t) { GPIOA, 0, 7, 0 };
static const GPIO_pin_t GPIO_ADC_AIN3 = (GPIO_pin_t) { GPIOB, 1, 0, 0 };
static const GPIO_pin_t* GPIO_ADC_PINS_LIST[GPIO_ADC_CHANNEL_INDEX_LAST] = { &GPIO_ADC_AIN0, &GPIO_ADC_AIN1, &GPIO_ADC_AIN2, &GPIO_ADC_AIN3 };
#endif
#ifdef UHFM
static const GPIO_pin_t GPIO_ADC_VRF_MEASURE = (GPIO_pin_t) { GPIOA, 0, 7, 0 };
static const GPIO_pin_t* GPIO_ADC_PINS_LIST[GPIO_ADC_CHANNEL_INDEX_LAST] = { &GPIO_ADC_VRF_MEASURE };
#endif
#ifdef XM_RGB_LED
// RGB LED.
#ifdef DDRM
static const GPIO_pin_t GPIO_LED_RED = (GPIO_pin_t) { GPIOA, 0, 10, 5 };
static const GPIO_pin_t GPIO_LED_GREEN = (GPIO_pin_t) { GPIOA, 0, 5, 5 };
static const GPIO_pin_t GPIO_LED_BLUE = (GPIO_pin_t) { GPIOA, 0, 1, 2 };
#endif
#if (defined LVRM) && (defined HW1_0)
static const GPIO_pin_t GPIO_LED_RED = (GPIO_pin_t) { GPIOA, 0, 10, 5 };
static const GPIO_pin_t GPIO_LED_GREEN = (GPIO_pin_t) { GPIOA, 0, 1, 2 };
static const GPIO_pin_t GPIO_LED_BLUE = (GPIO_pin_t) { GPIOA, 0, 5, 5 };
#endif
#if (defined LVRM) && (defined HW2_0)
static const GPIO_pin_t GPIO_LED_RED = (GPIO_pin_t) { GPIOB, 1, 3, 2 };
static const GPIO_pin_t GPIO_LED_GREEN = (GPIO_pin_t) { GPIOB, 1, 0, 5 };
static const GPIO_pin_t GPIO_LED_BLUE = (GPIO_pin_t) { GPIOA, 0, 8, 5 };
#endif
#ifdef RRM
static const GPIO_pin_t GPIO_LED_RED = (GPIO_pin_t) { GPIOA, 0, 1, 2 };
static const GPIO_pin_t GPIO_LED_GREEN = (GPIO_pin_t) { GPIOA, 0, 10, 5 };
static const GPIO_pin_t GPIO_LED_BLUE = (GPIO_pin_t) { GPIOA, 0, 5, 5 };
#endif
#ifdef GPSM
static const GPIO_pin_t GPIO_LED_RED = (GPIO_pin_t) { GPIOA, 0, 6, 5 };
static const GPIO_pin_t GPIO_LED_GREEN = (GPIO_pin_t) { GPIOA, 0, 7, 5 };
static const GPIO_pin_t GPIO_LED_BLUE = (GPIO_pin_t) { GPIOB, 1, 0, 5 };
#endif
// Timer channels.
static const TIM_channel_gpio_t GPIO_TIM_CHANNEL_RED = { GPIO_TIM_CHANNEL_LED_RED, &GPIO_LED_RED, TIM_POLARITY_ACTIVE_LOW };
static const TIM_channel_gpio_t GPIO_TIM_CHANNEL_GREEN = { GPIO_TIM_CHANNEL_LED_GREEN, &GPIO_LED_GREEN, TIM_POLARITY_ACTIVE_LOW };
static const TIM_channel_gpio_t GPIO_TIM_CHANNEL_BLUE = { GPIO_TIM_CHANNEL_LED_BLUE, &GPIO_LED_BLUE, TIM_POLARITY_ACTIVE_LOW };
// Timer pins list.
#ifdef GPSM
static const TIM_channel_gpio_t* GPIO_TIM_RG_PINS_LIST[GPIO_TIM_RG_CHANNEL_INDEX_LAST] = { &GPIO_TIM_CHANNEL_RED, &GPIO_TIM_CHANNEL_GREEN };
static const TIM_channel_gpio_t* GPIO_TIM_B_PINS_LIST[GPIO_TIM_B_CHANNEL_INDEX_LAST] = { &GPIO_TIM_CHANNEL_BLUE };
#else
static const TIM_channel_gpio_t* GPIO_TIM_PINS_LIST[GPIO_TIM_CHANNEL_INDEX_LAST] = { &GPIO_TIM_CHANNEL_RED, &GPIO_TIM_CHANNEL_GREEN, &GPIO_TIM_CHANNEL_BLUE };
#endif
#endif
#ifdef SM
// I2C1.
const GPIO_pin_t GPIO_I2C1_SCL = (GPIO_pin_t) { GPIOB, 1, 6, 1 };
const GPIO_pin_t GPIO_I2C1_SDA = (GPIO_pin_t) { GPIOB, 1, 7, 1 };
#endif
#ifdef GPSM
// USART2.
static const GPIO_pin_t GPIO_USART2_TX = (GPIO_pin_t) { GPIOA, 0, 9, 4 };
static const GPIO_pin_t GPIO_USART2_RX = (GPIO_pin_t) { GPIOA, 0, 10, 4 };
#endif
#ifdef UHFM
// SPI1.
static const GPIO_pin_t GPIO_SPI1_SCK = (GPIO_pin_t) { GPIOB, 1, 3, 0 };
static const GPIO_pin_t GPIO_SPI1_MISO = (GPIO_pin_t) { GPIOB, 1, 4, 0 };
static const GPIO_pin_t GPIO_SPI1_MOSI = (GPIO_pin_t) { GPIOB, 1, 5, 0 };
#endif

/*** GPIO MAPPING global variables ***/

// RS485.
const LPUART_gpio_t GPIO_RS485_LPUART = { &GPIO_LPUART1_TX, &GPIO_LPUART1_RX, &GPIO_LPUART1_DE, &GPIO_LPUART1_NRE };
// Analog inputs.
#ifdef BPSM
const GPIO_pin_t GPIO_MNTR_EN = (GPIO_pin_t) { GPIOA, 0, 1, 0 };
#endif
#if ((defined LVRM) && (defined HW2_0))
const GPIO_pin_t GPIO_MNTR_EN = (GPIO_pin_t) { GPIOB, 1, 7, 0 };
#endif
#ifdef SM
const GPIO_pin_t GPIO_ANALOG_POWER_ENABLE = (GPIO_pin_t) { GPIOA, 0, 8, 0 };
#endif
const ADC_gpio_t GPIO_ADC = { (const GPIO_pin_t**) &GPIO_ADC_PINS_LIST, GPIO_ADC_CHANNEL_INDEX_LAST };
#ifdef XM_RGB_LED
// RGB LED.
#ifdef GPSM
const TIM_gpio_t GPIO_LED_TIM_RG = { (const TIM_channel_gpio_t**) &GPIO_TIM_RG_PINS_LIST, GPIO_TIM_RG_CHANNEL_INDEX_LAST };
const TIM_gpio_t GPIO_LED_TIM_B = { (const TIM_channel_gpio_t**) &GPIO_TIM_B_PINS_LIST, GPIO_TIM_B_CHANNEL_INDEX_LAST };
#else
const TIM_gpio_t GPIO_LED_TIM = { (const TIM_channel_gpio_t**) &GPIO_TIM_PINS_LIST, GPIO_TIM_CHANNEL_INDEX_LAST };
#endif
#endif
#ifdef BPSM
const GPIO_pin_t GPIO_OUT_EN = (GPIO_pin_t) { GPIOA, 0, 5, 0 };
#endif
#ifdef DDRM
const GPIO_pin_t GPIO_OUT_EN = (GPIO_pin_t) { GPIOA, 0, 6, 0 };
#endif
#if (defined LVRM) && (defined HW1_0)
const GPIO_pin_t GPIO_OUT_EN = (GPIO_pin_t) { GPIOA, 0, 7, 0 };
#endif
#ifdef RRM
const GPIO_pin_t GPIO_OUT_EN = (GPIO_pin_t) { GPIOA, 0, 0, 0 };
#endif
#ifdef BPSM
// Charge control and status.
const GPIO_pin_t GPIO_CHRG_ST = (GPIO_pin_t) { GPIOA, 0, 8, 0 };
const GPIO_pin_t GPIO_CHRG_EN = (GPIO_pin_t) { GPIOA, 0, 9, 0 };
#endif
#ifdef SM
// Digital front-end.
const GPIO_pin_t GPIO_DIGITAL_POWER_ENABLE = (GPIO_pin_t) { GPIOB, 1, 3, 0 };
const GPIO_pin_t GPIO_DIO0 = (GPIO_pin_t) { GPIOA, 0, 0, 0 };
const GPIO_pin_t GPIO_DIO1 = (GPIO_pin_t) { GPIOA, 0, 1, 0 };
const GPIO_pin_t GPIO_DIO2 = (GPIO_pin_t) { GPIOA, 0, 9, 0 };
const GPIO_pin_t GPIO_DIO3 = (GPIO_pin_t) { GPIOA, 0, 10, 0 };
// Digital sensors.
const GPIO_pin_t GPIO_SENSORS_POWER_ENABLE = (GPIO_pin_t) { GPIOA, 0, 15, 0 };
const I2C_gpio_t GPIO_SENSORS_I2C = { &GPIO_I2C1_SCL, &GPIO_I2C1_SDA };
#endif
#if (defined LVRM) && (defined HW2_0)
// Bistable relay control.
const GPIO_pin_t GPIO_DC_DC_POWER_ENABLE = (GPIO_pin_t) { GPIOB, 1, 6, 0 };
const GPIO_pin_t GPIO_COIL_POWER_ENABLE = (GPIO_pin_t) { GPIOA, 0, 9, 0 };
const GPIO_pin_t GPIO_OUT_SELECT = (GPIO_pin_t) { GPIOA, 0, 10, 0 };
const GPIO_pin_t GPIO_OUT_CONTROL = (GPIO_pin_t) { GPIOA, 0, 15, 0 };
const GPIO_pin_t GPIO_MUX_SEL = (GPIO_pin_t) { GPIOA, 0, 4, 0 };
const GPIO_pin_t GPIO_MUX_EN = (GPIO_pin_t) { GPIOA, 0, 5, 0 };
#endif
#ifdef GPSM
// GPS.
const GPIO_pin_t GPIO_GPS_POWER_ENABLE = (GPIO_pin_t) { GPIOB, 1, 7, 0 };
const GPIO_pin_t GPIO_GPS_RESET = (GPIO_pin_t) { GPIOB, 1, 3, 0 };
const GPIO_pin_t GPIO_GPS_VBCKP = (GPIO_pin_t) { GPIOA, 0, 8, 0 };
const GPIO_pin_t GPIO_GPS_TIMEPULSE = (GPIO_pin_t) { GPIOA, 0, 15, 0 };
const GPIO_pin_t GPIO_ANT_POWER_ENABLE = (GPIO_pin_t) { GPIOB, 1, 6, 0 };
// USART.
const USART_gpio_t GPIO_GPS_USART = { &GPIO_USART2_TX, &GPIO_USART2_RX };
// Test point.
const GPIO_pin_t GPIO_TP1 = (GPIO_pin_t) { GPIOA, 0, 5, 0 };
#endif
#ifdef UHFM
// RF power enable.
const GPIO_pin_t GPIO_RF_POWER_ENABLE = (GPIO_pin_t) { GPIOB, 1, 8, 0 };
const GPIO_pin_t GPIO_RF_TX_ENABLE = (GPIO_pin_t) { GPIOB, 1, 7, 0 };
const GPIO_pin_t GPIO_RF_RX_ENABLE = (GPIO_pin_t) { GPIOB, 1, 6, 0 };
// TCXO power control.
const GPIO_pin_t GPIO_TCXO_POWER_ENABLE = (GPIO_pin_t) { GPIOA, 0, 8, 0 };
// Radio SPI.
const GPIO_pin_t GPIO_S2LP_CS = (GPIO_pin_t) { GPIOA, 0, 15, 0 };
const SPI_gpio_t GPIO_S2LP_SPI = { &GPIO_SPI1_SCK, &GPIO_SPI1_MOSI, &GPIO_SPI1_MISO };
// S2LP GPIOs.
const GPIO_pin_t GPIO_S2LP_SDN = (GPIO_pin_t) { GPIOA, 0, 9, 0 };
const GPIO_pin_t GPIO_S2LP_GPIO0 = (GPIO_pin_t) { GPIOA, 0, 11, 0 };
// Test points.
const GPIO_pin_t GPIO_TP1 = (GPIO_pin_t) { GPIOA, 0, 0, 0 };
const GPIO_pin_t GPIO_TP2 = (GPIO_pin_t) { GPIOA, 0, 5, 0 };
const GPIO_pin_t GPIO_TP3 = (GPIO_pin_t) { GPIOA, 0, 12, 0 };
#endif

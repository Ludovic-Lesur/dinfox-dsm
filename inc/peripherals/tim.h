/*
 * tim.h
 *
 *  Created on: 22 aug. 2020
 *      Author: Ludo
 */

#ifndef TIM_H
#define TIM_H

/*** TIM structures ***/

// Color bit masks defined as 0b<CH4><CH3><CH2><CH1>
typedef enum {
	TIM2_CHANNEL_MASK_OFF = 0b0000,
	TIM2_CHANNEL_MASK_RED = 0b0100,
	TIM2_CHANNEL_MASK_GREEN = 0b0010,
	TIM2_CHANNEL_MASK_YELLOW = 0b0110,
	TIM2_CHANNEL_MASK_BLUE = 0b0001,
	TIM2_CHANNEL_MASK_MAGENTA = 0b0101,
	TIM2_CHANNEL_MASK_CYAN = 0b0011,
	TIM2_CHANNEL_MASK_WHITE	= 0b0111
} TIM2_channel_mask_t;

/*** TIM functions ***/

void TIM2_init(void);
void TIM2_disable(void);
void TIM2_set_color_mask(TIM2_channel_mask_t led_color);
void TIM2_start(void);
void TIM2_stop(void);

void TIM21_init(unsigned int led_blink_period_ms);
void TIM21_disable(void);
void TIM21_Start(void);
void TIM21_Stop(void);
unsigned char TIM21_IsSingleBlinkDone(void);

#endif /* TIM_H */

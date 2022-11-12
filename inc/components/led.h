/*
 * led.h
 *
 *  Created on: 22 aug 2020
 *      Author: Ludo
 */

#ifndef LED_H
#define LED_H

#include "mode.h"
#include "tim.h"
#include "types.h"

/*** LED functions ***/

void LED_init(void);
#ifdef RSM
void LED_start_blink(uint32_t blink_duration_ms, TIM2_channel_mask_t color);
void LED_stop_blink(void);
#endif

#endif /* LED_H */

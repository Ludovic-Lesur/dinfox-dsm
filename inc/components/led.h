/*
 * led.h
 *
 *  Created on: 22 aug 2020
 *      Author: Ludo
 */

#ifndef LED_H
#define LED_H

#include "tim.h"
#include "types.h"

/*** LED functions ***/

void LED_init(void);
void LED_single_blink(uint32_t blink_duration_ms, TIM2_channel_mask_t color);

#endif /* LED_H */

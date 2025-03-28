/*
 * stm32l0xx_registers_flags.h
 *
 *  Created on: 15 aug. 2024
 *      Author: Ludo
 */

#ifndef __STM32L0XX_REGISTERS_FLAGS_H__
#define __STM32L0XX_REGISTERS_FLAGS_H__

/*** STM32L0xx registers compilation flags ***/

#if ((defined BCM) || (defined BPSM) || (defined DDRM) || ((defined LVRM) && (defined HW1_0)) || (defined RRM))
#define STM32L0XX_REGISTERS_MCU_CATEGORY    1
#endif
#if (((defined LVRM) && (defined HW2_0)) || (defined GPSM) || (defined SM) || (defined UHFM))
#define STM32L0XX_REGISTERS_MCU_CATEGORY    2
#endif

#endif /* __STM32L0XX_REGISTERS_FLAGS_H__ */

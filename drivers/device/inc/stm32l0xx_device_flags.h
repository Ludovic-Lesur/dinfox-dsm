/*
 * stm32l0xx_device_flags.h
 *
 *  Created on: 19 jan. 2025
 *      Author: Ludo
 */

#ifndef __STM32L0XX_DEVICE_FLAGS_H__
#define __STM32L0XX_DEVICE_FLAGS_H__

#ifndef STM32L0XX_REGISTERS_DISABLE_FLAGS_FILE
#include "stm32l0xx_registers_flags.h"
#endif

/*** STM32L0XX device compilation flags ***/

#ifdef MPMCM
#define STM32L0XX_DEVICE_DISABLE
#endif

#if (STM32L0XX_REGISTERS_MCU_CATEGORY == 1)
#define STM32L0XX_DEVICE_STACK_SIZE     0x00000100
#define STM32L0XX_DEVICE_HEAP_SIZE      0x00000400
#else
#define STM32L0XX_DEVICE_STACK_SIZE     0x00000400
#define STM32L0XX_DEVICE_HEAP_SIZE      0x00000C00
#endif

#endif /* __STM32L0XX_DEVICE_FLAGS_H__ */

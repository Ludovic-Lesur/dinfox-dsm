/*
 * dsm_flags_slave.h
 *
 *  Created on: 12 mar. 2026
 *      Author: Ludo
 */

#ifndef __DSM_FLAGS_SLAVE_H__
#define __DSM_FLAGS_SLAVE_H__

#include "dsm_flags.h"

/*** Slave compilation flags ***/

#if ((defined BCM) || (defined BPSM) || (defined LVRM) || (defined DDRM) || (defined RRM))
#define DSM_LOAD_CONTROL
#endif
#if (((defined BCM) && !(defined BCM_CHARGE_LED_FORCED_HARDWARE)) || ((defined LVRM) && !(defined LVRM_MODE_BMS)) || (defined DDRM) || (defined RRM))
#define DSM_OUTPUT_CURRENT_INDICATOR
#endif
#if ((defined DSM_OUTPUT_CURRENT_INDICATOR) || (defined GPSM) || (defined MPMCM))
#define DSM_RGB_LED
#endif

#endif /* __DSM_FLAGS_SLAVE_H__ */

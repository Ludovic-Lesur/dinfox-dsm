/*
 * mode.h
 *
 *  Created on: 18 apr. 2020
 *      Author: Ludo
 */

#ifndef __MODE_H__
#define __MODE_H__

#define AM		// Addressed mode.
//#define DM	// Direct mode.

#ifdef BPSM
#define BPSM_VSTR_VOLTAGE_DIVIDER_RATIO		2
#endif

/*** Debug mode ***/

//#define DEBUG		// Use programming pins for debug purpose if defined.

/*** Error management ***/

#if (defined AM && defined DM)
#error "Only 1 mode must be selected."
#endif

#endif /* __MODE_H__ */

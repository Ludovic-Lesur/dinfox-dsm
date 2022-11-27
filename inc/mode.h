/*
 * mode.h
 *
 *  Created on: 18 apr. 2020
 *      Author: Ludo
 */

#ifndef __MODE_H__
#define __MODE_H__

// Board selection.
#define LVRM
//#define BPSM
//#define DDRM
//#define RRM

#define AM		// Addressed mode.
//#define DM	// Direct mode.


/*** Debug mode ***/

//#define DEBUG		// Use programming pins for debug purpose if defined.

/*** Error management ***/

#if (defined AM && defined DM)
#error "Only 1 mode must be selected."
#endif

#endif /* __MODE_H__ */

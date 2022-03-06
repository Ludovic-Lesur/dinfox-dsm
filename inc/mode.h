/*
 * mode.h
 *
 *  Created on: 18 apr. 2020
 *      Author: Ludo
 */

#ifndef MODE_H
#define MODE_H

#define RSM		// RS485 mode with address check.
//#define ATM	// AT command mode without address check.


/*** Debug mode ***/

//#define DEBUG		// Use programming pins for debug purpose if defined.

/*** Error management ***/

#if (defined RSM && defined ATM)
#error "Only 1 mode must be selected."
#endif

#endif /* MODE_H */

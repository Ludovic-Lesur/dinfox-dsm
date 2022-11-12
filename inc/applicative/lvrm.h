/*
 * lvrm.h
 *
 *  Created on: Mar 12, 2022
 *      Author: Ludo
 */

#ifndef LVRM_H
#define LVRM_H

/*** LVRM registers mapping ***/

typedef enum {
	LVRM_REGISTER_RS485_ADDRESS = 0,
	LVRM_REGISTER_BOARD_ID,
	LVRM_REGISTER_VCOM_MV,
	LVRM_REGISTER_VOUT_MV,
	LVRM_REGISTER_IOUT_UA,
	LVRM_REGISTER_VMCU_MV,
	LVRM_REGISTER_OUT_EN,
	LVRM_REGISTER_LAST,
} LVRM_register_address_t;

#endif /* LVRM_H */

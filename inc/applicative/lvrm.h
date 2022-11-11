/*
 * lvrm.h
 *
 *  Created on: Mar 12, 2022
 *      Author: Ludo
 */

#ifndef LVRM_H
#define LVRM_H

#define LVRM_BOARD_ID	0x00

typedef enum {
	LVRM_REGISTER_RS485_ADDRESS,
	LVRM_REGISTER_BOARD_ID,
	LVRM_REGISTER_VIN_MV,
	LVRM_REGISTER_VOUT_MV,
	LVRM_REGISTER_IOUT_UA,
	LVRM_REGISTER_VMCU,
	LVRM_REGISTER_OUT_EN,
	LVRM_REGISTER_LAST,
} LVRM_register_address_t;

#endif /* LVRM_H */

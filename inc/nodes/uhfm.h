/*
 * uhfm.h
 *
 *  Created on: 4 jun. 2023
 *      Author: Ludo
 */

#ifndef __UHFM_H__
#define __UHFM_H__

#include "common.h"
#include "common_reg.h"
#include "dinfox.h"
#include "node.h"
#include "uhfm_reg.h"

#ifdef UHFM

/*** UHFM macros ***/

#define NODE_BOARD_ID		DINFOX_BOARD_ID_UHFM
#define NODE_REG_ADDR_LAST	UHFM_REG_ADDR_LAST

/*** UHFM global variables ***/

static const DINFOX_register_access_t NODE_REG_ACCESS[UHFM_REG_ADDR_LAST] = {
	COMMON_REG_ACCESS
	DINFOX_REG_ACCESS_READ_WRITE,
	DINFOX_REG_ACCESS_READ_ONLY,
	DINFOX_REG_ACCESS_READ_ONLY,
	DINFOX_REG_ACCESS_READ_ONLY,
	DINFOX_REG_ACCESS_READ_ONLY,
	DINFOX_REG_ACCESS_READ_ONLY,
	DINFOX_REG_ACCESS_READ_ONLY,
	DINFOX_REG_ACCESS_READ_WRITE,
	DINFOX_REG_ACCESS_READ_WRITE,
	DINFOX_REG_ACCESS_READ_WRITE,
	DINFOX_REG_ACCESS_READ_WRITE,
	DINFOX_REG_ACCESS_READ_WRITE,
	DINFOX_REG_ACCESS_READ_WRITE,
	DINFOX_REG_ACCESS_READ_ONLY,
	DINFOX_REG_ACCESS_READ_ONLY,
	DINFOX_REG_ACCESS_READ_ONLY,
	DINFOX_REG_ACCESS_READ_ONLY,
	DINFOX_REG_ACCESS_READ_ONLY,
	DINFOX_REG_ACCESS_READ_ONLY,
	DINFOX_REG_ACCESS_READ_WRITE,
	DINFOX_REG_ACCESS_READ_WRITE
};

/*** UHFM functions ***/

NODE_status_t UHFM_init_registers(void);

NODE_status_t UHFM_update_register(uint8_t reg_addr);
NODE_status_t UHFM_check_register(uint8_t reg_addr);

NODE_status_t UHFM_mtrg_callback(void);

#endif /* UHFM */

#endif /* __UHFM_H__ */

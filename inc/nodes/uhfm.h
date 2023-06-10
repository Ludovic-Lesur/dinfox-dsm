/*
 * uhfm.h
 *
 *  Created on: 4 jun. 2023
 *      Author: Ludo
 */

#ifndef __UHFM_H__
#define __UHFM_H__

#include "dinfox.h"
#include "dinfox_reg.h"
#include "dinfox_types.h"
#include "node.h"
#include "uhfm_reg.h"

#ifdef UHFM

/*** UHFM macros ***/

#define NODE_BOARD_ID	DINFOX_BOARD_ID_UHFM

/*** UHFM global variables ***/

static const DINFOX_register_access_t NODE_REG_ACCESS[NODE_REG_ADDR_LAST] = {
	DINFOX_REG_ACCESS
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

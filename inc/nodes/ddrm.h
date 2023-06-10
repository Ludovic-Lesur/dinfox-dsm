/*
 * ddrm.h
 *
 *  Created on: 4 jun. 2023
 *      Author: Ludo
 */

#ifndef __DDRM_H__
#define __DDRM_H__

#include "ddrm_reg.h"
#include "dinfox.h"
#include "dinfox_reg.h"
#include "dinfox_types.h"
#include "node.h"

#ifdef DDRM

/*** DDRM macros ***/

#define NODE_BOARD_ID	DINFOX_BOARD_ID_DDRM

/*** DDRM global variables ***/

static const DINFOX_register_access_t NODE_REG_ACCESS[NODE_REG_ADDR_LAST] = {
	DINFOX_REG_ACCESS
	DINFOX_REG_ACCESS_READ_WRITE,
	DINFOX_REG_ACCESS_READ_ONLY,
	DINFOX_REG_ACCESS_READ_ONLY
};

/*** DDRM functions ***/

NODE_status_t DDRM_init_registers(void);

NODE_status_t DDRM_update_register(uint8_t reg_addr);
NODE_status_t DDRM_check_register(uint8_t reg_addr);

NODE_status_t DDRM_mtrg_callback(void);

#endif /* DDRM */

#endif /* __DDRM_H__ */

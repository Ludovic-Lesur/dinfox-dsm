/*
 * lvrm.h
 *
 *  Created on: 4 jun. 2023
 *      Author: Ludo
 */

#ifndef __LVRM_H__
#define __LVRM_H__

#include "common.h"
#include "common_reg.h"
#include "dinfox_common.h"
#include "lvrm_reg.h"
#include "node.h"

#ifdef LVRM

/*** LVRM macros ***/

#define NODE_BOARD_ID	DINFOX_BOARD_ID_LVRM

/*** LVRM global variables ***/

static const DINFOX_register_access_t NODE_REG_ACCESS[NODE_REG_ADDR_LAST] = {
	COMMON_REG_ACCESS
	DINFOX_REG_ACCESS_READ_WRITE,
	DINFOX_REG_ACCESS_READ_ONLY,
	DINFOX_REG_ACCESS_READ_ONLY
};

/*** LVRM functions ***/

NODE_status_t LVRM_init_registers(void);

NODE_status_t LVRM_update_register(uint8_t reg_addr);
NODE_status_t LVRM_check_register(uint8_t reg_addr);

NODE_status_t LVRM_mtrg_callback(void);

#endif /* LVRM */

#endif /* __LVRM_H__ */

/*
 * sm.h
 *
 *  Created on: 10 jun. 2023
 *      Author: Ludo
 */

#ifndef __SM_H__
#define __SM_H__

#include "common.h"
#include "common_reg.h"
#include "dinfox_common.h"
#include "sm_reg.h"
#include "node.h"

#ifdef SM

/*** SM macros ***/

#define NODE_BOARD_ID		DINFOX_BOARD_ID_SM
#define NODE_REG_ADDR_LAST	SM_REG_ADDR_LAST

/*** SM global variables ***/

static const DINFOX_register_access_t NODE_REG_ACCESS[SM_REG_ADDR_LAST] = {
	COMMON_REG_ACCESS
	DINFOX_REG_ACCESS_READ_ONLY,
	DINFOX_REG_ACCESS_READ_ONLY,
	DINFOX_REG_ACCESS_READ_ONLY,
	DINFOX_REG_ACCESS_READ_ONLY
};

/*** SM functions ***/

NODE_status_t SM_init_registers(void);

NODE_status_t SM_update_register(uint8_t reg_addr);
NODE_status_t SM_check_register(uint8_t reg_addr);

NODE_status_t SM_mtrg_callback(void);

#endif /* SM */

#endif /* __SM_H__ */

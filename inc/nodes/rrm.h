/*
 * rrm.h
 *
 *  Created on: Jun 10, 2023
 *      Author: ludo
 */

#ifndef __RRM_H__
#define __RRM_H__

#include "common.h"
#include "common_reg.h"
#include "dinfox_common.h"
#include "node.h"
#include "rrm_reg.h"

#ifdef RRM

/*** RRM macros ***/

#define NODE_BOARD_ID	DINFOX_BOARD_ID_RRM

/*** RRM global variables ***/

static const DINFOX_register_access_t NODE_REG_ACCESS[NODE_REG_ADDR_LAST] = {
	COMMON_REG_ACCESS
	DINFOX_REG_ACCESS_READ_WRITE,
	DINFOX_REG_ACCESS_READ_ONLY,
	DINFOX_REG_ACCESS_READ_ONLY
};

/*** RRM functions ***/

NODE_status_t RRM_init_registers(void);

NODE_status_t RRM_update_register(uint8_t reg_addr);
NODE_status_t RRM_check_register(uint8_t reg_addr);

NODE_status_t RRM_mtrg_callback(void);

#endif /* RRM */

#endif /* INC_NODES_RRM_H_ */

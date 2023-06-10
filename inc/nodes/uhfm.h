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
#include "dinfox_common.h"
#include "node.h"
#include "uhfm_reg.h"

#ifdef UHFM

/*** UHFM macros ***/

#define NODE_BOARD_ID	DINFOX_BOARD_ID_UHFM

/*** UHFM structures ***/

typedef union {
	struct {
		unsigned ul_frame_1 : 1;
		unsigned ul_frame_2 : 1;
		unsigned ul_frame_3 : 1;
		unsigned dl_frame : 1;
		unsigned dl_conf_frame : 1;
		unsigned network_error : 1; // For LBT and downlink timeout.
		unsigned execution_error : 1; // For internal execution errors.
	};
	uint8_t all;
} UHFM_message_status_t;

/*** UHFM global variables ***/

static const DINFOX_register_access_t NODE_REG_ACCESS[NODE_REG_ADDR_LAST] = {
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

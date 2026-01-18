/*
 * node.h
 *
 *  Created on: 18 feb. 2023
 *      Author: Ludo
 */

#ifndef __NODE_H__
#define __NODE_H__

#include "node_status.h"
#include "types.h"
#include "una.h"

/*** NODE structures ***/

/*!******************************************************************
 * \enum NODE_state_t
 * \brief NODE states list.
 *******************************************************************/
typedef enum {
    NODE_STATE_IDLE = 0,
    NODE_STATE_RUNNING,
    NODE_STATE_LAST
} NODE_state_t;

/*** NODE functions ***/

/*!******************************************************************
 * \fn NODE_status_t NODE_init(void)
 * \brief Init node driver.
 * \param[in]   none
 * \param[out]  none
 * \retval      Function execution status.
 *******************************************************************/
NODE_status_t NODE_init(void);

/*!******************************************************************
 * \fn NODE_status_t NODE_de_init(void)
 * \brief Release node driver.
 * \param[in]   none
 * \param[out]  none
 * \retval      Function execution status.
 *******************************************************************/
NODE_status_t NODE_de_init(void);

/*!******************************************************************
 * \fn NODE_status_t NODE_process(void)
 * \brief Execute node tasks.
 * \param[in]   none
 * \param[out]  none
 * \retval      Function execution status.
 *******************************************************************/
NODE_status_t NODE_process(void);

#ifdef MPMCM
/*!******************************************************************
 * \fn NODE_status_t NODE_tick_second(void)
 * \brief Execute tick second tasks.
 * \param[in]   none
 * \param[out]  none
 * \retval      Function execution status.
 *******************************************************************/
NODE_status_t NODE_tick_second(void);
#endif

/*!******************************************************************
 * \fn NODE_state_t NODE_get_state(void)
 * \brief Get node state.
 * \param[in]   none
 * \param[out]  none
 * \retval      Current node state.
 *******************************************************************/
NODE_state_t NODE_get_state(void);

/*!******************************************************************
 * \fn NODE_status_t NODE_write_register(uint8_t reg_addr, uint32_t reg_value, uint32_t reg_mask)
 * \brief Write node register.
 * \param[in]   reg_addr: Address of the register to write.
 * \param[in]   reg_value: Value to write in register.
 * \param[in]   reg_mask: Writing operation mask.
 * \param[out]  none
 * \retval      Function execution status.
 *******************************************************************/
NODE_status_t NODE_write_register(uint8_t reg_addr, uint32_t reg_value, uint32_t reg_mask);

/*!******************************************************************
 * \fn NODE_status_t NODE_read_register(uint8_t reg_addr, uint32_t* reg_value)
 * \brief Read node register.
 * \param[in]   reg_addr: Address of the register to read.
 * \param[out]  reg_value: Pointer to the register value.
 * \retval      Function execution status.
 *******************************************************************/
NODE_status_t NODE_read_register(uint8_t reg_addr, uint32_t* reg_value);

/*******************************************************************/
#define NODE_exit_error(base) { ERROR_check_exit(node_status, NODE_SUCCESS, base) }

/*******************************************************************/
#define NODE_stack_error(base) { ERROR_check_stack(node_status, NODE_SUCCESS, base) }

/*******************************************************************/
#define NODE_stack_exit_error(base, code) { ERROR_check_stack_exit(node_status, NODE_SUCCESS, base, code) }

#endif /* __NODE_H__ */

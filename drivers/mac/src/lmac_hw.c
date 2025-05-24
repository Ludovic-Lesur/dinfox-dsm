/*
 * lmac_hw.c
 *
 *  Created on: 27 nov. 2024
 *      Author: Ludo
 */

#include "lmac_hw.h"

#ifndef LMAC_DRIVER_DISABLE_FLAGS_FILE
#include "lmac_driver_flags.h"
#endif
#include "error.h"
#include "error_base.h"
#include "lmac.h"
#include "lpuart.h"
#include "mcu_mapping.h"
#include "nvic_priority.h"
#include "nvm.h"
#include "nvm_address.h"
#include "types.h"
#include "una.h"

#ifndef LMAC_DRIVER_DISABLE

/*** LMAC HW functions ***/

/*******************************************************************/
LMAC_status_t LMAC_HW_init(uint32_t baud_rate, LMAC_rx_irq_cb_t rx_irq_callback, uint8_t* self_address) {
    // Local variables.
    LMAC_status_t status = LMAC_SUCCESS;
    NVM_status_t nvm_status = NVM_SUCCESS;
    LPUART_status_t lpuart_status = LPUART_SUCCESS;
    LPUART_configuration_t lpuart_config;
#ifdef MPMCM
    uint32_t tmp_u32 = 0;
#endif
    // Read self address.
#ifdef MPMCM
    nvm_status = NVM_read_word(NVM_ADDRESS_SELF_ADDRESS, &tmp_u32);
    (*self_address) = (UNA_node_address_t) tmp_u32;
#else
    nvm_status = NVM_read_byte(NVM_ADDRESS_SELF_ADDRESS, self_address);
#endif
    NVM_exit_error(LMAC_ERROR_BASE_NVM);
    // Init LPUART.
    lpuart_config.baud_rate = baud_rate;
    lpuart_config.nvic_priority = NVIC_PRIORITY_RS485;
    lpuart_config.rxne_irq_callback = rx_irq_callback;
    lpuart_config.self_address = (*self_address);
    lpuart_config.rs485_mode = LPUART_RS485_MODE_ADDRESSED;
    lpuart_status = LPUART_init(&LPUART_GPIO_RS485, &lpuart_config);
    LPUART_exit_error(LMAC_ERROR_BASE_HW_INTERFACE);
errors:
    return status;
}

/*******************************************************************/
LMAC_status_t LMAC_HW_de_init(void) {
    // Local variables.
    LMAC_status_t status = LMAC_SUCCESS;
    LPUART_status_t lpuart_status = LPUART_SUCCESS;
    // Release LPUART.
    lpuart_status = LPUART_de_init(&LPUART_GPIO_RS485);
    LPUART_stack_error(ERROR_BASE_LMAC + LMAC_ERROR_BASE_HW_INTERFACE);
    return status;
}

/*******************************************************************/
LMAC_status_t LMAC_HW_enable_rx(void) {
    // Local variables.
    LMAC_status_t status = LMAC_SUCCESS;
    LPUART_status_t lpuart_status = LPUART_SUCCESS;
    // Enable receiver.
    lpuart_status = LPUART_enable_rx();
    LPUART_exit_error(LMAC_ERROR_BASE_HW_INTERFACE);
errors:
    return status;
}

/*******************************************************************/
LMAC_status_t LMAC_HW_disable_rx(void) {
    // Local variables.
    LMAC_status_t status = LMAC_SUCCESS;
    LPUART_status_t lpuart_status = LPUART_SUCCESS;
    // Disable receiver.
    lpuart_status = LPUART_disable_rx();
    LPUART_exit_error(LMAC_ERROR_BASE_HW_INTERFACE);
errors:
    return status;
}

/*******************************************************************/
LMAC_status_t LMAC_HW_write(uint8_t* data, uint32_t data_size_bytes) {
    // Local variables.
    LMAC_status_t status = LMAC_SUCCESS;
    LPUART_status_t lpuart_status = LPUART_SUCCESS;
    // Send bytes over LPUART.
    lpuart_status = LPUART_write(data, data_size_bytes);
    LPUART_exit_error(LMAC_ERROR_BASE_HW_INTERFACE);
errors:
    return status;
}

/*******************************************************************/
void LMAC_HW_stack_error(LMAC_status_t status) {
    ERROR_stack_add(ERROR_BASE_LMAC + status);
}

#endif /* LMAC_DRIVER_DISABLE */

/*
 * spi.h
 *
 *  Created on: 16 aug. 2020
 *      Author: Ludo
 */

#ifndef __SPI_H__
#define __SPI_H__

#include "lptim.h"
#include "types.h"

/*** SPI macros ***/

#define SPI_POWER_ON_DELAY_MS	50

/*** SPI structures ***/

/*!******************************************************************
 * \enum SPI_status_t
 * \brief SPI driver error codes.
 *******************************************************************/
typedef enum {
	SPI_SUCCESS = 0,
	SPI_ERROR_NULL_PARAMETER,
	SPI_ERROR_TX_BUFFER_EMPTY,
	SPI_ERROR_RX_TIMEOUT,
	SPI_ERROR_BASE_LPTIM = 0x0100,
	SPI_ERROR_BASE_LAST = (SPI_ERROR_BASE_LPTIM + LPTIM_ERROR_BASE_LAST)
} SPI_status_t;

/*** SPI functions ***/

#ifdef UHFM
/*!******************************************************************
 * \fn void SPI1_init(void)
 * \brief Init SPI1 peripheral.
 * \param[in]  	none
 * \param[out] 	none
 * \retval		none
 *******************************************************************/
void SPI1_init(void);
#endif

#ifdef UHFM
/*!******************************************************************
 * \fn SPI_status_t SPI1_power_on(void)
 * \brief Power on all slaves connected to the SPI1 bus.
 * \param[in]  	none
 * \param[out] 	none
 * \retval		Function execution status.
 *******************************************************************/
SPI_status_t SPI1_power_on(void);
#endif

#ifdef UHFM
/*!******************************************************************
 * \fn void SPI1_power_off(void)
 * \brief Power off all slaves connected to the SPI1 bus.
 * \param[in]  	none
 * \param[out] 	none
 * \retval		none
 *******************************************************************/
void SPI1_power_off(void);
#endif

#ifdef UHFM
/*!******************************************************************
 * \fn SPI_status_t SPI1_write_byte(uint8_t tx_data)
 * \brief Write byte on SPI1 bus.
 * \param[in]	tx_data: Byte to send.
 * \param[out] 	none
 * \retval		Function execution status.
 *******************************************************************/
SPI_status_t SPI1_write_byte(uint8_t tx_data);
#endif

#ifdef UHFM
/*!******************************************************************
 * \fn SPI_status_t SPI1_read_byte(uint8_t tx_data, uint8_t* rx_data)
 * \brief Write byte on SPI1 bus.
 * \param[in]	tx_data: Byte to send.
 * \param[out] 	rx_data: Pointer to byte that will contain the read data.
 * \retval		Function execution status.
 *******************************************************************/
SPI_status_t SPI1_read_byte(uint8_t tx_data, uint8_t* rx_data);
#endif

/*******************************************************************/
#define SPI1_check_status(error_base) { if (spi1_status != SPI_SUCCESS) { status = error_base + spi1_status; goto errors; } }

/*******************************************************************/
#define SPI1_stack_error() { ERROR_stack_error(spi1_status, SPI_SUCCESS, ERROR_BASE_SPI1); }

/*******************************************************************/
#define SPI1_print_error() { ERROR_print_error(spi1_status, SPI_SUCCESS, ERROR_BASE_SPI1); }

#endif /* __SPI_H__ */

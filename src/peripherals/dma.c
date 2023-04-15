/*
 * dma.c
 *
 *  Created on: 16 aug. 2020
 *      Author: Ludo
 */

#include "dma.h"

#include "dma_reg.h"
#include "neom8n.h"
#include "nvic.h"
#include "rcc_reg.h"
#include "spi_reg.h"
#include "types.h"
#include "usart_reg.h"

/*** DMA local global variables ***/

#ifdef UHFM
static volatile uint8_t dma1_channel3_tcif = 0;
#endif

/*** DMA local functions ***/

#ifdef UHFM
/* DMA1 CHANNEL 3 INTERRUPT HANDLER.
 * @param:	None.
 * @return:	None.
 */
void __attribute__((optimize("-O0"))) DMA1_Channel2_3_IRQHandler(void) {
	// Transfer complete interrupt (TCIF3='1').
	if (((DMA1 -> ISR) & (0b1 << 9)) != 0) {
		// Set local flag.
		if (((DMA1 -> CCR3) & (0b1 << 1)) != 0) {
			dma1_channel3_tcif = 1;
		}
		// Clear flag.
		DMA1 -> IFCR |= (0b1 << 9); // CTCIF3='1'.
	}
}
#endif

#ifdef GPSM
/* DMA1 CHANNEL 6 INTERRUPT HANDLER.
 * @param:	None.
 * @return:	None.
 */
void __attribute__((optimize("-O0"))) DMA1_Channel4_5_6_7_IRQHandler(void) {
	// Transfer complete interrupt (TCIF6='1').
	if (((DMA1 -> ISR) & (0b1 << 21)) != 0) {
		// Switch DMA buffer without decoding.
		if (((DMA1 -> CCR6) & (0b1 << 1)) != 0) {
			NEOM8N_switch_dma_buffer(0);
		}
		// Clear flag.
		DMA1 -> IFCR |= (0b1 << 21); // CTCIF6='1'.
	}
}
#endif

/*** DMA functions ***/

#ifdef UHFM
/* CONFIGURE DMA1 CHANNEL3 FOR SPI1 TX TRANSFER (S2LP TX POLAR MODULATION).
 * @param:	None.
 * @return:	None.
 */
void DMA1_init_channel3(void) {
	// Enable peripheral clock.
	RCC -> AHBENR |= (0b1 << 0); // DMAEN='1'.
	// Disable DMA channel before configuration (EN='0').
	// Memory and peripheral data size are 8 bits (MSIZE='00' and PSIZE='00').
	// Disable memory to memory mode (MEM2MEM='0').
	// Peripheral increment mode disabled (PINC='0').
	// Circular mode disabled (CIRC='0').
	// Read from memory (DIR='1').
	// Very high priority (PL='11').
	// Memory increment mode enabled (MINC='1').
	// Enable transfer complete interrupt (TCIE='1').
	DMA1 -> CCR3 |= (0b11 << 12) | (0b1 << 7) | (0b1 << 4) | (0b1 << 1);
	// Configure peripheral address.
	DMA1 -> CPAR3 = (uint32_t) &(SPI1 -> DR); // Peripheral address = SPI1 TX register.
	// Configure channel 3 for SPI1 TX (request number 1).
	DMA1 -> CSELR |= (0b0001 << 8); // DMA channel mapped on SPI1_TX (C3S='0001').
	// Set interrupt priority.
	NVIC_set_priority(NVIC_INTERRUPT_DMA1_CH_2_3, 1);
}
#endif

#ifdef UHFM
/* START DMA1 CHANNEL 3 TRANSFER.
 * @param:	None.
 * @return:	None.
 */
void DMA1_start_channel3(void) {
	// Clear all flags.
	dma1_channel3_tcif = 0;
	DMA1 -> IFCR |= 0x00000F00;
	NVIC_enable_interrupt(NVIC_INTERRUPT_DMA1_CH_2_3);
	// Start transfer.
	DMA1 -> CCR3 |= (0b1 << 0); // EN='1'.
}
#endif

#ifdef UHFM
/* STOP DMA1 CHANNEL 3 TRANSFER.
 * @param:	None.
 * @return:	None.
 */
void DMA1_stop_channel3(void) {
	// Stop transfer.
	dma1_channel3_tcif = 0;
	DMA1 -> CCR3 &= ~(0b1 << 0); // EN='0'.
	NVIC_disable_interrupt(NVIC_INTERRUPT_DMA1_CH_2_3);
}
#endif

#ifdef UHFM
/* SET DMA1 CHANNEL 3 SOURCE BUFFER ADDRESS.
 * @param dest_buf_addr:	Address of source buffer (Sigfox modulation stream).
 * @param dest_buf_size:	Size of destination buffer.
 * @return:					None.
 */
void DMA1_set_channel3_source_addr(uint32_t source_buf_addr, uint16_t source_buf_size) {
	// Set address and buffer size.
	DMA1 -> CMAR3 = source_buf_addr;
	DMA1 -> CNDTR3 = source_buf_size;
}
#endif

#ifdef UHFM
/* GET DMA1 CHANNEL 3 TRANSFER STATUS.
 * @param:	None.
 * @return:	'1' if the transfer is complete, '0' otherwise.
 */
uint8_t DMA1_get_channel3_status(void) {
	return dma1_channel3_tcif;
}

#endif

#ifdef GPSM
/* CONFIGURE DMA1 CHANNEL 6 FOR LPUART RX TRANSFER (NMEA FRAMES FROM GPS MODULE).
 * @param:	None.
 * @return:	None.
 */
void DMA1_init_channel6(void) {
	// Enable peripheral clock.
	RCC -> AHBENR |= (0b1 << 0); // DMAEN='1'.
	// Memory and peripheral data size are 8 bits (MSIZE='00' and PSIZE='00').
	// Disable memory to memory mode (MEM2MEM='0').
	// Peripheral increment mode disabled (PINC='0').
	// Circular mode disabled (CIRC='0').
	// Read from peripheral (DIR='0').
	// Very high priority (PL='11').
	// Memory increment mode enabled (MINC='1').
	// Enable transfer complete interrupt (TCIE='1').
	DMA1 -> CCR6 |= (0b11 << 12) | (0b1 << 7) | (0b1 << 1);
	// Configure peripheral address.
	DMA1 -> CPAR6 = (uint32_t) &(USART2 -> RDR); // Peripheral address = LPUART RX register.
	// Configure channel 3 for USART2 RX (request number 4).
	DMA1 -> CSELR |= (0b0100 << 20); // DMA channel mapped on USART2_RX (C6S='0100').
	// Set interrupt priority.
	NVIC_set_priority(NVIC_INTERRUPT_DMA1_CH_4_7, 1);
}
#endif

/* START DMA1 CHANNEL 6 TRANSFER.
 * @param:	None.
 * @return:	None.
 */
void DMA1_start_channel6(void) {
	// Clear all flags.
	DMA1 -> IFCR |= 0x00F00000;
	NVIC_enable_interrupt(NVIC_INTERRUPT_DMA1_CH_4_7);
	// Start transfer.
	DMA1 -> CCR6 |= (0b1 << 0); // EN='1'.
}

/* STOP DMA1 CHANNEL 6 TRANSFER.
 * @param:	None.
 * @return:	None.
 */
void DMA1_stop_channel6(void) {
	// Stop transfer.
	DMA1 -> CCR6 &= ~(0b1 << 0); // EN='0'.
	NVIC_disable_interrupt(NVIC_INTERRUPT_DMA1_CH_4_7);
}

/* SET DMA1 CHANNEL 6 DESTINATION BUFFER ADDRESS.
 * @param dest_buf_addr:	Address of destination buffer (NMEA frame).
 * @param dest_buf_size:	Size of destination buffer.
 * @return:					None.
 */
void DMA1_set_channel6_dest_addr(uint32_t dest_buf_addr, uint16_t dest_buf_size) {
	// Set address and buffer size.
	DMA1 -> CMAR6 = dest_buf_addr;
	DMA1 -> CNDTR6 = dest_buf_size;
}

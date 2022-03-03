/*
 * lpuart.c
 *
 *  Created on: 9 juil. 2019
 *      Author: Ludo
 */

#include "lpuart.h"

#include "at.h"
#include "gpio.h"
#include "lpuart_reg.h"
#include "mapping.h"
#include "nvic.h"
#include "rcc.h"
#include "rcc_reg.h"

/*** LPUART local macros ***/

#define LPUART_BAUD_RATE 			9600

#define LPUART_ADDR_LENGTH_BYTES	1
#define LPUART_ADDR_NODE			0x31
#define LPUART_ADDR_MASTER			0x65

#define LPUART_TIMEOUT_COUNT		100000

/*** LPUART local global variables ***/

static volatile unsigned int lpuart_irq_count = 0;

/*** LPUART local functions ***/

void LPUART1_IRQHandler(void) {
	// RXNE interrupt.
	if (((LPUART1 -> ISR) & (0b1 << 5)) != 0) {
		// Increment IRQ count.
		lpuart_irq_count++;
		// Do not transmit address bytes to applicative layer.
		if (lpuart_irq_count > LPUART_ADDR_LENGTH_BYTES) {
			// Fill AT RX buffer with incoming byte.
			AT_fill_rx_buffer(LPUART1 -> RDR);
		}
		// Clear RXNE flag.
		LPUART1 -> RQR |= (0b1 << 3);

	}
	// Overrun error interrupt.
	if (((LPUART1 -> ISR) & (0b1 << 3)) != 0) {
		// Clear ORE flag.
		LPUART1 -> ICR |= (0b1 << 3);
	}
}

/* FILL LPUART1 TX BUFFER WITH A NEW BYTE.
 * @param tx_byte:	Byte to append.
 * @return:			None.
 */
static void LPUART1_fill_tx_buffer(unsigned char tx_byte) {
	// Fill transmit register.
	LPUART1 -> TDR = tx_byte;
	// Wait for transmission to complete.
	unsigned int loop_count = 0;
	while (((LPUART1 -> ISR) & (0b1 << 7)) == 0) {
		// Wait for TXE='1' or timeout.
		loop_count++;
		if (loop_count > LPUART_TIMEOUT_COUNT) break;
	}
}

/*** LPUART functions ***/

/* CONFIGURE LPUART1.
 * @param:	None.
 * @return:	None.
 */
void LPUART1_init(void) {
	// Select LSE as clock source.
	RCC -> CCIPR &= ~(0b11 << 10); // Reset bits 10-11.
	RCC -> CCIPR |= (0b11 << 10); // LPUART1SEL='11'.
	// Enable peripheral clock.
	RCC -> APB1ENR |= (0b1 << 18); // LPUARTEN='1'.
	// Configure TX and RX GPIOs.
	GPIO_configure(&GPIO_LPUART1_TX, GPIO_MODE_ALTERNATE_FUNCTION, GPIO_TYPE_PUSH_PULL, GPIO_SPEED_LOW, GPIO_PULL_NONE);
	GPIO_configure(&GPIO_LPUART1_RX, GPIO_MODE_ALTERNATE_FUNCTION, GPIO_TYPE_PUSH_PULL, GPIO_SPEED_LOW, GPIO_PULL_NONE);
	GPIO_configure(&GPIO_LPUART1_DE, GPIO_MODE_ALTERNATE_FUNCTION, GPIO_TYPE_PUSH_PULL, GPIO_SPEED_LOW, GPIO_PULL_NONE); // External pull-down resistor present.
	LPUART1_disable_rx();
	// Reset peripheral.
	LPUART1 -> CR1 &= 0xEC008000; // Disable peripheral (UE='0'), 1 stop bit and 8 data bits (M='00').
	LPUART1 -> CR2 &= 0x00F04FEF; // 1 stop bit (STOP='00').
	LPUART1 -> CR3 &= 0xFF0F0836;
	// TX configuration.
	LPUART1 -> CR1 |= (0b11111 << 21) | (0b11111 << 16);
	LPUART1 -> CR3 |= (0b1 << 14); // Enable DE pin.
	// RX configuration.
	LPUART1 -> CR3 |= (0b1 << 12); // No overrun detection (OVRDIS='0').
	LPUART1 -> CR1 |= (0b1 << 13); // Enable mute mode (MME='1').
	LPUART1 -> CR1 |= (0b1 << 11); // Use address for wake-up (WAKE='1').
	LPUART1 -> CR1 |= (0b1 << 5); // Enable RXNE interrupt (RXNEIE='1').
	LPUART1 -> CR1 |= (0b1 << 1); // LPUART enabled in stop mode (UESM='1').
	LPUART1 -> CR2 |= (LPUART_ADDR_NODE & 0x7F) << 24;
	LPUART1 -> CR2 |= (0b1 << 4); // Use 7-bits address.
	LPUART1 -> CR3 |= (0b1 << 23); // LPUART clock enabled in stop mode (UCESM='1').
	// Baud rate.
	unsigned int brr = (RCC_LSE_FREQUENCY_HZ * 256);
	brr /= LPUART_BAUD_RATE;
	LPUART1 -> BRR = (brr & 0x000FFFFF); // BRR = (256*fCK)/(baud rate). See p.730 of RM0377 datasheet.
	// Set interrupt priority.
	NVIC_set_priority(NVIC_IT_LPUART1, 0);
	// Enable transmitter.
	LPUART1 -> CR1 |= (0b1 << 3); // TE='1'.
	// Enable peripheral.
	LPUART1 -> CR1 |= (0b1 << 0); // UE='1'.
}

/* EANABLE LPUART RX OPERATION.
 * @param:	None.
 * @return:	None.
 */
void LPUART1_enable_rx(void) {
	// Enable receiver.
	LPUART1 -> CR1 |= (0b1 << 2); // RE='1'.
	// Mute mode request.
	LPUART1 -> RQR |= (0b1 << 2); // MMRQ='1'.
	// Enable interrupt.
	NVIC_enable_interrupt(NVIC_IT_LPUART1);
	// Enable RS485 receiver.
	GPIO_configure(&GPIO_LPUART1_NRE, GPIO_MODE_ANALOG, GPIO_TYPE_OPEN_DRAIN, GPIO_SPEED_LOW, GPIO_PULL_NONE); // External pull-down resistor present.
}

/* DISABLE LPUART RX OPERATION.
 * @param:	None.
 * @return:	None.
 */
void LPUART1_disable_rx(void) {
	// Reset IRQ count for next command reception.
	lpuart_irq_count = 0;
	// Disable RS485 receiver.
	GPIO_configure(&GPIO_LPUART1_NRE, GPIO_MODE_OUTPUT, GPIO_TYPE_PUSH_PULL, GPIO_SPEED_LOW, GPIO_PULL_NONE);
	GPIO_write(&GPIO_LPUART1_NRE, 1);
	// Disable receiver.
	LPUART1 -> CR1 &= ~(0b1 << 2); // RE='0'.
	// Disable interrupt.
	NVIC_disable_interrupt(NVIC_IT_LPUART1);
}

/* GET LPUART RX EVENT FLAG.
 * @param:	None.
 * @return:	1 if any LPUART RX interrupt occured, 0 otherwise.
 */
unsigned char LPUART1_get_rx_flag(void) {
	// Return flag.
	return ((lpuart_irq_count > 0) ? 1 : 0);
}

/* SEND A BYTE ARRAY THROUGH LPUART1.
 * @param tx_string:	Byte array to send.
 * @return:				None.
 */
void LPUART1_send_string(char* tx_string) {
	// Send master address.
	LPUART1_fill_tx_buffer(LPUART_ADDR_MASTER | 0x80);
	// Fill TX buffer with new bytes.
	while (*tx_string) {
		LPUART1_fill_tx_buffer((unsigned char) *(tx_string++));
	}
}

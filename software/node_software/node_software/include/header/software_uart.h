/*
* software_uart.h
*
* Created: 30/10/2020 21:54:47
*  Author: sylvain
*/

/* up to 19200 Baud, 8 bit data wthout parity bit */

#ifndef SOFTWARE_UART_H_
#define SOFTWARE_UART_H_

#define SOFTWARE_UART_TX_DDR DDRD
#define SOFTWARE_UART_TX_DDR_BIT DDD4
#define SOFTWARE_UART_TX_PORT PORTD
#define SOFTWARE_UART_TX_PORT_BIT PORTD4

#define SOFTWARE_UART_RX_DDR DDRD
#define SOFTWARE_UART_RX_DDR_BIT DDD5
#define SOFTWARE_UART_RX_PORT PORTD
#define SOFTWARE_UART_RX_PORT_BIT PORTD5
#define SOFTWARE_UART_RX_PIN PIND
#define SOFTWARE_UART_RX_PIN_BIT PIND5

#define SOFTWARE_UART_BAUD_RATE 9600

#define SOFTWARE_UART_BIT_TIME_TO_TICK_DIVIDER 20

#include "software_uart.c"

extern void software_uart_init();
extern void software_uart_end();
extern void software_uart_write_bytes(char *buffer, uint8_t number);
extern uint8_t software_uart_read_bytes(char *buffer, uint8_t number, uint16_t timeout);

#endif

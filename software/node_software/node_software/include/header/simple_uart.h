 /* 
 * hx711.h
 *
 * Created: 30/10/2020 21:54:47
 *  Author: sylvain
 */

#ifndef SIMPLE_UART_H_
#define SIMPLE_UART_H_

#include "simple_uart.c"

extern void serial_init(uint32_t baud);
extern void serial_send(char *string);
extern char serial_receive();

#endif

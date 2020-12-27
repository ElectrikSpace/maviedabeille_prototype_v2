/*
* spi_master.h
*
* Created: 30/10/2020 21:54:47
*  Author: sylvain
*/

#ifndef SPI_MASTER_H_
#define SPI_MASTER_H_

/* this library do not take care of chip select pin definition usage */

// data transmitting order
#define SPI_DATA_ORDER 0 // 0 for MSB first (default), 1 for LSB first
// SPI mode
#define SPI_CPOL 0
#define SPI_CPHA 0
// SPI speed
#define SPI_SPI2X 0
#define SPI_SPR1 0
#define SPI_SPR0 1 // here FCPU/16
// buffer size
#define SPI_BUFFER_SIZE 10

extern uint8_t spi_buffer[SPI_BUFFER_SIZE];
extern uint8_t spi_buffer_pointer;

#include "spi_master.c"

extern void spi_init();
extern void spi_end();
extern void spi_send(uint8_t *data, uint8_t size);
extern uint8_t spi_receive_byte();
extern void spi_flush_buffer();
extern void spi_start_listen();
extern void spi_stop_listen();
extern uint8_t spi_read_from_buffer(uint8_t *data, uint8_t size);

#endif

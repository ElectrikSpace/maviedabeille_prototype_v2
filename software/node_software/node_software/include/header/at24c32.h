/*
* software_uart.h
*
* Created: 30/10/2020 21:54:47
*  Author: sylvain
*/

#ifndef AT24C32_H_
#define AT24C32_H_

/* i2c buffer size must be greater than 4 to operate */

#define AT24C32_ADDRESS 0x80

#include "at24c32.h"

/* all function return 1 if success, 0 if fail */
extern uint8_t at24c32_ping();
extern uint8_t at24c32_byte_write(uint16_t address, uint8_t byte);
extern uint8_t at24c32_page_write(uint16_t address, uint8_t *data, uint8_t bytes);
extern uint8_t at24c32_current_address_read(uint8_t *byte);
extern uint8_t at24c32_random_read(uint16_t address, uint8_t *byte);
extern uint8_t at24c32_sequential_read(uint16_t address, uint8_t *data, uint8_t bytes);

#endif

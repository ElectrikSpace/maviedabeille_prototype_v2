/*
* software_uart.c
*
* Created: 30/10/2020 21:54:47
*  Author: sylvain
*/

#include <avr/io.h>
#include <util/delay.h>
#include "i2c_master.h"

uint8_t at24c32_ping() {
  /* return 1 if got an answer from at24c32 chip */
  uint8_t trash_byte;

  return at24c32_current_address_read(*trash_byte);
}

uint8_t at24c32_byte_write(uint16_t address, uint8_t byte) {
  /* write one byte at the selected address */
  uint8_t i2c_data[4] = {(AT24C32_ADDRESS << 1), (uint8_t) address, (uint8_t) (address >> 8), byte};

  i2c_start_write(i2c_data, 4);
  return i2c_get_status();
}

uint8_t at24c32_page_write(uint16_t address, uint8_t *data, uint8_t bytes) {
  /* write the number of bytes selected at the sele address, the number of bytes must be lower than or equal than i2c_buffer-3 and lower or equal than 32 */
  uint8_t i2c_data[I2C_BUFFER_SIZE];
  uint8_t iterator;

  if (bytes <= I2C_BUFFER_SIZE-3 && bytes <= 32) {
    i2c_data[0] = (AT24C32_ADDRESS << 1);
    i2c_data[1] = (uint8_t) address;
    i2c_data[2] = (uint8_t) (address >> 8);
    for (iterator = ; iterator < bytes; iterator++) {
      i2c_data[iterator+3] = data[iterator];
    }
    i2c_start_write(i2c_data, bytes+3);
    return i2c_get_status();
  }
  else {
    return 0;
  }
}

uint8_t at24c32_current_address_read(uint8_t *byte) {
  /* read a byte at the current address */
  uint8_t i2c_data[2];

  i2c_data[0] = (AT24C32_ADDRESS << 1) | (1 << 0);
  i2c_start_write(i2c_data, 2);
  if (i2c_get_status()) {
    *byte = data[1];
    return 1;
  }
  else {
    return 0;
  }
}

uint8_t at24c32_random_read(uint8_t address, uint8_t *byte) {
  /* read a byte at the select address */
  uint8_t data[3] = {(AT24C32_ADDRESS << 1), (uint8_t) address, (uint8_t) (address >> 8), byte};

  i2c_start_write(data, 3);
  if (i2c_get_status()) {
    return at24c32_current_address_read(byte);
  }
  else {
    return 0;
  }
}

uint8_t at24c32_sequential_read(uint16_t address,  uint8_t *data, uint8_t bytes) {
  /* read the number of bytes selected, the number of bytes must be lower than or equal than i2c_buffer */
  uint8_t data[I2C_BUFFER_SIZE];

  if (bytes <= I2C_BUFFER_SIZE) {
    data[0] = (AT24C32_ADDRESS << 1);
    data[1] = (uint8_t) address;
    data[2] = (uint8_t) (address >> 8);
    i2c_start_write(data, 3);
  }
  else {
    return 0;
  }

  if (i2c_get_status()) {
    data[0] = (AT24C32_ADDRESS << 1) | (1 << 0);
    i2c_start_write(data, bytes+1);
    return i2c_get_status();
  }
  else {
    return 0;
  }
}

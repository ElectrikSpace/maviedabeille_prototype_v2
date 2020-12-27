 /*
 * i2c_master.h
 *
 * Created: 30/10/2020 21:54:47
 *  Author: sylvain
 */

#ifndef I2C_MASTER_H_
#define I2C_MASTER_H_

#define I2C_FREQUENCY_REGISTER 0x20         // TWI Bit rate Register setting
// here for 100kHz clock speed

#define I2C_BUFFER_SIZE 10 // max number of bytes in the buffer

#include "i2c_master.c"

extern uint8_t i2c_status_OK; // 1 if OK, 0 otherwise
extern uint8_t i2c_data_size; // size of the message data
extern uint8_t i2c_buffer[I2C_BUFFER_SIZE]; // initialize the buffer

void i2c_init();
void i2c_end();
uint8_t i2c_is_transceiver_busy();
uint8_t i2c_get_status();
void i2c_start_write(uint8_t *data, uint8_t size);
void i2c_resend_last_data();
uint8_t i2c_read_buffer(uint8_t *data, uint8_t size);

#endif

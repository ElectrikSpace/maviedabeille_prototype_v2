/*
* sfm10r1.c
*
* Created: 30/10/2020 21:54:47
*  Author: sylvain
*/

#include <avr/io.h>
#include <util/delay.h>
#include "software_uart.h"

void sfm10r1_reset() {
	/* hard reset sfm10r1 */
	SFM10R1_RESET_DDR = SFM10R1_RESET_DDR | (1 << SFM10R1_RESET_DDR_BIT);

	// reset signal must be high for at least 600us
	SFM10R1_RESET_PORT = SFM10R1_RESET_PORT | (1 << SFM10R1_RESET_PORT_BIT);
	_delay_us(600);

	// reset for at least 1us
	SFM10R1_RESET_PORT = SFM10R1_RESET_PORT & ~(1 << SFM10R1_RESET_PORT_BIT);
	_delay_us(1);

	// set reset signal to high
	SFM10R1_RESET_PORT = SFM10R1_RESET_PORT | (1 << SFM10R1_RESET_PORT_BIT);
	
	// release reset pin
	SFM10R1_RESET_DDR = SFM10R1_RESET_DDR & ~(1 << SFM10R1_RESET_DDR_BIT);
}

void sfm10r1_init() {
  /* start UART bus and hard reset sfm10r1 */
  
  // start UART bus
  software_uart_init();
  
  // hard reset SFM10R1
  sfm10r1_reset();
}

void sfm10r1_end() {
  /* release UART bus */

  // make sure reset signal is high
  sfm10r1_reset();

  // release UART pins
  software_uart_end();
}

uint8_t sfm10r1_ping() {
 /* return 1 if got an respond from sfm10r1 */
 char buffer[4] = {'A', 'T', '\r'};

 // send AT
 software_uart_write_bytes(buffer, 3);

 // read result and check if it is OK
 if (software_uart_read_bytes(buffer, 4, SFM10R1_UART_TIMEOUT)) {
   if (buffer[0] == 'O' && buffer[1] == 'K') {
     return 1;
   }
   else {
     return 0;
   }
 }
 else {
   return 0;
 }
}

uint8_t sfm10r1_get_ID(uint8_t *id) {
 /* return 1 if success and put ID in *ID (must have 8 bytes size) */
 uint8_t iterator;
 char buffer[10] = {'A', 'T', '$', 'I', '=', '1', '0', '\r'};

 // send request
 software_uart_write_bytes(buffer, 8);

 // read answer
 if (!software_uart_read_bytes(buffer, 10, SFM10R1_UART_TIMEOUT)) {
   return 0;
 }

 // copy data
 for (iterator = 0; iterator < 8; iterator++) {
   id[iterator] = buffer[iterator];
 }

 return 1;
}

uint8_t sfm10r1_get_PAC(uint8_t *pac) {
 /* return 1 if success and put PAC in *PAC (must have 16 bytes size) */
 uint8_t iterator;
 char buffer[18] = {'A', 'T', '$', 'I', '=', '1', '1', '\r'};

 // send request
 software_uart_write_bytes(buffer, 8);

 // read answer
 if (!software_uart_read_bytes(buffer, 18, SFM10R1_UART_TIMEOUT)) {
   return 0;
 }

 // copy data
 for (iterator = 0; iterator < 16; iterator++) {
   pac[iterator] = buffer[iterator];
 }

 return 1;
}

uint8_t sfm10r1_send_data(char *data, uint8_t size) {
 /* return 1 if success, send data (1-12 bytes) and wait for OK, so it can take several seconds */
 uint8_t iterator;
 char buffer[32] = {'A', 'T', '$', 'S', 'F', '='};

 // copy data in buffer
 for (iterator = 0; iterator < size; iterator++) {
   buffer[iterator+6] = data[iterator];
 }
 buffer[iterator+6] = '\r';

 // send request
 software_uart_write_bytes(buffer, iterator+7);

 // read answer
 if (!software_uart_read_bytes(buffer, 4, 15000)) {
   return 0;
 }

 // check if it is OK
 if (buffer[0] == 'O' && buffer[1] == 'K') {
   return 1;
 }
 // error
 return 0;
}

uint8_t sfm10r1_send_data_downlink(char *data, uint8_t size, uint8_t *received_data) {
 /* return 1 if success, send data (1-12 bytes) and wait for downlink data which are put in *received_data (1-8 bytes so 8 bytes size) */
 uint8_t iterator;
 char buffer[21] = {'A', 'T', '$', 'S', 'F', '='};

 // copy data in buffer
 for (iterator = 0; iterator < size; iterator++) {
   buffer[iterator+6] = data[iterator];
 }
 buffer[iterator+6] = '\r';
 buffer[iterator+7] = (1 << 0);

 // send request
 software_uart_write_bytes(buffer, iterator+8);

 // read answer
 if (!software_uart_read_bytes(buffer, 10, 10000)) {
   return 0;
 }

 // copy buffer in data
 for (iterator = 0; iterator < 8; iterator++) {
   received_data[iterator] = buffer[iterator];
 }
 return 1;
}

uint8_t sfm10r1_get_temperature(float *temperature) {
  /* return 1 if success, put temperature in *temperature */
  uint8_t uint_value;
  uint8_t counter;
  char buffer[6] = {'A', 'T', '$', 'T', '?', '\r'};

  // send request
  software_uart_write_bytes(buffer, 6);

  // read answer
  if (!software_uart_read_bytes(buffer, 5, SFM10R1_UART_TIMEOUT)) {
    return 0;
  }

  // process data
  *temperature = 0;
  for (counter = 0; counter < 3; counter++) {
	  switch (buffer[counter]) { // convert ASCII character into uint
		case 0x30:
			uint_value = 0;
			break;
		case 0x31:
			uint_value = 1;
			break;
		case 0x32:
			uint_value = 2;
			break;
		case 0x33:
			uint_value = 3;
			break;
		case 0x34:
			uint_value = 4;
			break;
		case 0x35:
			uint_value = 5;
			break;
		case 0x36:
			uint_value = 6;
			break;
		case 0x37:
			uint_value = 7;
			break;	
		case 0x38:
			uint_value = 8;
			break;
		case 0x39:
			uint_value = 9;
			break;
		default:
			// error
			return 0;
	  }
	  if (counter == 0) {
		  *temperature = *temperature + 10*((float) uint_value);
	  }
	  else if (counter == 1) {
		  *temperature = *temperature + ((float) uint_value);
	  }
	  else { // counter == 2
		  *temperature = *temperature + 0.1*((float) uint_value);
	  }
  }

  return 1;
}

uint8_t sfm10r1_get_voltage(float *voltage) {
  /* return 1 if succes, put voltage in *voltage */
  int16_t raw_voltage;
  char buffer[6] = {'A', 'T', '$', 'V', '?', '\r'};

  // send request
  software_uart_write_bytes(buffer, 6);

  // read answer
  if (!software_uart_read_bytes(buffer, 4, SFM10R1_UART_TIMEOUT)) {
    return 0;
  }

  // process data
  raw_voltage = buffer[0] + (buffer[1] << 8);
  *voltage = (float) raw_voltage / 10;

  return 1;
}

uint8_t sfm10r1_get_transmit_repeats(uint8_t *repeats) {
  /* return 1 if success, put transmit repeats value in *repeats */
  char buffer[7] = {'A', 'T', '$', 'T', 'R', '?', '\r'};

  // send request
  software_uart_write_bytes(buffer, 7);

  // read answer
  if (!software_uart_read_bytes(buffer, 1, SFM10R1_UART_TIMEOUT)) {
    return 0;
  }

  // process data
  *repeats = buffer[0];

  return 1;
}

uint8_t sfm10r1_set_transmit_repeats(uint8_t number) {
  /* return 1 if success, set transmit repeats value (0-2) */
  char buffer[8] = {'A', 'T', '$', 'T', 'R', '=', 0x00, '\r'};

  // set the repeat number is the buffer
  if (number == 0) {
	  buffer[6] = '0';
  } 
  else if (number == 1) {
	  buffer[6] = '1';
  }
  else if (number == 2) {
	  buffer[6] = '2';
  }
  else {
	  return 0;
  }

  // send request
  software_uart_write_bytes(buffer, 8);

  // read answer
  if (!software_uart_read_bytes(buffer, 4, SFM10R1_UART_TIMEOUT)) {
    return 0;
  }
  else {
    if (buffer[0] == 'O' && buffer[1] == 'K') {
      return 1;
    }
  }
  return 0;
}

uint8_t sfm10r1_save_config() {
	/* save config register into flash memory of the chip, to keep change after reset or shutdown
		return 1 if it is a success or 0 if it is a failure */
	char buffer[6] = {'A', 'T', '$', 'W', 'R', '\r'};
		
	// send AT
	software_uart_write_bytes(buffer, 6);

	// read result and check if it is OK
	if (software_uart_read_bytes(buffer, 4, SFM10R1_UART_TIMEOUT)) {
		if (buffer[0] == 'O' && buffer[1] == 'K') {
			return 1;
		}
		else {
			return 0;
		}
	}
	else {
		return 0;
	}
}
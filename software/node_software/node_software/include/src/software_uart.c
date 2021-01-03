/*
* software_uart.c
*
* Created: 30/10/2020 21:54:47
*  Author: sylvain
*/

#include <avr/io.h>
#include <util/delay.h>

static const uint8_t tick = (1000000 / SOFTWARE_UART_BAUD_RATE) / SOFTWARE_UART_BIT_TIME_TO_TICK_DIVIDER; // in us
static const uint16_t bit_time = 1000000 / SOFTWARE_UART_BAUD_RATE; // in us
static const uint16_t bit_and_half_time = 1500000 / SOFTWARE_UART_BAUD_RATE; // in us

void software_uart_init() {
  /* setup TX and RX pins, and calculate tick */

  // TX pin
  SOFTWARE_UART_TX_DDR = SOFTWARE_UART_TX_DDR | (1 << SOFTWARE_UART_TX_DDR_BIT);
  SOFTWARE_UART_TX_PORT = SOFTWARE_UART_TX_PORT | (1 << SOFTWARE_UART_TX_PORT_BIT);

  // RX pin
  SOFTWARE_UART_RX_DDR = SOFTWARE_UART_RX_DDR & ~(1 << SOFTWARE_UART_RX_DDR_BIT);
  SOFTWARE_UART_RX_PORT = SOFTWARE_UART_RX_PORT | (1 << SOFTWARE_UART_RX_PORT_BIT);
}

void software_uart_end() {
  /* release TX and RX pins */

  // TX pin
  SOFTWARE_UART_TX_DDR = SOFTWARE_UART_TX_DDR & ~(1 << SOFTWARE_UART_TX_DDR_BIT);
  SOFTWARE_UART_TX_PORT = SOFTWARE_UART_TX_PORT & ~(1 << SOFTWARE_UART_TX_PORT_BIT);

  // RX pin
  SOFTWARE_UART_RX_PORT = SOFTWARE_UART_RX_PORT & ~(1 << SOFTWARE_UART_RX_PORT_BIT);
}

void software_uart_write_bytes(char *buffer, uint8_t number) {
  uint8_t number_iterator;
  uint8_t bit_iterator;
  uint8_t register_low = SOFTWARE_UART_TX_PORT & ~(1 << SOFTWARE_UART_TX_PORT_BIT); // TX pin is low
  uint8_t register_high = SOFTWARE_UART_TX_PORT | (1 << SOFTWARE_UART_TX_PORT_BIT); // TX pin is high

  for (number_iterator = 0; number_iterator < number; number_iterator++) {
    SOFTWARE_UART_TX_PORT = register_low;
    _delay_us(bit_time);
    for (bit_iterator = 0; bit_iterator < 8; bit_iterator++) {
      SOFTWARE_UART_TX_PORT = register_low | ( ( (*(buffer+number_iterator) >> bit_iterator) & (1 << 0) ) << SOFTWARE_UART_TX_PORT_BIT );
	  _delay_us(bit_time);
    }
    SOFTWARE_UART_TX_PORT = register_high;
    _delay_us(bit_time);
  }
}

uint8_t software_uart_read_bytes(char *buffer, uint8_t number, uint16_t timeout) {
  /* buffer size must be at least number, timeout is in ms */
  uint32_t ticks_number = 0;
  uint32_t ticks_counter = 0;
  int byte_iterator = 0;
  uint8_t bit_iterator;
  char *byte;

  // compute timeout variable
  ticks_number = (uint32_t) 1000*timeout/tick;

  // read
  while ((ticks_counter <= ticks_number) & (byte_iterator < number)) {
	  if (!(SOFTWARE_UART_RX_PIN & (1 << SOFTWARE_UART_RX_PIN_BIT))) { // start bit
		  _delay_us(bit_and_half_time);
		  byte = buffer + byte_iterator;
		  *byte = 0x00;
		  for (bit_iterator = 0; bit_iterator < 8; bit_iterator++) {
			  *byte = *byte | (((SOFTWARE_UART_RX_PIN & (1 << SOFTWARE_UART_RX_PIN_BIT)) >> SOFTWARE_UART_RX_PIN_BIT) << bit_iterator);
			  _delay_us(bit_time);
		  }
		  byte_iterator++;
	  }
	  ticks_counter++;
	  _delay_us(tick);
  }

  if (byte_iterator == number) {
    return 1; // success
  }
  else {
    return 0;
  }
}

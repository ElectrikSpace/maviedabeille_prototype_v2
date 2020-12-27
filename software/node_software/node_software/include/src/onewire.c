/*
* onewire.c
*
* Created: 30/10/2020 21:54:47
*  Author: sylvain
*/

#include <avr/io.h>
#include <util/delay.h>

// Buffer to read data or ROM code
static uint8_t DataBytes[9];

void onewire_pin_low() {
  /* switch OneWire pin to output */
  ONEWIRE_DDR = ONEWIRE_DDR | (1 << ONEWIRE_BIT);
}

void onewire_pin_release() {
  /* switch OneWire pin to input */
  ONEWIRE_DDR = ONEWIRE_DDR & ~(1 << ONEWIRE_BIT);
}

uint8_t onewire_pin_read() {
  /* read value of OneWire pin */
  return ( (ONEWIRE_PIN >> ONEWIRE_BIT) & 1);
}

void onewire_init() {
  /* init OneWire protocol */
  ONEWIRE_PORT = ONEWIRE_PORT & ~(1 << ONEWIRE_BIT);
}

uint8_t onewire_reset() {
  /* reset OneWire connection */
  uint8_t data = 1;

  // low release
  onewire_pin_low();
  _delay_us(480);
  onewire_pin_release();
  _delay_us(70);

  data = onewire_pin_read();
  _delay_us(410);
  return data; // 0 = device present
}

void onewire_write(uint8_t data) {
  for (int i = 0; i<8; i++) {
    if ((data & 1) == 1) {
      // low release
      onewire_pin_low();
      _delay_us(6);
      onewire_pin_release();
      _delay_us(64);
    }
    else {
      // low release
      onewire_pin_low();
      _delay_us(60);
      onewire_pin_release();
      _delay_us(10);
    }

    data = data >> 1;
  }
}

uint8_t onewire_read() {
  uint8_t data = 0;
  for (int i = 0; i<8; i++) {
    // low release
    onewire_pin_low();
    _delay_us(6);
    onewire_pin_release();
    _delay_us(9);

    data = data | onewire_pin_read()<<i;
    _delay_us(55);
  }
  return data;
}

void onewire_read_bytes(int bytes) {
  for (int i=0; i<bytes; i++) {
    DataBytes[i] = onewire_read();
  }
}

uint8_t onewire_crc(int bytes) {
  uint8_t crc = 0;
  for (int j=0; j<bytes; j++) {
    crc = crc ^ DataBytes[j];
    for (int i=0; i<8; i++) crc = crc>>1 ^ ((crc & 1) ? 0x8c : 0);
  }
  return crc;
}

void onewire_skip_rom() {
  onewire_write(ONEWIRE_SKIP_ROM);
}

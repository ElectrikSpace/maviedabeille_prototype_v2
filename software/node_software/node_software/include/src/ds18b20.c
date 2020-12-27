/*
* onewire.h
*
* Created: 30/10/2020 21:54:47
*  Author: sylvain
*/

#include <avr/io.h>
#include <util/delay.h>
#include <avr/interrupt.h>
#include "onewire.h"

uint8_t ds18b20_setup(uint8_t resolution) {
  /* setup DS18B20 configuration register with the correct resolution */
  uint8_t success;
  uint8_t configuration_register;
  cli();
  onewire_init();
  if (onewire_reset() != 0) {
    onewire_skip_rom();
    // prepare configuration register value
    switch (resolution) {
      case 9 :
        configuration_register = 0;
        break;
      case 10 :
        configuration_register = (1<<5);
        break;
      case 11 :
        configuration_register = (1<<6);
        break;
      case 12 :
        configuration_register = (1<<5) | (1<<5);
        break;
      default :
        success = 0;
        break;
    }
    if (success) {
      // write in DS18B20 configuration register
      onewire_write(DS18B20_WRITE_SCRATCHPAD);
      onewire_write(0);
      onewire_write(0);
      onewire_write(configuration_register);
      _delay_ms(1);
      onewire_reset();
      onewire_skip_rom();
      onewire_write(DS18B20_READ_SCRATCHPAD);
      onewire_read_bytes(9);
      if (onewire_crc(9) == 0 && DataBytes[4] == configuration_register) {
        success = 1;
      }
      else {
        success = 0;
      }
    }
  }
  else {
    success = 0;
  }
  sei();
  return success;
}

uint8_t ds18b20_start_conversion() {
  /* send a conversion command */
  uint8_t success;
  cli();
  onewire_init();
  if (onewire_reset() != 0) {
    onewire_skip_rom();
    onewire_write(DS18B20_CONVERT);
    success = 1;
  }
  else {
    success = 0;
  }
  sei();
  return success;
}

uint8_t ds18b20_read_temperature(uint16_t *temperature) {
  /* read the content of data registers */
  cli();
  onewire_init();
  if (onewire_reset() != 0) {
    onewire_skip_rom();
    onewire_write(DS18B20_READ_SCRATCHPAD);
    onewire_read(9);
    if (onewire_crc(9) == 0) {
      *temperature = DataBytes[0] | (DataBytes[1] << 8);
	  sei();
	  return 1;
    }
  }
  sei();
  return 0;
}

uint8_t ds18b20_get_temperature(uint16_t *temperature) {
  /* complete a full cycle of temperature conversion and data register read */
  if (ds18b20_start_conversion()) {
    cli();
    while (onewire_read() != 0xff);
    if (ds18b20_read_temperature(temperature)) {
      sei();
	  return 1;
    }
  }
  sei();
  return 0;
}
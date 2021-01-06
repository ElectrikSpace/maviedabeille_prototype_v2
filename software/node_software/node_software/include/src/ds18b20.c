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

uint8_t ds18b20_setup(uint8_t binary_resolution) {
  /* setup DS18B20 configuration register with the correct resolution */
  uint8_t success;
  uint8_t configuration_register;
  cli();
  onewire_init();
  if (onewire_reset()) {
    onewire_skip_rom();
    // prepare configuration register value
    switch (binary_resolution) {
      case 9 :
        configuration_register = 0;
		success = 1;
        break;
      case 10 :
        configuration_register = (1<<5);
		success = 1;
        break;
      case 11 :
        configuration_register = (1<<6);
		success = 1;
        break;
      case 12 :
        configuration_register = (1<<5) | (1<<5);
		success = 1;
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

uint8_t ds18b20_read_temperature(float *temperature) {
  /* read the content of data registers */
  cli();
  onewire_init();
  if (onewire_reset() != 0) {
    onewire_skip_rom();
    onewire_write(DS18B20_READ_SCRATCHPAD);
    onewire_read(9);
    if (onewire_crc(9) == 0) {
      // old with uint16_t : *temperature = DataBytes[0] | (DataBytes[1] << 8);
	  *temperature = (float) ((int16_t) ((DataBytes[1] & 0xf0) << 8) | (DataBytes[1] << 4) | (DataBytes[0] >> 4)); // entire part
	  *temperature = *temperature + (float) ((uint8_t) (DataBytes[0] & 0x0f))*0.0625; // add the decimal part
	  sei();
	  return 1;
    }
  }
  sei();
  return 0;
}

uint8_t ds18b20_get_temperature(float *temperature) {
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

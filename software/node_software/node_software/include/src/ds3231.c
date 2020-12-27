/*
* ds3231.c
*
* Created: 30/10/2020 21:54:47
*  Author: sylvain
*/

#include <avr/io.h>
#include <util/delay.h>
#include "i2c_master.h"

uint8_t ds3231_write_register(uint8_t register_address, uint8_t byte) {
	/* write 1 register in ds3231 register located by the address */
	uint8_t data[3] = {((DS3231_ADDRESS << 1) | 0x01), register_address, byte};
	i2c_start_write(data, 3); // write byte
	return i2c_get_status();
}

uint8_t ds3231_read_register(uint8_t register_address, uint8_t *value) {
	/* read 1 register from ds3231 and put the content in buffer */

	// set the ds3231 register pointer to the right address
	uint8_t data[2] = {(DS3231_ADDRESS << 1), register_address}; // adress+w, control register pointer adress
	i2c_start_write(data, 2); // set ds3231 register pointer to control register

	if (i2c_get_status()) {
		// read bytes
		data[0] = (DS3231_ADDRESS << 1) | 0x01;
		i2c_start_write(data, 2); // read one byte
		return i2c_get_status();
	}
	else {
		return 0;
	}
}

uint8_t ds3231_enable_alarm(uint8_t alarm) {
	/* *enable the selected alarm, clear flag, does not enable interrupt pin function */
	uint8_t status_register;
	uint8_t control_register;

	// check if selected alarm exists
	if (alarm == 1 || alarm == 2) {
		alarm = alarm - 1;
	}
	else {
		return 0;
	}

	// read status register
	if (ds3231_read_register(0x0f, &status_register)) {
		status_register = status_register & ~(1 << alarm);
	}
	else {
		return 0;
	}
	// write flag cleared status register
	if (ds3231_write_register(0x0f, status_register)) {
	}
	else {
		return 0;
	}

	// read control register and rewrite with selected alarm enable
	if (ds3231_read_register(0x0e, &control_register)) {
		control_register = control_register | (1 << alarm);
	}
	else {
		return 0;
	}

	// write control register with selected alarm enabled
	return ds3231_write_register(0x0e, control_register);
}

uint8_t ds3231_disable_alarm(uint8_t alarm) {
	/* disable the select alarm */
	uint8_t control_register;

	// check if selected alarm exists
	if (alarm == 1 || alarm == 2) {
		alarm = alarm - 1;
	}
	else {
		return 0;
	}

	// read control register
	if (ds3231_read_register(0x0e, &control_register)) {
		control_register = control_register & ~(1 << alarm);
	}
	else {
		return 0;
	}

	// write control register with selected alarm disabled
	return ds3231_write_register(0x0e, control_register);
}

uint8_t ds3231_reset() {
  /* enable oscillator, disable alarms and interrupt pin */
  uint8_t data[3] = {(DS3231_ADDRESS << 1), 0x0e, 0x00}; // adress+w, control register pointer adress, new control register value
  i2c_start_write(data, 3);
  _delay_ms(250); // complete reset time
  return i2c_get_status();
}

uint8_t ds3231_is_running() {
  /* return 1 of ds3231 is working with oscillator started, 0 if oscillator stopped or error */
  uint8_t status_register;

  if (ds3231_read_register(0x0f, &status_register)) {
    status_register = status_register | (1 << 3);
    if (ds3231_write_register(0x0f, status_register)) {
      if ( !(status_register & (1 << 7)) ) { // if oscillator stop flag is not set
        return 1;
      }
    }
  }
  return 0;
}

uint8_t ds3231_enable_32kHz_pin() {
  uint8_t status_register;

  if (ds3231_read_register(0x0f, &status_register)) {
    status_register = status_register | (1 << 3);
    return ds3231_write_register(0x0f, status_register);
  }
  else {
    return 0;
  }
}

uint8_t ds3231_disable_32kHz_pin() {
  uint8_t status_register;

  if (ds3231_read_register(0x0f, &status_register)) {
    status_register = status_register & ~(1 << 3);
    return ds3231_write_register(0x0f, status_register);
  }
  else {
    return 0;
  }
}

uint8_t ds3231_enable_interrupt_pin() {
  /* also disable square wave pin */
  uint8_t control_register;

  if (ds3231_read_register(0x0e, &control_register)) {
    control_register = control_register | (1 << 2);
    return ds3231_write_register(0x0e, control_register);
  }
  else {
    return 0;
  }
}

uint8_t ds3231_disable_interrupt_pin() {
  /* also enable square wave pin */
  uint8_t control_register;

  if (ds3231_read_register(0x0e, &control_register)) {
    control_register = control_register & ~(1 << 2);
    return ds3231_write_register(0x0e, control_register);
  }
  else {
    return 0;
  }
}

uint8_t ds3231_set_square_wave_pin(uint16_t frequency) {
  /* frequency in Hz must be one of these : 1 Hz / 1024 Hz / 4096 Hz / 8192 Hz
     also enable square wave pin and disable interrupt pin function */
 uint8_t control_register;
 uint8_t control_register_mask;

 switch (frequency) {
   case 1 :
      control_register_mask = 0x00;
      break;
   case 1024 :
      control_register_mask = (1 << 3);
      break;
   case 4096 :
      control_register_mask = (1 << 4);
      break;
   case 8192 :
      control_register_mask = (1 << 3) | (1 << 4);
      break;
   default :
      return 0;
 }

 if (ds3231_read_register(0x0e, &control_register)) {
   control_register = control_register & ~( (1 << 3) | (1 << 4) );
   control_register = control_register | control_register_mask;
   return ds3231_write_register(0x0e, control_register);
 }
 else {
   return 0;
 }
}

uint8_t ds3231_set_datetime(uint8_t seconds, uint8_t minutes, uint8_t hours, uint8_t day_of_week, uint8_t day_of_month, uint8_t month, uint16_t year) {
  /* set date in the ds3231 internal memory */
  uint8_t data[7];

  // seconds
  if (seconds >= 0 && seconds <= 59) {
    data[0] = ((seconds / 10) << 4) | (seconds % 10);
  }
  else {
    return 0;
  }
  // minutes
  if (minutes >= 0 && minutes <= 59) {
    data[1] = ((minutes / 10) << 4) | (minutes % 10);
  }
  else {
    return 0;
  }
  // hours
  if (hours >= 0 && hours <= 23) {
    data[2] = (hours / 20 << 6) | ( ((hours % 20) / 10) << 5) | ((hours % 20) % 10);
  }
  else {
    return 0;
  }
  // day of week
  if (day_of_week >= 1 && day_of_week <= 7) {
    data[3] = day_of_week;
  }
  else {
    return 0;
  }
  // day of month
  if (day_of_month >= 1 && day_of_month <= 31) {
    data[4] = ((day_of_month / 10) << 4) | (day_of_month % 10);
  }
  else {
    return 0;
  }
  // month
  if (month >= 1 && month <= 12) {
    data[5] = ((month / 10) << 4) | (month % 10);
  }
  else {
    return 0;
  }
  // year
  if (year >= 0 && year <= 99) {
    data[6] = ((year / 10) << 4) | (year % 10);
  }
  else {
    return 0;
  }

  // write ds3231 registers
  return ( ds3231_write_register(0x00, data[0]) &&
           ds3231_write_register(0x01, data[1]) &&
           ds3231_write_register(0x02, data[2]) &&
           ds3231_write_register(0x03, data[3]) &&
           ds3231_write_register(0x04, data[4]) &&
           ds3231_write_register(0x05, data[5]) &&
           ds3231_write_register(0x06, data[6])
         );
}

uint8_t ds3231_get_datetime(uint8_t *seconds, uint8_t *minutes, uint8_t *hours, uint8_t *day_of_week, uint8_t *day_of_month, uint8_t *month, uint16_t *year) {
  /* assuming 21st century */
  uint8_t data[7];

  if ( ds3231_read_register(0x00, data) &&
       ds3231_read_register(0x01, data + 1) &&
       ds3231_read_register(0x02, data + 2) &&
       ds3231_read_register(0x03, data + 3) &&
       ds3231_read_register(0x04, data + 4) &&
       ds3231_read_register(0x05, data + 5) &&
       ds3231_read_register(0x06, data + 6)
     ) {
        *seconds = (data[0] >> 4)*10 + (data[0] & 0x0f);
        *minutes = (data[1] >> 4)*10 + (data[1] & 0x0f);
        *hours = ((data[2] & (1 << 5)) >> 5)*20 + ((data[2] & (1 << 4)) >> 4)*10 + (data[2] & 0x0f);
        *day_of_week = data[3];
        *day_of_month = (data[4] >> 4)*10 + (data[4] & 0x0f);
        *month = ((data[5] & (1 << 4)) >> 4)*10 + (data[5] & 0x0f);
        *year = 2000 + (data[6] >> 4)*10 + (data[6] & 0x0f);
        return 1;
  }
  else {
    return 0;
  }
}

uint8_t ds3231_get_time(uint8_t *seconds, uint8_t *minutes, uint8_t *hours) {
  uint8_t data[3];

  if ( ds3231_read_register(0x00, data) &&
       ds3231_read_register(0x01, data + 1) &&
       ds3231_read_register(0x02, data + 2)
     ) {
        *seconds = (data[0] >> 4)*10 + (data[0] & 0x0f);
        *minutes = (data[1] >> 4)*10 + (data[1] & 0x0f);
        *hours = ((data[2] & (1 << 5)) >> 5)*20 + ((data[2] & (1 << 4)) >> 4)*10 + (data[2] & 0x0f);
        return 1;
  }
  else {
    return 0;
  }
}

uint8_t ds3231_get_date(uint8_t *day_of_week, uint8_t *day_of_month, uint8_t *month, uint16_t *year) {
  /* assuming 21st century */
  uint8_t data[4];

  if ( ds3231_read_register(0x03, data) &&
       ds3231_read_register(0x04, data + 1) &&
       ds3231_read_register(0x05, data + 2) &&
       ds3231_read_register(0x06, data + 3)
     ) {
        *day_of_week = data[3];
        *day_of_month = (data[4] >> 4)*10 + (data[4] & 0x0f);
        *month = ((data[5] & (1 << 4)) >> 4)*10 + (data[5] & 0x0f);
        *year = 2000 + (data[6] >> 4)*10 + (data[6] & 0x0f);
        return 1;
  }
  else {
    return 0;
  }
}

uint8_t ds3231_set_alarm_1(uint8_t seconds, uint8_t minutes, uint8_t hours, uint8_t day, uint8_t alarm_mode_bits, uint8_t is_day_of_the_week) {
  /* day is day of the week or day of the month depending on the alarm_mode_bits (please use predefined values) */
  uint8_t data[4];

  // seconds
  if (seconds >= 0 && seconds <= 59) {
    data[0] = ((seconds / 10) << 4) | (seconds % 10);
  }
  else {
    return 0;
  }
  // minutes
  if (minutes >= 0 && minutes <= 59) {
    data[1] = ((minutes / 10) << 4) | (minutes % 10);
  }
  else {
    return 0;
  }
  // hours
  if (hours >= 0 && hours <= 23) {
    data[2] = (hours / 20 << 6) | ( ((hours % 20) / 10) << 5) | ((hours % 20) % 10);
  }
  else {
    return 0;
  }
  if (is_day_of_the_week) {
	  // day of week
	  if (day >= 1 && day <= 7) {
		  data[3] = day;
	  }
	  else {
		  return 0;
	  }
  }
  else {
	  // day of month
	  if (day >= 1 && day <= 31) {
		  data[3] = ((day / 10) << 4) | (day % 10);
	  }
	  else {
		  return 0;
	  }
  }

  // alarm mode processing
  data[0] = data[0] | ( ( alarm_mode_bits & (1 << 0) ) << 7 );
  data[1] = data[1] | ( ( alarm_mode_bits & (1 << 1) ) << 6 );
  data[2] = data[2] | ( ( alarm_mode_bits & (1 << 2) ) << 5 );
  data[3] = data[3] | ( ( alarm_mode_bits & (1 << 3) ) << 4 );
  data[3] = data[3] | ( ( alarm_mode_bits & (1 << 4) ) << 2 );

  // write ds3231 registers
  return ( ds3231_write_register(0x07, data[0]) &&
           ds3231_write_register(0x08, data[1]) &&
           ds3231_write_register(0x09, data[2]) &&
           ds3231_write_register(0x0a, data[3])
         );
}

uint8_t ds3231_enable_alarm_1() {
  /* also clear flag, does not enable interrupt pin function */
  return ds3231_enable_alarm(1);
}

uint8_t ds3231_disable_alarm_1() {
  /* only disable alarm */
  return ds3231_disable_alarm(1);
}

uint8_t ds3231_set_alarm_2(uint8_t minutes, uint8_t hours, uint8_t day, uint8_t alarm_mode_bits, uint8_t is_day_of_the_week) {
  /* day is day of the week or day of the month depending on the alarm_mode_bits (please use predefined values) */
  uint8_t data[3];

  // minutes
  if (minutes >= 0 && minutes <= 59) {
    data[0] = ((minutes / 10) << 4) | (minutes % 10);
  }
  else {
    return 0;
  }
  // hours
  if (hours >= 0 && hours <= 23) {
    data[1] = (hours / 20 << 6) | ( ((hours % 20) / 10) << 5) | ((hours % 20) % 10);
  }
  else {
    return 0;
  }
  if (is_day_of_the_week) {
	  // day of week
	  if (day >= 1 && day <= 7) {
		  data[2] = day;
	  }
	  else {
		  return 0;
	  }
  }
  else {
	   // day of month
	   if (day >= 1 && day <= 31) {
		   data[2] = ((day / 10) << 4) | (day % 10);
	   }
	   else {
		   return 0;
	   }
  }

  // alarm mode processing
  data[0] = data[0] | ( ( alarm_mode_bits & (1 << 0) ) << 7 );
  data[1] = data[1] | ( ( alarm_mode_bits & (1 << 1) ) << 6 );
  data[2] = data[2] | ( ( alarm_mode_bits & (1 << 2) ) << 5 );
  data[2] = data[2] | ( ( alarm_mode_bits & (1 << 3) ) << 3 );

  // write ds3231 registers
  return ( ds3231_write_register(0x0b, data[0]) &&
           ds3231_write_register(0x0c, data[1]) &&
           ds3231_write_register(0x0d, data[2])
         );
}

uint8_t ds3231_enable_alarm_2() {
  /* also clear flag, does not enable interrupt pin function */
  return ds3231_enable_alarm(2);
}

uint8_t ds3231_disable_alarm_2() {
  /* only disable alarm */
  return ds3231_disable_alarm(2);
}

uint8_t ds3231_clear_alarm_flags(uint8_t *cleared_alarms) {
  /* cleared_alarms contains :
        - 0 if no alarm had a flag
        - 1 if only alarm 1 had a flag
        - 2 if alarm 2 had a flag
        - 3 if both alarm 1 and alarm 2 had a flag
  */
  uint8_t status_register;

  // read status register
  if (ds3231_read_register(0x0f, &status_register)) {
  }
  else {
    return 0;
  }

  // process status register content
  *cleared_alarms = 0;
  if (status_register & (1 << 0)) {
    *cleared_alarms = *cleared_alarms + 1;
  }
  if (status_register & (1 << 1)) {
    *cleared_alarms = *cleared_alarms + 2;
  }
  status_register = status_register & ~((1 << 0) | (1 << 1));

  // write status register with alarm flags cleared
  return (ds3231_write_register(0x0f, status_register));
}

uint8_t ds3231_get_temperature(float *temperature) {
  /* return the temperature measured by the ds3231 chip */
  // get the current control register
  uint8_t control_register;
  uint8_t temperature_register[2];

  if (ds3231_read_register(0x0e, &control_register)) {
    control_register = control_register | (1<<5); // initiate a temperature conversion
    if (ds3231_write_register(0x0e, control_register)) {
      _delay_ms(201); // max temperature conversion time + transmission
      if (ds3231_read_register(0x11, temperature_register) && ds3231_read_register(0x12, temperature_register+1)) {
        *temperature = (float) temperature_register[0]; // int part of temperature
        *temperature = *temperature + (float) ( (temperature_register[1]*25 ) / 100); // decimal part of temperature
        return 1;
      }
    }
  }
  return 0;
}

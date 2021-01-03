/*
 * node_software.c
 *
 * Created: 26/10/2020 17:48:33
 * Author : sylvain
 */ 

#define F_CPU 8000000UL // core frequency, must be lower or equal than 8 MHz
#define SIGFOX_TRANSMIT_REPEATS 1 // message repeats when transmitting data, must be between 0 and 2

#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/sleep.h>
#include <util/delay.h>
#include <string.h>

#include "power.h"
#include "hx711.h"
#include "onewire.h"
#include "simple_uart.h"
#include "i2c_master.h"
#include "ds3231.h"
#include "ds18b20.h"
#include "sht3x.h"
#include "software_uart.h"
#include "sfm10r1.h"

// not used yet
/*#include "spi_master.h"
#include "at24c32.h"*/

void serial_test(char test) {
	/* test function only for debug purpose */
	char data[4] = {test, '\r', '\n', '\0'};
	serial_send(data);
}

ISR (INT0_vect) // wake up interrupt
{
	// nothing to do here, but executed after each wake up
}

void enter_sleep() {
	/* enter in sleep with power down mode */
	EIMSK = EIMSK | (1 << INT0);
	set_sleep_mode(SLEEP_MODE_PWR_DOWN);
	sei();
	sleep_mode();
	cli();
	EIMSK = EIMSK & ~(1 << INT0);
}

void enter_error_state(uint8_t state) {
	/* error state */

	// must switch ON peripherals to use LED
	power_on_external_peripherals();

	// error during launch sequence
	if (state == 1) {
		power_blink_state_led(250, 250, 60000);
		power_off_external_peripherals();
		enter_sleep();
	}

	// error during the test cycle
	else if (state == 2) {
		power_blink_state_led(1000, 1000, 60000);
		power_off_external_peripherals();
		enter_sleep();
	}
}

void init() {
	/* init sequence */
	uint8_t cleared_ds3231_alarms;

	/* disable global interrupts */
	cli();
	
	/* setup wake up interrupt setting */
	EICRA = 0x00;

	/* prepare peripherals for the launch sequence */
	power_on_external_peripherals();

	/* power ON state led */
	power_on_state_led();

	/* setup clock system */
	if (!power_setup_clock()) { // if failure
		enter_error_state(1);
	}
	

	/* check battery level */
	if (!power_battery_startup_check()) { // if failure
		enter_error_state(1);
	}
	

	/* setup DS3231 */
	// start i2c bus
	i2c_init();
	// reset ds3231
	if (!ds3231_reset()) { // if failure
		enter_error_state(1);
	}
	// disable 32kHz pin
	if (!ds3231_disable_32kHz_pin()) { // if failure
		enter_error_state(1);
	}
	// set default date time
	if (!ds3231_set_datetime(0, 0, 0, 7, 3, 1, 21)) { // if failure
		enter_error_state(1);
	}
	// setup alarm 1
	if (!ds3231_set_alarm_1(0, 0, 0, 1, DS3231_ALARM_1_MATCH_SECONDS_MINUTES, 0)) { // if failure
		enter_error_state(1);
	}
	// enable alarm 1 and enable the use of the interrupt pin for alarm 1
	if (!ds3231_enable_alarm_1()) { // if failure
		enter_error_state(1);
	}
	// disable alarm 2 and disable the use of the interrupt pin for alarm 2
	if (!ds3231_disable_alarm_2()) { // if failure
		enter_error_state(1);
	}
	// enable alarm interrupt function on INT pin of the DS3231 chip
	if (!ds3231_enable_interrupt_pin()) {
		enter_error_state(1);
	}
	// enable interrupt pin of the chip and clear current alarms flags
	if (!ds3231_enable_interrupt_pin() || !ds3231_clear_alarm_flags(&cleared_ds3231_alarms)) { // if failure
		enter_error_state(1);
	}
	i2c_end(); // stop i2c bus
	
	/* setup SFM10R1 */
	sfm10r1_init(); // init communication
	if (!sfm10r1_ping()) {
		enter_error_state(1);
	}
	if (!sfm10r1_set_transmit_repeats(SIGFOX_TRANSMIT_REPEATS)) {
		enter_error_state(1);
	}
	sfm10r1_end(); // stop communication

	// power off state led, launch sequence is finished
	power_off_state_led();
}

void measure_cycle(uint8_t is_test_cycle) {
	/* collect the measurements from each sensor and send it other Sigfox */
	uint8_t counter;
	uint8_t counter2;
	uint8_t message[12];
	char message_ascii[24];
	float battery_voltage;
	uint8_t battery_voltage_processed[2];
	float solar_intensity;
	uint8_t solar_intensity_processed;
	uint32_t raw_weight;
	float sht_humidity;
	uint8_t sht_humidity_processed;
	float sht_temperature;
	uint8_t sht_temperature_processed[2];
	float ds18b20_temperature;
	uint8_t ds18b20_temperature_processed[2];
	float sfm10r1_temperature;
	uint8_t sfm10r1_temperature_processed[2];

	// power state led for 100ms
	power_on_state_led();
	_delay_ms(100);
	power_off_state_led();

	/* collect measurements */

	// battery voltage
	battery_voltage = power_get_battery_voltage();

	// solar intensity
	solar_intensity = power_get_solar_intensity();

	// raw weight
	HX711_init(); // init hx711 chip
	while(!HX711_is_ready()); // wait until the chip is ready
	raw_weight = HX711_multiple_conversion_average(0, 10); // average of 10 conversions using channel A with gain = 128

	// SHT3x humidity and temperature
	i2c_init();
	_delay_ms(1); // wait until the chip is ready
	sht3x_reset(); // reset the chip
	if (!sht3x_get_measurement(0x03, &sht_humidity, &sht_temperature)) { // collect temperature and humidity
		// server backend will detect a failure if temperature and humidity have unbelievable values
		sht_humidity = 101;
		sht_temperature = 101;
	}
	i2c_end();

	// DS18B20 temperature
	if (ds18b20_setup(10)) { // setup the chip with the desired resolution
		if (!ds18b20_get_temperature(&ds18b20_temperature)) { // collect the temperature
			// server backend will detect a failure if temperature has an unbelievable value
			ds18b20_temperature = 101;
		}
	}
	else {
		// server backend will detect a failure if temperature has an unbelievable value
		ds18b20_temperature = 101;
	}

	// SFM10R1 temperature
	sfm10r1_init();
	counter = 0;
	while (counter < 3 && !sfm10r1_ping()) {
		counter++;
	}
	if (counter == 3) {
		if (is_test_cycle) {
			enter_error_state(2);
		}
		else {
			return; // unexpected error, cannot do anything
		}
	}
	if (!sfm10r1_get_temperature(&sfm10r1_temperature)) {
		sfm10r1_temperature = 101;
	}

	/* process data */
	// put a 100°C offset for each temperature value
	// and put data into a 12 bits format for each temperature value
	sht_temperature = sht_temperature + 100;
	sht_temperature_processed[0] = (uint8_t) (int) sht_temperature;
	sht_temperature = sht_temperature - (float) sht_temperature_processed[0];
	sht_temperature_processed[1] = (uint8_t) ((int) ((float) 1000)*sht_temperature) / 625;

	ds18b20_temperature = ds18b20_temperature + 100;
	ds18b20_temperature_processed[0] = (uint8_t) (int) ds18b20_temperature;
	ds18b20_temperature = ds18b20_temperature - (float) ds18b20_temperature_processed[0];
	ds18b20_temperature_processed[1] = (uint8_t) ((int) ((float) 1000)*ds18b20_temperature) / 625;

	sfm10r1_temperature = sfm10r1_temperature + 100;
	sfm10r1_temperature_processed[0] = (uint8_t) (int) sfm10r1_temperature;
	sfm10r1_temperature = sfm10r1_temperature - (float) sfm10r1_temperature_processed[0];
	sfm10r1_temperature_processed[1] = (uint8_t) ((int) ((float) 1000)*sfm10r1_temperature) / 625;

	// humidity is int between 0 and 99
	sht_humidity_processed = (uint8_t) (int) sht_humidity;

	// battery voltage
	if (battery_voltage < 2.5) {
		battery_voltage = 2.5;
	}
	else if (battery_voltage > 4.5) {
		battery_voltage = 4.5;
	}
	else {
		battery_voltage = battery_voltage - 2.5;
	}
	battery_voltage_processed[0] = (((uint16_t) ((float) 512*battery_voltage)) >> 2);
	battery_voltage_processed[1] = (((uint16_t) ((float) 512*battery_voltage)) & ((1 << 0) | (1 << 1)));

	// solar intensity
	solar_intensity_processed = (uint8_t) (int) (float) 19*solar_intensity;

	/* create the 12 bytes Sigfox message */
	message[0] = 0x01; // header code for this kind of message
	message[1] = battery_voltage_processed[0]; // battery voltage
	message[2] = (battery_voltage_processed[1] << 6) | solar_intensity_processed; // battery voltage + solar  intensity
	for (counter = 3; counter < 7; counter++) { // weight
		message[counter] = (uint8_t) raw_weight;
		raw_weight = (raw_weight >> 8);
	}
	message[7] = sht_humidity_processed; // humidity
	message[8] = sht_temperature_processed[0]; // SHT temperature
	message[9] = ds18b20_temperature_processed[0]; // DS18B20 temperature
	message[10] = sfm10r1_temperature_processed[0]; // SFM10R1 temperature
	message[11] = (sht_temperature_processed[1] << 4) | (ds18b20_temperature_processed[1] << 2) | sfm10r1_temperature_processed[1]; // SHT + DS18B20 + SFM10R1 temperatures

	/* convert message from HEX to ASCII HEX (0x1A -> '1' and 'A')*/
	for (counter = 0; counter < 12; counter++) {
		for (counter2 = 0; counter2 < 2; counter2++) {
			switch ( (message[counter] >> (4*(1-counter2))) & 0x0f ) {
				case 0x00:
					message_ascii[2*counter + counter2] = '0';
					break;
				case 0x01:
					message_ascii[2*counter + counter2] = '1';
					break;
				case 0x02:
					message_ascii[2*counter + counter2] = '2';
					break;
				case 0x03:
					message_ascii[2*counter + counter2] = '3';
					break;
				case 0x04:
					message_ascii[2*counter + counter2] = '4';
					break;
				case 0x05:
					message_ascii[2*counter + counter2] = '5';
					break;
				case 0x06:
					message_ascii[2*counter + counter2] = '6';
					break;
				case 0x07:
					message_ascii[2*counter + counter2] = '7';
					break;
				case 0x08:
					message_ascii[2*counter + counter2] = '8';
					break;
				case 0x09:
					message_ascii[2*counter + counter2] = '9';
					break;
				case 0x0a:
					message_ascii[2*counter + counter2] = 'A';
					break;
				case 0x0b:
					message_ascii[2*counter + counter2] = 'B';
					break;
				case 0x0c:
					message_ascii[2*counter + counter2] = 'C';
					break;
				case 0x0d:
					message_ascii[2*counter + counter2] = 'D';
					break;
				case 0x0e:
					message_ascii[2*counter + counter2] = 'E';
					break;
				case 0x0f:
					message_ascii[2*counter + counter2] = 'F';
				break;
				default :
					// impossible state
					break;
			}
		}
	}

	/* send the 12 bytes Sigfox message */
	if (!sfm10r1_send_data(message_ascii, 24)) {
		if (is_test_cycle) {
			enter_error_state(2);
		}
		else {
			return;
		}
	}
	sfm10r1_end();
}

void clear_hourly_flags() {
	/* clear RTC alarm flags and MCU wake up interrupt flag */
	uint8_t cleared_ds3231_alarms;

	// RTC alarm flags
	i2c_init(); // start i2c bus

	ds3231_clear_alarm_flags(&cleared_ds3231_alarms); // clear flags

	i2c_end(); // stop i2c bus
}

int main(void)
{
	
	// test
	serial_init(9600);
	char data[8] = {'S', 'T', 'A', 'R', 'T', '\r', '\n', '\0'};
	serial_send(data);
	
	init(); // launch and setup sequence
	
	measure_cycle(1); // do a test cycle and send data

	while(1) {
		// switch ON peripherals
		power_on_external_peripherals();
		
		// clear hourly flags
		clear_hourly_flags();
		
		// measure cycle and data transmit
		measure_cycle(0);
		
		// switch OFF peripherals before entering in sleep
		power_off_external_peripherals();
		
		// enter in sleep mode, will wake up by RTC alarm interrupt
		enter_sleep();
	}
}


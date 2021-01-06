/*
 * hx711.c
 *
 * Created: 23/04/2020 23:15:18
 *  Author: sylvain
 */

#include <avr/io.h>
#include <util/delay.h>

void hx711_init(){
	// set up HX711 to make it ready to use

	// setup PD_SCK pin to output at low level
	PD_SCK_DDR = PD_SCK_DDR | (1 << PD_SCK_BIT);
	PD_SCK_PORT = PD_SCK_PORT & ~(1 << PD_SCK_BIT);

	// setup DOUT pin to input at Hi-Z
	DOUT_DDR = DOUT_DDR & ~(1 << DOUT_PIN);
	DOUT_PORT = DOUT_PORT & ~(1 << DOUT_PIN);
	
	// wait for chip startup
	_delay_ms(1);
}

void hx711_end(){
	/* release IOs pins */

	// put PD_SCK in Hi-Z
	PD_SCK_DDR = PD_SCK_DDR & ~(1 << PD_SCK_BIT);
	PD_SCK_PORT = PD_SCK_PORT & ~(1 << PD_SCK_BIT);

	// put DOUT in Hi-Z
	DOUT_DDR = DOUT_DDR & ~(1 << DOUT_PIN);
	DOUT_PORT = DOUT_PORT & ~(1 << DOUT_PIN);
}

uint8_t hx711_wait_until_ready(uint16_t timeout) {
	/* return 1 if it becomes ready before timeout, 0 if not
	   timeout must be given in ms */
	uint8_t is_ready = 0;
	uint16_t time_counter = 0;
	
	// check every ms until the chip is ready or timeout
	while (!is_ready && (time_counter < timeout)) {
		if (!(DOUT_PIN & (1 << DOUT_BIT))) {
			is_ready = 1;
		}
		time_counter++;
		_delay_ms(1);
	}
	
	return is_ready;
}

uint8_t hx711_change_mode(uint8_t conversion_mode){
	// change channel and gain for next data sending

	// --> mode = 0 : channel A and gain 128
	// --> mode = 1 : channel B and gain 32
	// --> mode = 2 : channel B and gain 64

	// default mode after power up or sleep is channel A and gain 128
	uint8_t counter;
	uint8_t pulses = 25 + conversion_mode;

	// wait until the chip is ready
	if (!hx711_wait_until_ready(1000)) {
		return 0;
	}
		
	// wait for "DOUT falling edge to PD_SCK rising edge"
	_delay_us(10);

	// send the correct number of SCK pulses to change next conversion mode
	for(counter = 0; counter < pulses; counter++){
		// SCK high level
		PD_SCK_PORT = PD_SCK_PORT | (1 << PD_SCK_BIT);

		_delay_us(10); // SCK high time

		// SCK low level
		PD_SCK_PORT = PD_SCK_PORT & ~(1 << PD_SCK_BIT);

		_delay_us(10); // SCK low time
	}
	
	// success
	return 1;

}

uint8_t hx711_read_stored_conversion(uint8_t next_conversion_mode, uint8_t *output_data){
	// read the stored conversion

	if (!hx711_wait_until_ready(1000)) {
		return 0;
	}

	// initialize buffer
	uint32_t buffer = 0;

	// read data
	for(int i = 0; i < 24; i++){ // for each 24 bits of data
		// SCK high level
		PD_SCK_PORT = PD_SCK_PORT | (1 << PD_SCK_BIT);

		_delay_us(10); // SCK high time

		//store data
		buffer = buffer << 1; 
		buffer = buffer | ((DOUT_PIN & (1 << DOUT_BIT)) >> DOUT_BIT);

		// SCK low level
		PD_SCK_PORT = PD_SCK_PORT & ~(1 << PD_SCK_BIT);

		_delay_us(10); // SCK low time
	}

	// configure the next conversion
	for(uint8_t i = 0; i <= next_conversion_mode; i++){
		// SCK high level
		PD_SCK_PORT = PD_SCK_PORT | (1 << PD_SCK_BIT);

		_delay_us(10); // SCK high time

		// SCK low level
		PD_SCK_PORT = PD_SCK_PORT & ~(1 << PD_SCK_BIT);

		_delay_us(10); // SCK low time
	}

	// save the content of the buffer into output_data
	output_data[0] = (uint8_t) (buffer >> 16);
	output_data[1] = (uint8_t) (buffer >> 8);
	output_data[2] = (uint8_t) buffer;

	return 1;
}

uint8_t hx711_read_new_conversion(uint8_t conversion_mode, uint8_t next_conversion_mode, uint8_t *output_data){
	// start a conversion and read data by configuring the next conversion mode

	// configuring and start a new conversion
	hx711_change_mode(conversion_mode);

	// read data and configure the next configure at the same time
	return (hx711_read_stored_conversion(next_conversion_mode, output_data));
}

uint8_t hx711_multiple_conversion_average(uint8_t conversion_mode, uint8_t number_samples, uint8_t *output_data){
	// return the average of wanted samples
	int32_t data_buffer_value = 0;
	uint8_t data_buffer[3];
	uint8_t msbs;

	// initialize the data variable with a first conversion
	hx711_change_mode(conversion_mode);
	
	// add directly the wanted samples, mode was configured before
	for(uint8_t i = 0; i < number_samples; i++){
		if (!hx711_read_stored_conversion(conversion_mode, data_buffer)) {
			return 0;
		}
		if (!hx711_wait_until_ready(1000)) {
			return 0;
		}
		
		if (data_buffer[0] >> 7) {
			msbs = 0xff;
		}
		else {
			msbs = 0x00;
		}
		data_buffer_value = data_buffer_value + ((int32_t) (uint32_t) ((uint32_t) msbs << 24) | ((uint32_t) data_buffer[0] << 16) | ((uint32_t) data_buffer[1] << 8) | ((uint32_t) data_buffer[2]));
	}
	
	// put the mean value into output_data
	data_buffer_value = data_buffer_value / number_samples;
	output_data[0] = (uint8_t) (data_buffer_value >> 16);
	output_data[1] = (uint8_t) (data_buffer_value >> 8);
	output_data[2] = (uint8_t) data_buffer_value; 

	// success
	return 1;
}

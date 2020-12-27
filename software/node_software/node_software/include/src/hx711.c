/*
 * hx711.c
 *
 * Created: 23/04/2020 23:15:18
 *  Author: sylvain
 */

#include <avr/io.h>
#include <util/delay.h>

void HX711_init(){
	// set up HX711 to make it ready to use

	// setting up PD_SCK pin to output at low level
	PD_SCK_DDR = PD_SCK_DDR | (1 << PD_SCK_PIN);
	PD_SCK_PORT = PD_SCK_PORT & ~(1 << PD_SCK_PIN);

	// setting up DOUT pin to input at Hi-Z
	DOUT_DDR = DOUT_DDR & ~(1 << DOUT_PIN);
	DOUT_PORT = DOUT_PORT & ~(1 << DOUT_PIN);

	// wait until HX711 ready to send data, a conversion is made if it was in sleep mode
	while( (DOUT_INPUT & (1 << DOUT_PIN) ) != 0){
		asm volatile("nop");
	}

	// wait for "DOUT falling edge to PD_SCK rising edge"
	_delay_us(0.1);
}

uint8_t HX711_is_ready(){
	// return 0 if HX711 is not ready to send data, return 1 if HX711 ready

	if( (DOUT_INPUT & (1 << DOUT_PIN) ) == 0){ // DOUT at low level, ready
		_delay_us(0.1); // wait to make sure for "DOUT falling edge to PD_SCK rising edge"
		return 1;
	}
	else{
		return 0;
	}
}

void HX711_sleep(){
	// make HX711 inter in sleep mode

	// setting up PD_SCK pin to output at high level
	PD_SCK_DDR = PD_SCK_DDR | (1 << PD_SCK_PIN);
	PD_SCK_PORT = PD_SCK_PORT | (1 << PD_SCK_PIN);

	// 60us are required to really enter en sleep mode
}

void HX711_change_mode(uint8_t mode){
	// change channel and gain for next data sending

	// --> mode = 0 : channel A and gain 128
	// --> mode = 1 : channel B and gain 32
	// --> mode = 2 : channel B and gain 64

	// default mode after power up or sleep is channel A and gain 128

	// setting up PD_SCK pin to output at low level
	PD_SCK_DDR = PD_SCK_DDR | (1 << PD_SCK_PIN);
	PD_SCK_PORT = PD_SCK_PORT & ~(1 << PD_SCK_PIN);

	// wait until HX711 is ready
	while(HX711_is_ready() == 0){
		asm volatile("nop");
	}

	// send correct SCK pulses
	for(int i = 0; i < (25 + mode); i++){
		// SCK high level
		PD_SCK_PORT = PD_SCK_PORT | (1 << PD_SCK_PIN);

		_delay_us(1); // SCK high time

		// SCK low level
		PD_SCK_PORT = PD_SCK_PORT & ~(1 << PD_SCK_PIN);

		_delay_us(1); // SCK low time
	}

}

int32_t HX711_read_stored_conversion(uint8_t next_conversion_mode){
	// read the stored conversion

	// initialize data
	uint32_t data = 0x00000000;

	// wait until HX711 is ready
	while(!HX711_is_ready());

	// read data
	for(int i = 0; i < 24; i++){ // for each 24 bits of data
		// SCK high level
		PD_SCK_PORT = PD_SCK_PORT | (1 << PD_SCK_PIN);

		_delay_us(1); // SCK high time

		//store data
		data = data << 1;
		if( (DOUT_INPUT & (1 << DOUT_PIN) ) != 0){ // if high level
			data = data | (1 << 0); // store a 1
		}

		// SCK low level
		PD_SCK_PORT = PD_SCK_PORT & ~(1 << PD_SCK_PIN);

		_delay_us(1); // SCK low time
	}

	// configure the next conversion
	for(uint8_t i = 0; i <= next_conversion_mode; i++){
		// SCK high level
		PD_SCK_PORT = PD_SCK_PORT | (1 << PD_SCK_PIN);

		_delay_us(1); // SCK high time

		// SCK low level
		PD_SCK_PORT = PD_SCK_PORT & ~(1 << PD_SCK_PIN);

		_delay_us(1); // SCK low time
	}

	// change sign bit from 24th bit to 32nd bit
	if(data & 0x800000){
		data = data | 0xFF000000;
	}

	return (int32_t) data;
}

int32_t HX711_read_new_conversion(uint8_t conversion_mode, uint8_t next_conversion_mode){
	// start a conversion and read data by configuring the next conversion mode

	// configuring and start a new conversion
	HX711_change_mode(conversion_mode);

	// read data and configure the next configure at the same time
	return (HX711_read_stored_conversion(next_conversion_mode));
}

int32_t HX711_multiple_conversion_average(uint8_t conversion_mode, uint8_t number_samples){
	// return the average of wanted samples

	// initialize the data variable with a first conversion
	int32_t data = HX711_read_new_conversion(conversion_mode, conversion_mode);

	// add directly the wanted samples, mode was configured before
	for(uint8_t i = 0; i < number_samples; i++){
		data = data + HX711_read_stored_conversion(conversion_mode);
	}

	// return the average
	return (data / number_samples);
}

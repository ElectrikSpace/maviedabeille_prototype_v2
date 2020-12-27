 /*
 * sht3x.c
 *
 * Created: 30/10/2020 21:54:47
 *  Author: sylvain
 */
 #include <avr/io.h>
 #include <util/delay.h>
 #include "i2c_master.h"

 uint8_t sht3x_reset() {
	/* make soft reset on the chip */
	uint8_t data[3] = {(SHT3X_ADDRESS<<1), 0x30, 0xa2};
	uint8_t size = 3;
	i2c_start_write(data, size);
	_delay_ms(4); // include 3 ms for command transmitting at 10kHz and 1 ms soft reset
	return (i2c_status_OK);
 }

 uint16_t sht3x_get_command(uint8_t reliability) {
	 /* return the command to send */
	 uint16_t command;
	 if (SHT3X_CLOCK_STRETCHING) {
		 switch(reliability) {
			 case 0x01 :
				command = 0x2c10;
				break;
			 case 0x02 :
				command = 0x2c0d;
				break;
			case 0x03 :
				command = 0x2c06;
				break;
			default :
				command = 0x0000;
				break;
		 }
	 }
	 else {
		 command = 0x2400;
		 switch(reliability) {
			 case 0x01 :
				 command = 0x2c16;
				 break;
			 case 0x02 :
				 command = 0x2c0b;
				 break;
			 case 0x03 :
				 command = 0x2c00;
				 break;
			 default :
				 command = 0x0000;
				 break;
		 }
	 }
	 return (command);
 }

void sht3x_delay(uint8_t reliability) {
	 switch (reliability) {
		 case 0x01 :
			_delay_ms(4);
			break;
		 case 0x02 :
			_delay_ms(6);
			break;
		 case 0x03 :
			_delay_ms(14);
			break;
		 default :
			break;
	 }
 }

 uint8_t sht3x_check_crc(uint8_t *data) {
	 /* return 1 if CRC check is OK, 0 otherwise */
	 uint32_t working_data = ( (uint32_t) ( data[0] ^ 0xFF ) << 16 ) | (uint32_t) ( data[8] << 8 ) | (uint32_t) data[2];
	 uint8_t is_success;
	 uint16_t CRC;
	 uint16_t poly = 0x0131;
	 uint8_t i;
	 for (i = 16; i >= 1; i--) { // for all values to shift
		 i--;
		 CRC = (working_data >> i); // put the 9 bits to calculate with in a uint_16t
		 working_data = working_data & (0xFFFF >> (16-i) ); // clean data from old 9 bits
		 if ( CRC & (1 << 8) ) { // if MSB is 1
			 CRC = ( CRC ^ poly ); // XOR with the poly, here x^8 + x^5 + x^4 + 1
			 CRC = CRC & 0x01FF; // make sure unused bits are 0, optional
		 }
		 working_data = working_data | (CRC << i); // concatenate CRC 9 bits with existing data
		 i++;
	 }
	 if (working_data) { // if remain is 0
		is_success = 0;
	 }
	 else {
		is_success = 1;
	 }
	 return is_success;
 }

 uint8_t sht3x_decode_data(uint8_t *data, uint8_t is_only_temperature, float *temperature, float *humidity) {
	 /* decode data using CRC */
	 uint8_t is_success;

	 // CRC check algorithm
	 if (is_only_temperature) {
		is_success = sht3x_check_crc(data);
	 }
	 else {
		is_success = sht3x_check_crc(data) & sht3x_check_crc(data+3);
	 }

	 // get data
	 if (is_success) {

		*temperature = -45 + 175*( (float) ( ( data[0] << 8 ) | data[1] ) / 65535 );
		if (!is_only_temperature) {
			*humidity = 100*( (float) ( ( data[4] << 8 ) | data[5] ) / 65535 );
		}
	 }

	 return is_success;
 }

 uint8_t sht3x_measurement(uint8_t reliability, float *temperature, float *humidity, uint8_t is_only_temperature) {
	 /* get temperature and humidity on sht3x, return 1 if success 0 otherwise */
   uint8_t is_success;
   uint8_t data_length;
   uint8_t command_length;
   uint16_t command = sht3x_get_command(reliability);
   if (is_only_temperature) {
     data_length = 3;
   }
   else {
     data_length = 6;
   }
   if (SHT3X_CLOCK_STRETCHING) {
     command_length = data_length + 3;
   }
   else {
     command_length = 3;
   }
	 if (command) {
		 uint8_t data[6] = {(SHT3X_ADDRESS<<1), (uint8_t) command & 0xff, (uint8_t) (command>>8) & 0xff };
		 i2c_start_write(data, command_length);
		 if (i2c_get_status()) {
			 if (!SHT3X_CLOCK_STRETCHING) {
				 sht3x_delay(reliability);
         // send a download request
         data[0] = (SHT3X_ADDRESS<<1) | 0x01;
         i2c_start_write(data, data_length+1);
         _delay_ms(1); // download time
			 }
       else {
         while (i2c_is_transceiver_busy());
       }
			 if (i2c_read_buffer(data, data_length)) {
				 if (sht3x_decode_data(data, is_only_temperature, temperature, humidity)) {
					 is_success = 1;
				 }
				 else {
					 is_success = 0;
				 }
			 }
			 else {
				 is_success = 0;
			 }

		 }
		 else {
			 is_success = 0;
		 }
	 }
	 else {
		 is_success = 0;
	 }
	 return (is_success);
 }

 uint8_t sht3x_get_temperature(uint8_t reliability, float *temperature) {
	  /* only get temperature, return 1 if success 0 otherwise */
	  return sht3x_measurement(reliability, temperature, temperature, 1);
  }

 uint8_t sht3x_get_humidity(uint8_t reliability, float *humidity) {
	  /* only get humidity, return 1 if success 0 otherwise */
	  return sht3x_measurement(reliability, humidity, humidity, 0);
  }

uint8_t sht3x_get_measurement(uint8_t reliability, float *humidity, float *temperature) {
	  /* get temperature and humidity, return 1 if success 0 otherwise */
	  return sht3x_measurement(reliability, humidity, temperature, 0);
  }

 /*
 * i2c_master.c
 *
 * Created: 30/10/2020 21:54:47
 *  Author: sylvain
 */

#include <avr/io.h>
#include <avr/interrupt.h>

uint8_t i2c_status_OK; // 1 if OK, 0 otherwise
uint8_t i2c_data_size; // size of the message data
uint8_t i2c_buffer[I2C_BUFFER_SIZE]; // initialize the buffer

void i2c_init() {
	/* initialize i2c transceiver */
  TWBR = I2C_FREQUENCY_REGISTER;                    // Set bit rate register (Baud rate). Defined in header file
  TWSR = TWSR & ( (0<<TWPS1)|(0<<TWPS0) );			// set prescaler to 1
  TWDR = 0xFF;                                      // Default content = SDA released
  TWCR = (1<<TWEN)|                                 // Enable TWI-interface and release TWI pins
         (0<<TWIE)|(0<<TWINT)|                      // Disable Interrupt
         (0<<TWEA)|(0<<TWSTA)|(0<<TWSTO);           // No Signal requests
  TWAR = (0<<TWGCE);								// disable slave mode
  sei(); // enable global interrupts
  i2c_status_OK = 1;
}

void i2c_end() {
	/* stop i2c transceiver and release pins */
	TWCR = (0<<TWEN)|                                 // Disable TWI-interface and release TWI pins
		   (0<<TWIE)|(0<<TWINT)|                      // Disable Interrupt
		   (0<<TWEA)|(0<<TWSTA)|(0<<TWSTO);           // No Signal requests
}

uint8_t i2c_is_transceiver_busy() {
	/* return 0 if not busy, otherwise return a non 0 value */
	return ( TWCR & (1<<TWIE) ) ; // TWI interrupt is disabled if transceiver is not busy
}

uint8_t i2c_get_status() {
	/* return the status of i2c communication */
	while (i2c_is_transceiver_busy()); // wait transceiver not to be busy
	return (i2c_status_OK);
}

void i2c_start_write(uint8_t *data, uint8_t size) {
	/* start a write sequence, data must contain the address+r/w bit
	   keep in mind that the process use interrupt and all data is not sent at the end of the function */
	uint8_t i;
	i2c_data_size = size;
	while (i2c_is_transceiver_busy()); // wait transceiver not to be busy
	if (data[0] & (1<<0)) {
		// read message
		i2c_buffer[0] = data[0];
		if (size <= I2C_BUFFER_SIZE) { // prevent from non 1 data size
			i2c_data_size = size;
			i2c_status_OK = 1; // no error
		}
		else {
			i2c_status_OK = 0; // no error
		}
	}
	else {
    // write message
		if (size <= I2C_BUFFER_SIZE) {
			for(i=0; i < size; i++) {
				i2c_buffer[i] = data[i];
			}
			i2c_data_size = size;
			i2c_status_OK = 1; // no error
		}
		else {
			i2c_status_OK = 0; // error
		}
	}
	if (i2c_status_OK) {
		TWCR = (1<<TWEN)|                             // TWI Interface enabled.
			   (1<<TWIE)|(1<<TWINT)|                  // Enable TWI Interrupt and clear the flag.
			   (0<<TWEA)|(1<<TWSTA)|(0<<TWSTO);       // Initiate a START condition.
	}
}

void i2c_resend_last_data() {
	/* resend last message */
	while (i2c_is_transceiver_busy()); // wait transceiver not to be busy
	i2c_status_OK = 0;
	TWCR = (1<<TWEN)|                             // TWI Interface enabled.
		   (1<<TWIE)|(1<<TWINT)|                  // Enable TWI Interrupt and clear the flag.
		   (0<<TWEA)|(1<<TWSTA)|(0<<TWSTO);       // Initiate a START condition.
}

uint8_t i2c_read_buffer(uint8_t *data, uint8_t size) {
	/* read the buffer of the transceiver */
	uint8_t i;
	uint8_t is_success;
	while (i2c_is_transceiver_busy());
	if (i2c_status_OK && size <= I2C_BUFFER_SIZE) {
		for (i=0; i < size; i++) {
			data[i] = i2c_buffer[i];
		}
		is_success = 1;
	}
	else {
		is_success = 0;
	}
	return (is_success);
}

ISR(TWI_vect) {
	/* TWI interrupt sub routine */
	static uint8_t i2c_buffer_pointer;

	switch (TWSR & ~((1<<TWPS0)|(1<<TWPS1))) // i2c master status code
	{
		// keep in mind the behavior of a switch statement without break at some points
		// take a look at the i2c status codes

		// transmitter
		case 0x08 : // start transmitted
			i2c_buffer_pointer = 0;
		case 0x10 : // repeated start transmitted
		case 0x18 : // (address + W) transmitted
		case 0x28 : // data transmitted
			if (i2c_buffer_pointer < i2c_data_size) { // if remain data in buffer
				TWDR = i2c_buffer[i2c_buffer_pointer]; // copy data into transceiver data register
				i2c_buffer_pointer++; // increment pointer offset
				TWCR = (1<<TWEN)|                                 // TWI Interface enabled
					   (1<<TWIE)|(1<<TWINT)|                      // Enable TWI Interrupt and clear the flag to send byte
					   (0<<TWEA)|(0<<TWSTA)|(0<<TWSTO);           // no signal
			}
			else { // no more data to transmit
				TWCR = (1<<TWEN)|                                 // TWI Interface enabled
					   (0<<TWIE)|(1<<TWINT)|                      // Disable TWI Interrupt and clear the flag
					   (0<<TWEA)|(0<<TWSTA)|(1<<TWSTO);           // Initiate a STOP condition.
				i2c_status_OK = 1;
			}
			break;
		// receiver
		case 0x50 : // data received + ack transmitted
			i2c_buffer[i2c_buffer_pointer] = TWDR;
			i2c_buffer_pointer++; // increment pointer offset
		case 0x40 : // (address + R) transmitted + ack received
			if (i2c_buffer_pointer < ( i2c_data_size - 1 ) ) {
				// not the last byte to be received
				TWCR = (1<<TWEN)|                                 // TWI Interface enabled
					   (1<<TWIE)|(1<<TWINT)|                      // Enable TWI Interrupt and clear the flag to read next byte
					   (1<<TWEA)|(0<<TWSTA)|(0<<TWSTO);           // Send ACK after reception
			}
			else {
				TWCR = (1<<TWEN)|                                 // TWI Interface enabled
				   	   (1<<TWIE)|(1<<TWINT)|                      // Enable TWI Interrupt and clear the flag to read next byte
				       (0<<TWEA)|(0<<TWSTA)|(0<<TWSTO);           // Send NACK after reception
			}
			break;
		case 0x58 : // data received + nack transmitted
			i2c_buffer[i2c_buffer_pointer] = TWDR;
			i2c_status_OK = 1;
			TWCR = (1<<TWEN)|								  // TWI Interface enabled
				   (0<<TWIE)|(1<<TWINT)|                      // Disable TWI Interrupt and clear the flag
				   (0<<TWEA)|(0<<TWSTA)|(1<<TWSTO);           // Initiate a STOP condition.
			break;
		case 0x38 : // loss
			TWCR = (1<<TWEN)|                                 // TWI Interface enabled
				   (1<<TWIE)|(1<<TWINT)|                      // Enable TWI Interrupt and clear the flag
				   (0<<TWEA)|(1<<TWSTA)|(0<<TWSTO);           // Initiate a (RE)START condition.
			i2c_status_OK = 0;
		default :
			TWCR = (1<<TWEN)|                                 // Enable TWI-interface and release TWI pins
			       (0<<TWIE)|(0<<TWINT)|                      // Disable Interrupt
				   (0<<TWEA)|(0<<TWSTA)|(0<<TWSTO);           // No Signal requests
			i2c_status_OK = 0;
	}
}

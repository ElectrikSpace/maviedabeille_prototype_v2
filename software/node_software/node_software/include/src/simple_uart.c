/*
 * uart.c
 *
 * Created: 30/10/2020 21:54:47
 *  Author: sylvain
 */ 

#include <avr/io.h>
#include <util/delay.h>
#include <string.h>

void serial_init(uint32_t baud){
	/* init UART bus*/
	
	// set UBBRn registers with baud rate
	uint16_t value = F_CPU / (16*baud) - 1;
	UBRR0L = value & 0xFF;
	UBRR0H = (value>>7) && 0xFF;
	//Enable receiver and transmitter
	UCSR0B = (1<<RXEN0)|(1<<TXEN0);
	// Set frame format: 8data, 2stop bit
	UCSR0C = (3<<UCSZ00);
}

void serial_send(char *string) {
	/* transmit the string*/
	
	// Wait for empty transmit buffer
	int iterator = 0;
	char current_char = 0;
	do
	{
		current_char = string[iterator];
		iterator++;
		while (!(UCSR0A & (1<<UDRE0)));
		// Put data into buffer, sends the data
		UDR0 = current_char;
	}
	while(current_char != '\0');
}

char serial_receive(){
	/* wait until a character arrives and return it */

	// Wait for data to be received
	while (!(UCSR0A & (1<<RXC0)));
	// Get and return received data from buffer
	return UDR0;
}


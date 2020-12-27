/*
* spi_master.c
*
* Created: 30/10/2020 21:54:47
*  Author: sylvain
*/

#include <avr/io.h>
#include <util/delay.h>

uint8_t spi_buffer[SPI_BUFFER_SIZE];
uint8_t spi_buffer_pointer;

void spi_init() {
  /* init SPI */
  PORTB = PORTB | ( (1<<5)|(1<<3)|(1<<2) );  // sck, mosi set to high, pullup to ss
  DDRB = DDRB | ( (1<<5)|(1<<3) );            // sck, mosi set as output
  DDRB = DDRB & ( !((1<<4)|(1<<2)) );           // miso, ss set as input
  SPCR = (0<<SPIE)|                               // interrupt disabled
         (1<<SPE)|                                // spi enabled
         (SPI_DATA_ORDER<<DORD)|                  // data order
         (1<<MSTR)|                               // master mode
         (SPI_CPOL<<CPOL)|                        // clock polarity
         (SPI_CPHA<<CPHA)|                        // clock phase
         (SPI_SPR1<<SPR1)|(SPI_SPR0<<SPR0);       // spi clock speed
  SPSR = (SPI_SPI2X<<SPI2X);
}

void spi_end() {
  /* stop SPI */
  SPCR = (0<<SPIE)|(0<<SPE); // disable SPI
}

void spi_send(uint8_t *data, uint8_t size) {
  /* send data other SPI */
  uint8_t i;
  for (i = 0; i < size; i++) {
    while (!(SPSR & (1<<SPIF))); // wait for transceiver to be available
    SPDR = data[i]; // copy data into transceiver data register
  }
  while (!(SPSR & (1<<SPIF))); // wait for transceiver to finish transmitting
}

uint8_t spi_receive_byte() {
  /* return data buffer */
  while(!(SPSR & (1<<SPIF) ));
  return (SPDR);
};

void spi_flush_buffer() {
	/* flush the buffer */
	spi_buffer_pointer = 0;
}

void spi_start_listen() {
  /* start read listen, data is saved on spi buffer, do not send data during this period */
  while (!(SPSR & (1<<SPIF))); // wait for transceiver not to be busy
  spi_flush_buffer();
  SPCR = SPCR | (1<<SPIE); // enable SPI interrupt
  sei();
}

void spi_stop_listen() {
  /* stop read listen */
  while (!(SPSR & (1<<SPIF))); // wait for transceiver not to be busy
  SPCR = SPCR & !(1<<SPIE); // disable SPI interrupt
}

uint8_t spi_read_from_buffer(uint8_t *data, uint8_t max_size) {
  /* this returns 0 if error or current buffer > max_size, real data size otherwise,
     do not flush the buffer at the end, do not stop listening */
  uint8_t data_number;
  if ( (spi_buffer_pointer) <= max_size) {
    for (data_number = 0; data_number < spi_buffer_pointer; data_number++) {
      data[data_number] = spi_buffer[data_number];
    }
    data_number++;
  }
  else {
    data_number = 0;
  }
  return (data_number);
}

ISR(SPI_STC_vect) {
  /* interrupt sub routine when data is received */
  if (spi_buffer_pointer < SPI_BUFFER_SIZE) {
    spi_buffer[spi_buffer_pointer] = SPDR;
    spi_buffer_pointer++;
  }
}

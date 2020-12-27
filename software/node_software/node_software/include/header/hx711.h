 /* 
 * hx711.c
 *
 * Created: 30/10/2020 21:54:47
 *  Author: sylvain
 */

#ifndef HX711_H_
#define HX711_H_

#define PD_SCK_IO_BLOCK C // SCK IO block on MCU
#define PD_SCK_PIN 2 // SCK PIN on the IO block
#define DOUT_IO_BLOCK C // DOUT IO block on MCU
#define DOUT_PIN 3 // DOUT on the IO block

#if PD_SCK_IO_BLOCK == B
	#define PD_SCK_PORT PORTB
	#define PD_SCK_DDR DDRB
#elif PD_SCK_IO_BLOCK == C
	#define PD_SCK_PORT PORTC
	#define PD_SCK_DDR DDRC
#elif PD_SCK_IO_BLOCK == D
	#define PD_SCK_PORT PORTD
	#define PD_SCK_DDR DDRD
#endif
#if DOUT_IO_BLOCK == B
	#define DOUT_PORT PORTB
	#define DOUT_DDR DDRB
	#define DOUT_INPUT PINB
#elif DOUT_IO_BLOCK == C
	#define DOUT_PORT PORTC
	#define DOUT_DDR DDRC
	#define DOUT_INPUT PINC
#elif DOUT_IO_BLOCK == D
	#define DOUT_PORT PORTD
	#define DOUT_DDR DDRD
	#define DOUT_INPUT PIND
#endif

#include "hx711.c"

extern void HX711_init();
extern uint8_t HX711_is_ready();
extern void HX711_sleep();
extern void HX711_change_mode(uint8_t mode);
extern int32_t HX711_read_stored_conversion(uint8_t next_conversion_mode);
extern int32_t HX711_read_new_conversion(uint8_t conversion_mode, uint8_t next_conversion_mode);
extern int32_t HX711_multiple_conversion_average(uint8_t conversion_mode, uint8_t number_samples);

#endif

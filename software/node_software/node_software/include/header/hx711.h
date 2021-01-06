 /* 
 * hx711.c
 *
 * Created: 30/10/2020 21:54:47
 *  Author: sylvain
 */

#ifndef HX711_H_
#define HX711_H_

#define PD_SCK_DDR DDRC
#define PD_SCK_PORT PORTC
#define PD_SCK_BIT 2

#define DOUT_DDR DDRC
#define DOUT_PORT PORTC
#define DOUT_PIN PINC
#define DOUT_BIT 3

#include "hx711.c"

extern void hx711_init();
extern void hx711_end();
extern uint8_t hx711_wait_until_ready(uint16_t timeout);
extern uint8_t hx711_change_mode(uint8_t conversion_mode);
extern uint8_t hx711_read_stored_conversion(uint8_t next_conversion_mode, uint8_t *output_data);
extern uint8_t hx711_read_new_conversion(uint8_t conversion_mode, uint8_t next_conversion_mode, uint8_t *output_data);
extern uint8_t hx711_multiple_conversion_average(uint8_t conversion_mode, uint8_t number_samples, uint8_t *output_data);

#endif

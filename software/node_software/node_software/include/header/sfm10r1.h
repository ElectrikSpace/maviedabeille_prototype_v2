/*
* sfm10r1.h
*
* Created: 30/10/2020 21:54:47
*  Author: sylvain
*/

#ifndef SFM10R1_H_
#define SFM10R1_H_

#define SFM10R1_RESET_DDR DDRD
#define SFM10R1_RESET_DDR_BIT DDD7
#define SFM10R1_RESET_PORT PORTD
#define SFM10R1_RESET_PORT_BIT PORTD7

#define SFM10R1_BAUD_RATE 9600
#define SFM10R1_UART_TIMEOUT 500 // in ms, not used for transmitting and receiving functions

#include "sfm10r1.c"

extern void sfm10r1_reset();
extern void sfm10r1_init();
extern void sfm10r1_end();
extern uint8_t sfm10r1_ping();
extern uint8_t sfm10r1_get_ID(uint8_t *id);
extern uint8_t sfm10r1_get_PAC(uint8_t *pac);
extern uint8_t sfm10r1_send_data(char *data, uint8_t size);
extern uint8_t sfm10r1_send_data_downlink(char *data, uint8_t size, uint8_t *received_data);
extern uint8_t sfm10r1_get_temperature(float *temperature);
extern uint8_t sfm10r1_get_voltage(float *voltage);
extern uint8_t sfm10r1_get_transmit_repeats(uint8_t *repeats);
extern uint8_t sfm10r1_set_transmit_repeats(uint8_t number);
extern uint8_t sfm10r1_save_config();

#endif

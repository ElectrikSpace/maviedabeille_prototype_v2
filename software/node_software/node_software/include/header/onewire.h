/*
* onewire.h
*
* Created: 30/10/2020 21:54:47
*  Author: sylvain
*/

#ifndef ONEWIRE_H_
#define ONEWIRE_H_

#define ONEWIRE_DDR DDRC
#define ONEWIRE_PORT PORTC
#define ONEWIRE_PIN PINC
#define ONEWIRE_BIT 1
#define ONEWIRE_SKIP_ROM 0xcc
/* not implemented yet
#define ONEWIRE_READ_ROM 0x33
#define ONEWIRE_MATCH_ROM 0x55
#define ONEWIRE_SEARCH_ROM 0xf0
#define ONEWIRE_CONDITIONAL_SEARCH 0xec
*/

#include "onewire.c"

extern void onewire_pin_low();
extern void onewire_pin_release();
extern uint8_t onewire_pin_read();
extern void onewire_init();
extern uint8_t onewire_reset();
extern void onewire_write(uint8_t data);
extern uint8_t onewire_read();
extern void onewire_skip_rom();

#endif

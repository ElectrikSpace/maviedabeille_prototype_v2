/*
* onewire.h
*
* Created: 30/10/2020 21:54:47
*  Author: sylvain
*/

#ifndef DS18B20_H_
#define DS18B20_H_

#define DS18B20_CONVERT 0x44
#define DS18B20_COPY_SCRATCHPAD 0x48
#define DS18B20_WRITE_SCRATCHPAD 0x4e
#define DS18B20_READ_SCRATCHPAD 0xbe
#define DS18B20_RECALL_E2 0xb8
#define DS18B20_READ_POWER_SUPPLY 0xb4

#include "ds18b20.c"

/* only support one device per bus, multiple devices per bus is not implemented yet */
/* alarm system is not implemented yet */

extern uint8_t ds18b20_setup(uint8_t binary_resolution);
extern uint8_t ds18b20_start_conversion();
extern uint8_t ds18b20_read_temperature(float *temperature);
extern uint8_t ds18b20_get_temperature(float *temperature);

#endif

/*
 * node_software.c
 *
 * Created: 26/10/2020 17:48:33
 * Author : sylvain
 */ 

#define F_CPU 8000000UL

#include <avr/io.h>
#include <util/delay.h>
#include <string.h>

#include "power.h"
/*#include "hx711.h"
#include "onewire.h"*/
#include "simple_uart.h"/*
#include "i2c_master.h"
#include "ds3231.h"
#include "ds18b20.h"
#include "sht3x.h"
#include "software_uart.h"
#include "sfm10r1.h"*/

// not used yet 
/*#include "spi_master.h"
#include "at24c32.h"*/

int main(void)
{
    /* Replace with your application code */
	power_on_external_peripherals();
	serial_init(9600);
    while (1) 
    {
		char data[3] = {'a', '\n', '\0'};
		data[0] = serial_receive();
		serial_send(data);
    }
}


/*
* power.h
*
* Created: 30/10/2020 21:54:47
*  Author: sylvain
*/

#ifndef POWER_H_
#define POWER_H_

#define POWER_EXTERNAL_PERIPHERALS_STARTUP_TIME 100  // in ms
#define POWER_VREF 3.3 // ADC reference voltage
#define POWER_R30 300000 // R30 resistor value on schematics
#define POWER_R31 150000 // R31 resistor value on schematics

// state led
#define POWER_STATE_LED_DDR DDRD
#define POWER_STATE_LED_PORT PORTD
#define POWER_STATE_LED_BIT 6
#define POWER_ON 1
#define POWER_OFF 

#include "power.c"

extern float power_get_battery_voltage();
extern float power_get_solar_intensity();
extern void power_on_external_peripherals();
extern void power_off_external_peripherals();
extern void power_enable_wakeup_interrupt();
extern void power_disable_wakeup_interrupt();
extern void power_on_state_led();
extern void power_off_state_led();
extern void power_blink_state_led(uint16_t high_time, uint16_t low_time, uint32_t total_time);

#endif

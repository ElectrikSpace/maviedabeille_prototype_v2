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

// clock
#define USE_EXTERNAL_OSCILLATOR 0 // 1 for external oscillator use, 0 for internal oscillator use
#define EXTERNAL_OSCILLATOR_FREQUENCY 8000000 // frequency in Hz of the external oscillator if used

// battery level
#define POWER_BATTERY_MIN_STARTUP 3.5 // minimum battery level at startup in Volt
#define POWER_R30 300000 // R30 resistor value on schematics
#define POWER_R31 150000 // R31 resistor value on schematics

// state led
#define POWER_STATE_LED_DDR DDRD
#define POWER_STATE_LED_PORT PORTD
#define POWER_STATE_LED_BIT 6

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
extern uint8_t power_setup_clock();
extern uint8_t power_battery_startup_check();

#endif

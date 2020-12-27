/*
* power.c
*
* Created: 30/10/2020 21:54:47
*  Author: sylvain
*/

#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>

float power_get_battery_voltage() {
  /* return the voltage of the battery */
  float row_voltage;

  // setup ADC
  ADMUX = (1 << MUX2) | (1 << MUX1) | (1 << MUX0); // ADC7
  ADCSRA = (1 << ADEN) | (1 << ADPS2) | (1 << ADPS1);
  ADCSRB = 0;

  // start conversion
  ADCSRA = ADCSRA | (1 << ADSC);

  // wait until conversion is finished
  while( !(ADCSRA & (1 << ADIF)) );

  // disable ADC
  ADCSRA = ADCSRA & ~(1 << ADEN);

  // process result
  row_voltage = (float) ((ADCL + ADCH*256)*POWER_VREF)/1024; // measured voltage
  return row_voltage / (float) (POWER_R30/(POWER_R30 + POWER_R31)); // adjusted voltage because of the voltage divider bridge
}

float power_get_solar_intensity() {
  /* return the voltage on luminosity pin */

  // setup ADC
  ADMUX = (1 << MUX2) | (1 << MUX1); // ADC6
  ADCSRA = (1 << ADEN) | (1 << ADPS2) | (1 << ADPS1);
  ADCSRB = 0;

  // start conversion
  ADCSRA = ADCSRA | (1 << ADSC);

  // wait until conversion is finished
  while( !(ADCSRA & (1 << ADIF)) );

  // disable ADC
  ADCSRA = ADCSRA & ~(1 << ADEN);

  // process result
  return (float) ((ADCL + ADCH*256)*POWER_VREF)/1024;
}

void power_on_external_peripherals() {
  /* switch on all sensors and peripheral ICs */

  // put external peripherals voltage regulator signal to HIGH (PC0)
  DDRC = DDRC | (1 << DDC0);
  PORTC = PORTC | (1 << PORTC0);

  // wait until external peripherals start up complete
  _delay_ms(POWER_EXTERNAL_PERIPHERALS_STARTUP_TIME);
}

void power_off_external_peripherals() {
  /* switch off all sensors and peripheral ICs */

  // put external peripherals voltage regulator signal to HIGH (PC0)
  DDRC = DDRC & ~(1 << DDC0);
  PORTC = PORTC & ~(1 << PORTC0);
}

void power_enable_wakeup_interrupt() {
  /* set wake up pin as external wake up interrupt and enable gloab interrupts */

  // setup wake up pin as input
  DDRD = DDRC & ~(1 << PORTD2);
  PORTD = PORTD & ~(1 << DDD2);

  // enable wake up pin interrupt
  EICRA = EICRA & ~( (1 << 1) | (1 << 0) ); // detect on low level
  EIMSK = EIMSK | (1 << 0);

  // enable global interrupts
  sei();
}

void power_disable_wakeup_interrupt() {
  /* disable wake up pin as external wake up interrupt and disable global interrupt */

  // disable wake up pin interrupt
  EIMSK = EIMSK & ~(1 << 0);

  // disable global interrupts
  cli();
}

void power_on_state_led() {
	// set state led as output
	POWER_STATE_LED_DDR = POWER_STATE_LED_DDR | (1 << POWER_STATE_LED_BIT);
	
	// put state led pin in low state
	POWER_STATE_LED_PORT = POWER_STATE_LED_PORT & ~(1 << POWER_STATE_LED_BIT);
}

void power_off_state_led() {
  // set state led as output
  POWER_STATE_LED_DDR = POWER_STATE_LED_DDR | (1 << POWER_STATE_LED_BIT);
  
  // put state led pin in high state
  POWER_STATE_LED_PORT = POWER_STATE_LED_PORT | (1 << POWER_STATE_LED_BIT);
}

void power_blink_state_led(uint16_t high_time, uint16_t low_time, uint32_t total_time){
  /* blink led (hold CPU) during total_time + end of current state time when total_time occurs
     high_time, low_time, and total_time is in ms
  */
  uint32_t time = 0;
  uint16_t delay_time;
  uint8_t state = 0;

  POWER_STATE_LED_DDR = POWER_STATE_LED_DDR | (1 << POWER_STATE_LED_BIT);
  while (time < total_time) {
    if (state) {
      state = 0;
      time = time + low_time;
      POWER_STATE_LED_PORT = POWER_STATE_LED_PORT & ~(1 << POWER_STATE_LED_BIT);
      delay_time = 0;
      while (delay_time < low_time) {
        _delay_ms(1);
        delay_time = delay_time + 1;
      }
    }
    else {
      state = 1;
      time = time + high_time;
      POWER_STATE_LED_PORT = POWER_STATE_LED_PORT | (1 << POWER_STATE_LED_BIT);
      delay_time = 0;
      while (delay_time < high_time) {
          _delay_ms(1);
          delay_time = delay_time + 1;
      }
    }
  }
}

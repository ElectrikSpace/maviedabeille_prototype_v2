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
  float multiplier = ( (float) POWER_R30 + POWER_R31 ) / ( (float) POWER_R30 );
  float adjusted_voltage;

  // setup and start ADC
  ADMUX = (1 << MUX2) | (1 << MUX1) | (1 << MUX0); // ADC7
  ADCSRA = (1 << ADEN) | (1 << ADPS2) | (1 << ADPS1);
  ADCSRB = 0;

  // start conversion
  ADCSRA = ADCSRA | (1 << ADSC);

  // wait until conversion is finished
  while( !(ADCSRA & (1 << ADIF)) );

  // process result
  row_voltage = (float) ((ADCL + ADCH*256)*POWER_VREF)/1024; // measured voltage
  
  adjusted_voltage = (float) row_voltage*multiplier; // adjusted voltage because of the voltage divider bridge
  
  // disable ADC
  ADCSRA = ADCSRA & ~(1 << ADEN);

  return adjusted_voltage;
}

float power_get_solar_intensity() {
  /* return the voltage on luminosity pin */
  float raw_voltage;

  // setup and start ADC
  ADMUX = (1 << MUX2) | (1 << MUX1); // ADC6
  ADCSRA = (1 << ADEN) | (1 << ADPS2) | (1 << ADPS1);
  ADCSRB = 0;

  // start conversion
  ADCSRA = ADCSRA | (1 << ADSC);

  // wait until conversion is finished
  while( !(ADCSRA & (1 << ADIF)) );

  // process result
  raw_voltage = (float) ((ADCL + ADCH*256)*POWER_VREF)/1024;

  // disable ADC
  ADCSRA = ADCSRA & ~(1 << ADEN);

  // return result
  return raw_voltage;
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
  
  // release led pin
  POWER_STATE_LED_PORT = POWER_STATE_LED_PORT & ~(1 << POWER_STATE_LED_BIT);
}

void power_enable_wakeup_interrupt() {
  /* set wake up pin as external wake up interrupt and enable global interrupts */

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
  /* disable wake up pin as external wake up interrupt and disable global interrupts */

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
	  POWER_STATE_LED_PORT = POWER_STATE_LED_PORT | (1 << POWER_STATE_LED_BIT);
      delay_time = 0;
      while (delay_time < low_time) {
        _delay_ms(1);
        delay_time = delay_time + 1;
      }
    }
    else {
      state = 1;
      time = time + high_time;
      POWER_STATE_LED_PORT = POWER_STATE_LED_PORT & ~(1 << POWER_STATE_LED_BIT);
      delay_time = 0;
      while (delay_time < high_time) {
          _delay_ms(1);
          delay_time = delay_time + 1;
      }
    }
  }
  POWER_STATE_LED_PORT = POWER_STATE_LED_PORT | (1 << POWER_STATE_LED_BIT);
}

uint8_t power_setup_clock() {
	/* setup the desired clock system, return 1 if config is correct, 0 otherwise */
	uint8_t prescaler_value;
	uint8_t CLKPS;

	// check if F_CPU is correct
	if (USE_EXTERNAL_OSCILLATOR) {
		if (EXTERNAL_OSCILLATOR_FREQUENCY < 1200000 || EXTERNAL_OSCILLATOR_FREQUENCY > 16000000) {
			return 0;
		}
		if (F_CPU > EXTERNAL_OSCILLATOR_FREQUENCY) {
			return 0;
		}
	}
	else {
		if (F_CPU < 1200000 || F_CPU > 8000000) {
			return 0;
		}
	} 
	
	// calculate prescaler value
	if (USE_EXTERNAL_OSCILLATOR) {
		prescaler_value = EXTERNAL_OSCILLATOR_FREQUENCY / F_CPU;
	}
	else {
		prescaler_value = 8000000 / F_CPU;
	}
	
	// calculate CLKPS bits
	if (prescaler_value == 1) {
		CLKPS = 0x00;
	}
	else if (prescaler_value == 2) {
		CLKPS = (1 << 0);
	}
	else if (prescaler_value <= 4) {
		CLKPS = (1 << 1);
	}
	else if (prescaler_value <= 8) {
		CLKPS = (1 << 0) | (1 << 1);
	}
	else { // never enter here
		CLKPS = 0x00;
	}
	
	// change clock
	CLKPR = CLKPR | (1 << 7); // enable clock change
	CLKPR = CLKPS | (1 << 7); // change prescaler
	
	_delay_us(1); // wait clock change is done (4 clock cycles min)

	return 1;
}


uint8_t power_battery_startup_check() {
	/* return 1 if battery level is correct, 0 otherwise */ 
	
	// get current battery level
	float battery_level = power_get_battery_voltage();
	
	// check if battery level is correct
	if (battery_level >= POWER_BATTERY_MIN_STARTUP) {
		return 1;
	}
	
	return 0;
}
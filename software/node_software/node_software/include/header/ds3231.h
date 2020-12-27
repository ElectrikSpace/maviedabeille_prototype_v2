/*
* ds3231.h
*
* Created: 30/10/2020 21:54:47
*  Author: sylvain
*/

#ifndef DS3231_H_
#define DS3231_H_

 /* please initialize i2c before */

#define DS3231_ADDRESS (uint8_t) 0x68

#define DS3231_ALARM_1_ONCE_PER_SECOND (1 << 3) | (1 << 2) | (1 << 1) | (1 << 0)
#define DS3231_ALARM_1_MATCH_SECONDS (1 << 3) | (1 << 2) | (1 << 1)
#define DS3231_ALARM_1_MATCH_SECONDS_MINUTES (1 << 3) | (1 << 2)
#define DS3231_ALARM_1_MATCH_SECONDS_MINUTES_HOURS (1 << 3)
#define DS3231_ALARM_1_MATCH_SECONDS_MINUTES_HOURS_DAYOFTHEWEEK 0
#define DS3231_ALARM_1_MATCH_SECONDS_MINUTES_HOURS_DAYOFTHEMONTH (1 << 4)

#define DS3231_ALARM_2_ONCE_PER_MINUTE (1 << 2) | (1 << 1) | (1 << 0)
#define DS3231_ALARM_2_MATCH_MINUTES (1 << 2) | (1 << 1)
#define DS3231_ALARM_2_MATCH_MINUTES_HOURS (1 << 2)
#define DS3231_ALARM_2_MATCH_MINUTES_HOURS_DAYOFTHEWEEK 0
#define DS3231_ALARM_2_MATCH_MINUTES_HOURS_DAYOFTHEMONTH (1 << 3)

#include "ds3231.c"

extern uint8_t ds3231_reset();
extern uint8_t ds3231_is_running();
extern uint8_t ds3231_enable_32kHz_pin();
extern uint8_t ds3231_disable_32kHz_pin();
extern uint8_t ds3231_enable_interrupt_pin();
extern uint8_t ds3231_disable_interrupt_pin();
extern uint8_t ds3231_enable_square_wave_pin(uint16_t frequency);
extern uint8_t ds3231_disable_square_wave_pin();
extern uint8_t ds3231_set_datetime(uint8_t seconds, uint8_t minutes, uint8_t hours, uint8_t day_of_week, uint8_t day_of_month, uint8_t month, uint16_t year);
extern uint8_t ds3231_get_datetime(uint8_t *seconds, uint8_t *minutes, uint8_t *hours, uint8_t *day_of_week, uint8_t *day_of_month, uint8_t *month, uint16_t *year);
extern uint8_t ds3231_get_time(uint8_t *seconds, uint8_t *minutes, uint8_t *hours);
extern uint8_t ds3231_get_date(uint8_t *day_of_week, uint8_t *day_of_month, uint8_t *month, uint16_t *year);
extern uint8_t ds3231_set_alarm_1(uint8_t seconds, uint8_t minutes, uint8_t hours, uint8_t day, uint8_t alarm_mode_bits, uint8_t is_day_of_the_week);
extern uint8_t ds3231_enable_alarm_1();
extern uint8_t ds3231_enable_interrupt_alarm_1();
extern uint8_t ds3231_disable_alarm_1();
extern uint8_t ds3231_disable_interrupt_alarm_1();
extern uint8_t ds3231_set_alarm_2(uint8_t minutes, uint8_t hours, uint8_t day, uint8_t alarm_mode_bits, uint8_t is_day_of_the_week);
extern uint8_t ds3231_enable_alarm_2();
extern uint8_t ds3231_enable_interrupt_alarm_2();
extern uint8_t ds3231_disable_alarm_2();
extern uint8_t ds3231_disable_interrupt_alarm_2();
extern uint8_t ds3231_clear_alarm_flags(uint8_t *cleared_alarms);
extern uint8_t ds3231_get_temperature(float *temperature);

#endif

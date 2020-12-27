 /*
 * sht3x.h
 *
 * Created: 30/10/2020 21:54:47
 *  Author: sylvain
 */

 #ifndef SHT3X_H_
 #define SHT3X_H_

 #define SHT3X_CLOCK_STRETCHING 0
 #define SHT3X_ADDRESS 0x44 // 0x44 if ADDR pin to VSS (default), 0x45 if ADDR pin to VDD

 #include "sht3x.c"

 /* please initialize i2c before */

 extern uint8_t sht3x_reset(); // return 1 if success
 extern uint8_t sht3x_get_temperature(uint8_t reliability, float *temperature); // return 1 if success
 extern uint8_t sht3x_get_humidity(uint8_t reliability, float *humidity); // return 1 if success
 extern uint8_t sht3x_get_measurement(uint8_t reliability, float *humidity, float *temperature); // return 1 if success

 #endif

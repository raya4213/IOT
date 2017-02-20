/*
 * i2c.h
 *
 *  Created on: Sep 28, 2016
 *      Author: Rahul Yamasani
 */

#ifndef I2C_H_
#define I2C_H_
volatile int led_status;
void i2c_config();
void Enable_TSL2651();
void Disable_TSL2651();
void i2c_WriteByte(uint8_t I2C_tx_buffer[], uint8_t size);
uint8_t i2c_ReadByte();
//void i2c_ReadByte();
void gpio_interrupt_Enable();
void readCheck();
uint8_t ADC_High;
uint8_t ADC_Low;
#endif /* I2C_H_ */

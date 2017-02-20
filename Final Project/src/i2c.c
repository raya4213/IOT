/*
 * i2c.c
 *
 *  Created on: Sep 28, 2016
 *      Author: Rahul Yamasani
 */
#include "main.h"
#include "em_i2c.h"
#include "i2c.h"
#include "em_int.h"
#include "em_leuart.h"
#include <stdio.h>

I2C_TransferReturn_TypeDef I2C_Status;     // To know the status of the I2C1

uint8_t I2C_tx_buffer1[] = {0x00};         // Buffer for address in readByte function
uint8_t I2C_tx_buffer_size1 = 1;	       // Size of readByte buffer

uint8_t I2C_rx_buffer[I2C_rx_buffer_size]; // receive buffer

/*****************************************************
 * Enable_TSL2651(): Enables TSL2561
 * 1. Sets power supply to TSL2561
 * 2. Configures  the necessary registers to power on
 * 3. Enables the interrupt pin
 *****************************************************/
void Enable_TSL2651()
{

	uint8_t I2C_tx_buffer[2]; // I2C_tx_buffer[0]:command I2C_tx_buffer[1]:value to write
	//GPIO_DriveModeSet(I2C1_INTERRUPT_PORT, gpioDriveModeLowest);
	//GPIO_DriveModeSet(I2C1_PORT, gpioDriveModeLowest);
	GPIO_PinModeSet(I2C1_PORT, I2C1_SCL, gpioModeWiredAndPullUpFilter, 0);
	GPIO_PinModeSet(I2C1_PORT, I2C1_SDA, gpioModeWiredAndPullUpFilter, 0);
	GPIO_PinModeSet(I2C1_INTERRUPT_PORT, I2C1_VDD, gpioModePushPull, 0);
	GPIO_PinModeSet(I2C1_INTERRUPT_PORT, I2C1_InterruptPin, gpioModeInput, 0);
	//GPIO_DriveModeSet(I2C1_INTERRUPT_PORT, gpioDriveModeLowest);
	GPIO_PinOutSet(I2C1_INTERRUPT_PORT, I2C1_VDD);  // Setting the vcc

	// providing a delay to facilitate setup time
	for (uint16_t i = 0; i<1000; i++);

	GPIO_PinOutSet(I2C1_INTERRUPT_PORT, I2C1_InterruptPin);
	GPIO_PinOutSet(I2C1_PORT, I2C1_SCL);
	GPIO_PinOutSet(I2C1_PORT, I2C1_SDA);

	// Writing 00 to control register
	I2C_tx_buffer[0] = TSL2561_CONTROL_REG;
	I2C_tx_buffer[1] = 0x00;
	i2c_WriteByte(I2C_tx_buffer, 2);

	// Writing 0x03 (power on) to control register
	I2C_tx_buffer[0] = TSL2561_CONTROL_REG;
	I2C_tx_buffer[1] = 0x03;
	i2c_WriteByte(I2C_tx_buffer,2);

	// Writing 0x01 for 101ms (power on) to Timing register
	I2C_tx_buffer[0] = TSL2561_TIMING_REG;
	I2C_tx_buffer[1] = 0x01;
	i2c_WriteByte(I2C_tx_buffer,2);

	// Writing 0x0f in TSL2561_THRESLowLow_REG
	I2C_tx_buffer[0] = TSL2561_THRESLowLow_REG;
	I2C_tx_buffer[1] = 0x0f;
	i2c_WriteByte(I2C_tx_buffer,2);

	// Writing 0x00 in TSL2561_THRESLowhigh_REG
	I2C_tx_buffer[0] = TSL2561_THRESLowhigh_REG;
	I2C_tx_buffer[1] = 0x00;
	i2c_WriteByte(I2C_tx_buffer,2);

	// Writing 0x00 in TSL2561_THRESHighLow_REG
	I2C_tx_buffer[0] = TSL2561_THRESHighLow_REG;
	I2C_tx_buffer[1] = 0x00;
	i2c_WriteByte(I2C_tx_buffer,2);

	// Writing 0x08 in TSL2561_THRESHighHigh_REG
	I2C_tx_buffer[0] = TSL2561_THRESHighHigh_REG;
	I2C_tx_buffer[1] = 0x08;
	i2c_WriteByte(I2C_tx_buffer,2);

	// Writing 0x14 in TSL2561_INTERRUPT_REG
	I2C_tx_buffer[0] = TSL2561_INTERRUPT_REG;
	I2C_tx_buffer[1] = 0x14;
	i2c_WriteByte(I2C_tx_buffer,2);

	// Enables the interrupt
	gpio_interrupt_Enable();
}

/*****************************************************
 * Disable_TSL2651(): Diables TSL2561
 * 1. Sets power supply to TSL2561
 * 2. Configures  the necessary registers to power on
 * 3. Enables the interrupt pin
 *****************************************************/
void Disable_TSL2651()
{
	uint8_t I2C_tx_buffer[2];

	// power of the device
	I2C_tx_buffer[0] = TSL2561_CONTROL_REG;
	I2C_tx_buffer[1] = 0x00;
	i2c_WriteByte(I2C_tx_buffer, 2);

	// Disable and clear the interrupts
	GPIO_IntConfig(I2C1_INTERRUPT_PORT, I2C1_InterruptPin, false, true, false);   // rising or falling (rising for now)
	GPIO_IntClear(GPIO_IntGet());  // Disabling the interrupts
	NVIC_ClearPendingIRQ(GPIO_ODD_IRQn);
	NVIC_DisableIRQ(GPIO_ODD_IRQn);
	//GPIO_DriveModeSet(I2C1_INTERRUPT_PORT, gpioDriveModeStandard);
	GPIO_DriveModeSet(I2C1_INTERRUPT_PORT, gpioDriveModeLowest);

	// Clearing the power supply to tsl2561
//	GPIO_PinOutClear(I2C1_INTERRUPT_PORT, I2C1_InterruptPin);
//	GPIO_PinOutClear(I2C1_PORT, I2C1_SCL);
//	GPIO_PinOutClear(I2C1_PORT, I2C1_SDA);
	GPIO_PinModeSet(I2C1_PORT, I2C1_SCL, gpioModeDisabled, 0);
	GPIO_PinModeSet(I2C1_PORT, I2C1_SDA, gpioModeDisabled, 0);
	GPIO_PinModeSet(I2C1_INTERRUPT_PORT, I2C1_VDD, gpioModeDisabled, 0);
	GPIO_PinModeSet(I2C1_INTERRUPT_PORT, I2C1_InterruptPin, gpioModeDisabled, 0);

	GPIO_PinOutClear(I2C1_INTERRUPT_PORT, I2C1_VDD);
}

/*****************************************************
 * i2c_WriteByte(uint8_t I2C_tx_buffer[], uint8_t size)
 * 1. Writes a byte
 * 2. Takes input as buffer to send and its size
 * 3. Waits till transfer is complete
 *****************************************************/
void i2c_WriteByte(uint8_t I2C_tx_buffer[], uint8_t size)
{
	I2C_TransferSeq_TypeDef Writeseq;
	Writeseq.addr = SLAVE_ADDRESS << 1;
	Writeseq.flags = I2C_FLAG_WRITE;

	Writeseq.buf[0].data = I2C_tx_buffer;
	Writeseq.buf[0].len = size;

	Writeseq.buf[1].data = I2C_rx_buffer;
	Writeseq.buf[1].len = I2C_rx_buffer_size;

	//I2C_Status = I2C_TransferInit(I2C1, &Writeseq);
	I2C_TransferInit(I2C1, &Writeseq);
	while(I2C_Transfer(I2C1) == i2cTransferInProgress);
}

/*****************************************************
 * uint8_t i2c_ReadByte()
 * 1. Reads a I2C byte
 * 2. Receives the value and puts it in a buffer
 * 3. Waits till reception is complete
 * 4. Returns the byte read
 *****************************************************/

uint8_t i2c_ReadByte()
{

	I2C_TransferSeq_TypeDef Readseq;
	// Sending the address
	Readseq.addr = SLAVE_ADDRESS << 1;
	Readseq.flags = I2C_FLAG_WRITE_READ;  // this flag corresponds to restart event

	// sends the command
	Readseq.buf[0].data = I2C_tx_buffer1;
	Readseq.buf[0].len = 1;

	// stores the received value
	Readseq.buf[1].data = I2C_rx_buffer;
	Readseq.buf[1].len = I2C_rx_buffer_size;

	// checking for status
	I2C_Status = I2C_TransferInit(I2C1, &Readseq);
	while(I2C_Status != i2cTransferDone)
	{
		if (I2C_Status != i2cTransferInProgress)
		{
			break;
		}
		I2C_Status = I2C_Transfer(I2C1);
	}

	return I2C_rx_buffer[0];

}

/*****************************************************
 * void GPIO_ODD_IRQHandler()
 * 1. Gets the final ADC_value
 * 2. Determines led0 on and off
 *****************************************************/
size_t event_check = 0;
void GPIO_ODD_IRQHandler()
{
	INT_Disable();
	GPIO_IntClear(GPIO_IntGet());
	if ((event_check & 0x01) == 0)
	{
		uint8_t I2C_tx_buffer[1];
		//uint8_t I2C_tx_buffer1[2];

		I2C_tx_buffer[0] = 0xC0;
		i2c_WriteByte(I2C_tx_buffer, 1);

		I2C_tx_buffer1[0] = TSL2561_DATA0Low;
		ADC_Low = i2c_ReadByte();

		I2C_tx_buffer1[0] = TSL2561_DATA0High;
		ADC_High = i2c_ReadByte();

		uint16_t ADC_Final = ADC_High<<8 | ADC_Low;

		if (ADC_Final < TSL2561_LowerLimit)
		{
			//GPIO_PinOutSet(gpioPortE,2);
			LED0_ON;
			led_status = 0x01;
			//LEUART_Tx(LEUART0, 0x01);
	//  		I2C_tx_buffer1[0] = TSL2561_THRESLowLow_REG;
	//  		I2C_tx_buffer1[1] = 0x00;
	//  		i2c_WriteByte(I2C_tx_buffer1, 2);
	//
	//  		I2C_tx_buffer1[0] = TSL2561_THRESLowhigh_REG;
	//  		I2C_tx_buffer1[1] = 0x00;
	//  		i2c_WriteByte(I2C_tx_buffer1, 2);
		}

		else if (ADC_Final > TSL2561_UpperLimit)
		{
			//GPIO_PinOutClear(gpioPortE,2);
			LED0_OFF;
			led_status = 0x00;
			//LEUART_Tx(LEUART0, 0x00);
	//  		I2C_tx_buffer1[0] = TSL2561_THRESHighLow_REG;
	//  		I2C_tx_buffer1[1] = 0xff;
	//  		i2c_WriteByte(I2C_tx_buffer1, 2);
	//
	//  		I2C_tx_buffer1[0] = TSL2561_THRESHighHigh_REG;
	//  		I2C_tx_buffer1[1] = 0xff;
	//  		i2c_WriteByte(I2C_tx_buffer1, 2);
		}
	}
	event_check++;
	INT_Enable();
}


/*****************************************************
 * void gpio_interrupt_Enable()
 * 1. Enables interrupt for tsl2561 on falling edge
 *****************************************************/
void gpio_interrupt_Enable()
{
	// Enbles the interrupt
	GPIO_IntConfig(I2C1_INTERRUPT_PORT, I2C1_InterruptPin, false, true, true);   // falling edge
	NVIC_ClearPendingIRQ(GPIO_ODD_IRQn);
	NVIC_EnableIRQ(GPIO_ODD_IRQn);
}

/*****************************************************
 * void i2c_config()
 * 1. Configures i2c
 *****************************************************/
void i2c_config()
{

	//CMU_ClockEnable(cmuClock_HFPER, true);
	CMU_ClockEnable(cmuClock_I2C1, true);
	//I2C_Init_TypeDef i2cInit = I2C_INIT_DEFAULT;
	I2C_Init_TypeDef i2cInit =

			{                                                                         \
			  true,                    /* Enable when init done */                    \
			  true,                    /* Set to master mode */                       \
			  0,                       /* Use currently configured reference clock */ \
			  I2C_FREQ_STANDARD_MAX,   /* Set to standard rate assuring being */      \
									   /* within I2C spec */                          \
			  i2cClockHLRStandard      /* Set to use 4:4 low/high duty cycle */       \
			};

	I2C1->ROUTE = I2C_ROUTE_SDAPEN |   // /**< SDA Pin Enable */
				  I2C_ROUTE_SCLPEN; //& ~(_I2C_ROUTE_LOCATION_LOC0);
	I2C_Init(I2C1, &i2cInit);

	// Exit the busy state
	if (I2C1->STATE & I2C_STATE_BUSY){
		I2C1->CMD = I2C_CMD_ABORT;
	}


	I2C1->SADDR = SLAVE_ADDRESS << 1; // Giving the slave address
	// Enableing auto acknoledgement, slave control and autosn
	I2C1->CTRL |= (I2C_CTRL_AUTOACK | I2C_CTRL_SLAVE | I2C_CTRL_AUTOSN);

}



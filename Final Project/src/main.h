/*
 * main.h
 *
 *  Created on: Sep 20, 2016
 *      Author: Rahul Yamasani
 */

#ifndef MAIN_H_
#define MAIN_H_

#include "em_cmu.h"
#include "em_letimer.h"
#include "em_gpio.h"
#include "em_timer.h"
#include "em_leuart.h"
#include "circBuffer.h"
#include "em_dma.h"
//#include <stdio.h>

// Energy modes

#define EM0 0
#define EM1 1
#define EM2 2
#define EM3 3

//Circular buffer stuff
#define CIRC_BUFFER_ENABLE
#define CIRC_BUFFER_SIZE 100
//#define CIRC_BUFF_BACK2BACK
#define LEUART0_GPIO    gpioPortD
#define LEUART0_TX_PIN  4
#define LEUART0_RX_PIN  5

// User can change the following variables
#define ENERGY_MODE 		  EM2   // defines the energy mode
#define LED0_TURNON_TIME      0.004 // Turn on time for GPIO_D Pin 6
#define LED0_DUTYCYCLE_TIME   6.25

//#define DISABLE_ONBOARD_PAS_LIGHTSENSOR


//When DMA_TURN_OFF is commented dma is enabled
//When DMA_TURN_OFF is uncommented dma is disabled
//#define DMA_TURN_OFF

//When ADC_POLLING is commented & dma is disabled adc data capture is done using ADC0_IRQHandler
//When ADC_POLLING is uncommented & dma is disabled adc data capture is done using polling
//#define ADC_POLLING
#define NO_OF_ADCSAMPLES 500
// Energy mode for ADC blocking
#define ADC_EM 1
// Energy mode for I2C blocking
#define I2C_EM 1
// Upper and lower limits for temperature sensor
#define TEMP_UPPER_LIMIT 35
#define TEMP_LOWER_LIMIT 15

// LED 0 and 1 set and clear
#define LED0_OFF GPIO_PinOutClear(gpioPortE,2)   // Enabling the LED 0
#define LED0_ON GPIO_PinOutSet(gpioPortE,2)
#define LED1_OFF GPIO_PinOutClear(gpioPortE,3)   // Enabling the LED 0
#define LED1_ON GPIO_PinOutSet(gpioPortE,3)   // Enabling the LED 0

// Used for controlling the ambient light sensor
#define Light_Excite_Port gpioPortD
#define Light_Excite_Pin  6
#define Light_Sensor_Port gpioPortC
#define Light_Sensor_Pin  6

#define Light_Sensor_AcmpRef         acmpChannelVDD;
#define Light_Sensor_AcmpChannel     acmpChannel6;

// Used for i2c
// Using PD6 (SDA) and PD7 (SCL)
#define I2C1_PORT   gpioPortC
#define I2C1_INTERRUPT_PORT gpioPortD
#define I2C1_SCL     5
#define I2C1_SDA     4
#define I2C1_InterruptPin  1

#define I2C1_VDD     0
#define SLAVE_ADDRESS 0x39
#define I2C_rx_buffer_size 10
#define TSL2561_UpperLimit 0x0800
#define TSL2561_LowerLimit 0x000f

// ADDRESS FOR REGISTERS ON TSL2651
#define TSL2561_CONTROL_REG          0x80
#define TSL2561_TIMING_REG			 0x81
#define TSL2561_THRESLowLow_REG      0x82
#define TSL2561_THRESLowhigh_REG     0x83
#define TSL2561_THRESHighLow_REG     0x84
#define TSL2561_THRESHighHigh_REG    0x85
#define TSL2561_INTERRUPT_REG        0x86

#define TSL2561_DATA0Low			 0x8c
#define TSL2561_DATA0High			 0x8d
#define TSL2561_DATA1Low			 0x8e
#define TSL2561_DATA1High			 0x8f

// Setting for ADC single init typedef
#define ADC_Singleinit_Input    adcSingleInpTemp
#define ADC_Singleinit_Ref      adcRef1V25
#define ADC_Singleinit_Resol    adcRes12Bit
#define ADC_Singleinit_AcqTime  adcAcqTime1
#define ADCFreq                 1300000//975000//130000
#define ADC_Select_HFPER_Freq   0

#define LEUART0_CHANNEL0 0
#define CHANNEL_ADC_0 1
#define CHANNEL_NO 2
DMA_CB_TypeDef callBack[CHANNEL_NO];
// Parameters for DMA Channel config for LEUART0

#define LEUART0_Dma_ChannelCfgSelect DMAREQ_LEUART0_TXBL

// Parameters for DMA destination config
#define LEUART0_Dma_DestCfg_dstInc  dmaDataIncNone
#define LEUART0_Dma_DestCfg_srcInc  dmaDataInc1
#define LEUART0_Dma_DestCfg_size    dmaDataSize1
#define LEUART0_Dma_DestCfg_arbRate dmaArbitrate1

// Parameters for DMA Channel config for ADC0
#define Dma_ChannelCfgSelect DMAREQ_ADC0_SINGLE

// Parameters for DMA destination config
#define Dma_DestCfg_dstInc  dmaDataInc2
#define Dma_DestCfg_srcInc  dmaDataIncNone
#define Dma_DestCfg_size    dmaDataSize2
#define Dma_DestCfg_arbRate dmaArbitrate1
// Used for Energy mode EM3
#if ENERGY_MODE == EM3
	#define ENABLE_EM3
	#define ENABLE_SELF_CALIB           // Enables Self calibration
#endif


#endif /* MAIN_H_ */

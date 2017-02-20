// Header Files

#include <stdint.h>
#include <stdbool.h>
#include "em_device.h"
#include "em_chip.h"
#include <malloc.h>
#include "em_acmp.h"
#include "sleep.h"
#include "dmactrl.h"
#include "em_dma.h"
#include "main.h"
#include "letimer_gpio_clock.h"
#include "self_calibtarion.h"
#include "acmp.h"
#include "adc.h"
#include "i2c.h"
#include "leuart.h"
#include <string.h>
#include "circBuffer.h"
#include "segmentlcd.h"
cbuffer tx_buffer;
cbuffer rx_buffer;


volatile int16_t ramBufferAdcData[NO_OF_ADCSAMPLES];
extern volatile int led_status;
#define AMBI_High_Thres 0x3D
#define AMBI_Low_Thres  0x2
int ULF_TIMEPeriod =  1000;         // Ultra low frequency clock period
#define LF_TIMEPeriod  8092        // Low frequency clock period

// Used for storing count values
int LED0_TURNON_COUNT;
int LED0_DUTYCYCLE_COUNT;
float avrgTemp = 20;      //Making sure the LED 1 is turned off initially
// Function Prototypes
void LETIMER0_IRQHandler(void);
//void Letimer_clock();
//void Gpio_clock_config();
//void ACMP_Config();
void LETIMER_Config();

int handle;   // Used for setting count on  LETIMER0_CompareSet
//extern uint8_t ADC_High;
//extern uint8_t ADC_Low;
ACMP_Init_TypeDef acmpInit  = {
	  false,              /* fullBias */                                        \
	  true,              /* halfBias */                                        \
	  0x0,                /* biasProg */                                        \
	  true,              /* No interrupt on falling edge. */                   \
	  true,              /* No interrupt on rising edge. */                    \
	  acmpWarmTime512,    /* 512 cycle warmup to be safe */                     \
	  acmpHysteresisLevel5,                                                     \
	  false,              /* Disabled emitting inactive value during warmup. */ \
	  true,              /* low power reference */                             \
	  0x02,               /* VDD level */                                       \
	  true                /* Enable after init. */

};

void sendUartCircBuff()
{
	int i=0,j=0;
	int8_t remove_element;
	int8_t sendBuffer[3];
	//LEUART_Tx(LEUART0, remove_element);
	cbuffer_remove(&tx_buffer, &sendBuffer[0]);
	cbuffer_remove(&tx_buffer, &sendBuffer[1]);
	cbuffer_remove(&tx_buffer, &sendBuffer[2]);
	//flagForUart = 0;
	dmaControlBlock->SRCEND = sendBuffer+3-1;
	/* Enable DMA wake-up from LEUART1 TX */
	LEUART0->CTRL = LEUART_CTRL_TXDMAWU;

	/* (Re)starting the transfer. Using Basic Mode */
	DMA_ActivateBasic(LEUART0_CHANNEL0,                  /* Activate channel selected */
					true,                         /* Use primary descriptor */
					false,                        /* No DMA burst */
					NULL,                         /* Keep destination address */
					NULL,                         /* Keep source address*/
					3 - 1);                 /* Size of buffer minus1 */
//	for (i=0; i<3;i++)
//	{
//		cbuffer_remove(&tx_buffer, &remove_element);
//		LEUART_Tx(LEUART0, remove_element);
//		#if (ENERGY_MODE == EM3)
//			for (j = 0;j<3500;j++);
//		#endif
//	}
}

void sendTempToSamB11(float TempSend)
{
	//blockSleepMode(EM2);
	int i = 0, j=0;
	int8_t IntPart, FloatPart;
	IntPart = (int)(TempSend);
	FloatPart = (int)((TempSend - IntPart) * 10);
#ifdef CIRC_BUFFER_ENABLE
	int8_t remove_element;
	cbuffer_add(&tx_buffer, &led_status);
	cbuffer_add(&tx_buffer, &IntPart);
	cbuffer_add(&tx_buffer, &FloatPart);

	sendUartCircBuff();

//	for (i=0; i<3;i++)
//	{
//		cbuffer_remove(&tx_buffer, &remove_element);
//		LEUART_Tx(LEUART0, remove_element);
//		#if (ENERGY_MODE == EM3)
//			for (j = 0;j<3500;j++);
//		#endif
//	}
//	cbuffer_remove(&tx_buffer, &remove_element);
//	LEUART_Tx(LEUART0, remove_element);
//#if (ENERGY_MODE == EM3)
//	for (i = 0;i<2500;i++);
//#endif
//	cbuffer_remove(&tx_buffer, &remove_element);
//	LEUART_Tx(LEUART0, remove_element);
//#if (ENERGY_MODE == EM3)
//	for (i = 0;i<2500;i++);
//#endif
//	cbuffer_remove(&tx_buffer, &remove_element);
//	LEUART_Tx(LEUART0, remove_element);
//#if (ENERGY_MODE == EM3)
//	for (i = 0;i<2500;i++);
//#endif

//#if (ENERGY_MODE == EM3)
//	for (i = 0;i<2500;i++);
//#endif
//	cbuffer_remove(&tx_buffer, &remove_element);
//	LEUART_Tx(LEUART0, remove_element);
//#if (ENERGY_MODE == EM3)
//	for (i = 0;i<2500;i++);
//#endif
//	LEUART_Tx(LEUART0, FloatPart);
//#if (ENERGY_MODE == EM3)
//	for (i = 0;i<2500;i++);
//#endif

#else
	LEUART_Tx(LEUART0, led_status);
#if (ENERGY_MODE == EM3)
	for (i = 0;i<2500;i++);
#endif
	LEUART_Tx(LEUART0, IntPart);
#if (ENERGY_MODE == EM3)
	for (i = 0;i<2500;i++);
#endif
	LEUART_Tx(LEUART0, FloatPart);
#if (ENERGY_MODE == EM3)
	for (i = 0;i<2500;i++);
#endif
#endif
	//unblockSleepMode(EM2);
}

void ADCdmaTransferDone(unsigned int channel, bool primary, void *user);
//void dma_config();

/**************************************************************************//**
 * @file
 * @brief Internal temperature sensor example for EFM32GG_STK3700
 * @details
 *   Show temperature using internal sensor on the EFM32.
 * @note
 *   Due to bugs in earlier chip revisions, this demo only works correctly for
 *   revision C chips or later.
 *
 * @par Usage
 * @li Buttons toggle Celcius and Fahrenheit temperature modes
 *
 * @version 3.20.12
 ******************************************************************************
 * @section License
 * <b>(C) Copyright 2014 Silicon Labs, http://www.silabs.com</b>
 *******************************************************************************
 *
 * This file is licensed under the Silabs License Agreement. See the file
 * "Silabs_License_Agreement.txt" for details. Before using this software for
 * any purpose, you must agree to the terms of that agreement.
 *
 ******************************************************************************
 * The function float convertCelsius(int32_t adcSample) is referenced from
 * the file written for internal temp sensor as in above disclaimer
 *
 * I would like to credit Silicon Labs and Professor Keith A. Graham, University
 * of Colorado, Boulder for the following function
 * float convertCelsius(int32_t adcSample)
 *
 * I agree to the terms and conditions of Silicon labs and Prof. Keith A. Graham
 * for the use of this code.
*****************************************************************************/

/*****************************************************
 * float convertCelsius(int32_t adcSample)
 *
 * 1. Calculates the temperature
 *****************************************************/

float convertCelsius(int32_t adcSample)
{
	float temperature;
	float cal_temp_0 = (float) ((DEVINFO->CAL & _DEVINFO_CAL_TEMP_MASK)>>_DEVINFO_CAL_TEMP_SHIFT);
	float cal_value_0 = (float) ((DEVINFO->ADC0CAL2 & _DEVINFO_ADC0CAL2_TEMP1V25_MASK)>>_DEVINFO_ADC0CAL2_TEMP1V25_SHIFT);

	float t_grad = -6.27;
	temperature = (cal_temp_0 - ((cal_value_0-adcSample)/t_grad));

	return temperature;
}


/*****************************************************
 * ADCdmaTransferDone(unsigned int channel, bool primary, void *user)
 *
 * 1. Calculates the average of the NO_OF_ADCSAMPLES ADC Samples
 * 2. Calls convertCelsius to convert temperature into centigrade
 *****************************************************/

//DMA_CB_TypeDef ADC_cb;

void ADCdmaTransferDone(unsigned int channel, bool primary, void *user)
{
	int calcTemp = 0;
	unblockSleepMode(ADC_EM);               // Unblocking ADC from EM1
	ADC0->CMD |= ADC_CMD_SINGLESTOP;        // Stopping the ADC0
	for (uint16_t i =0; i<NO_OF_ADCSAMPLES;i++)
	{
		calcTemp += ramBufferAdcData[i];    // Calculating the average of the ADC samples
	}
	avrgTemp = convertCelsius((float)calcTemp/NO_OF_ADCSAMPLES);  // Temp in centigrade scale
	sendTempToSamB11(avrgTemp);

 }

/*****************************************************
 * dma_config(): Configures DMA
 *
 * 1. Initializes DMA with dmaControlBlock
 * 2. Sets the parameters for ADCdmaTransferDone
 * 3. Configures DMA Channel
 * 4. Configures DMA descriptor
 *****************************************************/

#ifndef DMA_TURN_OFF
void dma_config()
{
	CMU_ClockEnable(cmuClock_DMA, true);
	DMA_Init_TypeDef dmaInit;
	/* Initializing the DMA */

	/*HPROT signal state when accessing the primary/alternate
	 * descriptors. Normally set to 0 if protection is not an issue.*/
	dmaInit.hprot        = 0;
	dmaInit.controlBlock = dmaControlBlock;// giving the parameters for control block
	DMA_Init(&dmaInit);

	//  Setting up call-back function
	callBack[CHANNEL_ADC_0].cbFunc  = ADCdmaTransferDone;   // Associate call back function here
	callBack[CHANNEL_ADC_0].userPtr = NULL;                 // User defined pointer to provide with call back function
	callBack[CHANNEL_ADC_0].primary = true;

	DMA_CfgChannel_TypeDef  dmachnlCfg;
	// Setting up channel
	dmachnlCfg.highPri   = false;                 // Default priority
	dmachnlCfg.enableInt = true;                  // interrupt shall be enabled for channel
	dmachnlCfg.select    = Dma_ChannelCfgSelect;  // DMA channel select for ADC0_SINGLE
	dmachnlCfg.cb        = &callBack[CHANNEL_ADC_0];               // Associating the call back function with DMA channel

	DMA_CfgChannel(CHANNEL_ADC_0, &dmachnlCfg);
	DMA_CfgDescr_TypeDef    dmadescrCfg;

	// Setting up channel descriptor
	dmadescrCfg.dstInc  = Dma_DestCfg_dstInc;      // destination address: 2 bytes
	dmadescrCfg.srcInc  = Dma_DestCfg_srcInc;      // Source address:
	dmadescrCfg.size    = Dma_DestCfg_size;        // data size: 2 bytes
	dmadescrCfg.arbRate = Dma_DestCfg_arbRate;     // arbitrate:  set to zero
	dmadescrCfg.hprot   = 0;
	DMA_CfgDescr(CHANNEL_ADC_0, true, &dmadescrCfg);

}
#endif
/*****************************************************
 * TempLed1_Control(): Controls LED1 function
 *
 * 1.Sets LED 1 if temperature is outside limits
 * 2.Clears LED 1 if temperature is in limits
 *
 *****************************************************/

void TempLed1_Control()
{
	if (avrgTemp > TEMP_UPPER_LIMIT || avrgTemp < TEMP_LOWER_LIMIT)
	{
		LED1_ON;
	}

	else
	{
		LED1_OFF;

	}
}

uint32_t AdcIrqTemp=0;
void ADC0_IRQHandler()
{
	// Clearing the pending interrupts
	int intFlags;
	intFlags = ADC_IntGet(ADC0);
	ADC_IntClear(ADC0, intFlags);

	static uint32_t check = 0;
	AdcIrqTemp += ADC_DataSingleGet(ADC0);
	if (check == NO_OF_ADCSAMPLES - 1)
	{
		//AdcIrqCompleCheck = true;
		unblockSleepMode(ADC_EM);               // Unblocking ADC from EM1
		ADC0->CMD |= ADC_CMD_SINGLESTOP;        // Stopping the ADC0
		avrgTemp = convertCelsius((float)AdcIrqTemp/NO_OF_ADCSAMPLES);  // Temp in centigrade scale
		sendTempToSamB11(avrgTemp);
		AdcIrqTemp = 0;
		check = 0;
	}
	check++;
}

/*****************************************************
 * LETIMER0_IRQHandler: Interrupt handler for LETIMER0
 *
 * 1.Handles the interrupts for every 4ms and 3 sec alternatively
 * 2.Initializes the Ambient light sensor based on the comparator value
 *
 *****************************************************/

uint32_t adcSampleCount = 0;
uint8_t selectExternalTimerOperation = 0;
void LETIMER0_IRQHandler(void)
{
  // Clearing the interrupts
  int intFlags;
  intFlags = LETIMER_IntGet(LETIMER0);
  LETIMER_IntClear(LETIMER0, intFlags);

  blockSleepMode(ENERGY_MODE);  // enters into desired energy mode



  if (handle == LED0_DUTYCYCLE_COUNT)
  {
#ifndef DISABLE_ONBOARD_PAS_LIGHTSENSOR
	  int acmpStatusbit = (ACMP0->STATUS & ACMP_STATUS_ACMPOUT)>>_ACMP_STATUS_ACMPOUT_SHIFT;
	  	// comparator value is 0
	  	if (acmpStatusbit == 0)
	  	{
	  		LED0_ON;
	  		led_status = 0x01;
//#ifdef CIRC_BUFFER_ENABLE
//	  		cbuffer_add(&tx_buffer, &led_status);
//#endif
	  		//LEUART_Tx(LEUART0, 0x01);
	  		//GPIO_PinOutSet(gpioPortE,2);   // Enabling the LED 2
	  		acmpInit.vddLevel = AMBI_High_Thres;      // Setting higher threshold
	  		ACMP_Init(ACMP0, &acmpInit);   // Iniatilizing the ACMP timer
	  		// Setting parameters for channel 6
	  		ACMP_Channel_TypeDef negSel =  Light_Sensor_AcmpRef;  // selecting parameters for negative select
	  		ACMP_Channel_TypeDef posSel =  Light_Sensor_AcmpChannel;    // selecting parameters for positive select
	  		ACMP_ChannelSet(ACMP0,negSel,posSel);
	  	}

	  	// comparator value is 0
	  	else if (acmpStatusbit == 1)
	  	{
	  		LED0_OFF;
	  		led_status = 0x0;
//#ifdef CIRC_BUFFER_ENABLE
//	  		cbuffer_add(&tx_buffer, &led_status);
//#endif
	  		//LEUART_Tx(LEUART0, 0x00);
	  		//GPIO_PinOutClear(gpioPortE,2); // Disabling the LED 2
	  		acmpInit.vddLevel = AMBI_Low_Thres;       // Setting low threshold
	  		ACMP_Init(ACMP0, &acmpInit);
	  		ACMP_Channel_TypeDef negSel =  Light_Sensor_AcmpRef; // selecting parameters for negative select
	  		ACMP_Channel_TypeDef posSel =  Light_Sensor_AcmpChannel;   // selecting parameters for positive select
	  		ACMP_ChannelSet(ACMP0,negSel,posSel);

	  	}

	  	GPIO_PinOutClear(Light_Excite_Port,Light_Excite_Pin);
	  	ACMP_Disable(ACMP0);     // Disabling ADC
#endif
//	  	adcSampleCount = 0;
//#ifdef DMA_TURN_OFF
//	#ifdef ADC_POLLING
//			blockSleepMode(ADC_EM);
//			int calcTemp = 0;
//			while(adcSampleCount < NO_OF_ADCSAMPLES)
//			{
//				ADC_Start(ADC0, adcStartSingle);
//				while(ADC0->STATUS & ADC_STATUS_SINGLEACT);
//
//				calcTemp += ADC_DataSingleGet(ADC0);
//				adcSampleCount++;
//
//			}
//
//
//
//			unblockSleepMode(ADC_EM);               // Unblocking ADC from EM1
//			ADC0->CMD |= ADC_CMD_SINGLESTOP;        // Stopping the ADC0
//			avrgTemp = convertCelsius((float)calcTemp/NO_OF_ADCSAMPLES);  // Temp in centigrade scale
//	#else
//			blockSleepMode(ADC_EM);
//			ADC_Start(ADC0, adcStartSingle);
//	#endif
//
//#else
//		blockSleepMode(ADC_EM);
//		DMA_ActivateBasic(CHANNEL_ADC_0,  // DMA channel 0 to activate DMA cycle for.
//						true,             // primary descriptor ??
//						false,            // The burst feature is only used on peripherals supporting DMA bursts
//						(void *)ramBufferAdcData, // dst: Address to start location to transfer data to
//						(void *)&(ADC0->SINGLEDATA),// src: Address to start location to transfer data from
//						NO_OF_ADCSAMPLES - 1); // Number of DMA transfer elements (minus 1)
//		ADC_Start(ADC0, adcStartSingle);
//		// Implementing the call back routine
//
//
//#endif
//
//	  	TempLed1_Control();      // LED 1 Control

#ifdef DISABLE_ONBOARD_PAS_LIGHTSENSOR
	  	switch (selectExternalTimerOperation)
	  	{
	  	case 0:
		  	//blockSleepMode(I2C_EM);
	  		Enable_TSL2651();
	  		selectExternalTimerOperation = 1;
	  		break;
	  	case 1:

	  		selectExternalTimerOperation = 2;
	  		break;
	  	case 2:
	  		Disable_TSL2651();
	  		selectExternalTimerOperation = 0;
	  		//unblockSleepMode(I2C_EM);
	  		break;
	  	}
#endif
  }

  else
  {
#ifndef DISABLE_ONBOARD_PAS_LIGHTSENSOR
	  // Enabling Ambient light sensor for 4 ms
		ACMP_Enable(ACMP0);
	  	GPIO_PinOutSet(Light_Excite_Port,Light_Excite_Pin);
#endif
	  	adcSampleCount = 0;
	  	#ifdef DMA_TURN_OFF
	  		#ifdef ADC_POLLING
	  				blockSleepMode(ADC_EM);
	  				int calcTemp = 0;
	  				while(adcSampleCount < NO_OF_ADCSAMPLES)
	  				{
	  					ADC_Start(ADC0, adcStartSingle);
	  					while(ADC0->STATUS & ADC_STATUS_SINGLEACT);

	  					calcTemp += ADC_DataSingleGet(ADC0);
	  					adcSampleCount++;

	  				}



	  				unblockSleepMode(ADC_EM);               // Unblocking ADC from EM1
	  				ADC0->CMD |= ADC_CMD_SINGLESTOP;        // Stopping the ADC0
	  				avrgTemp = convertCelsius((float)calcTemp/NO_OF_ADCSAMPLES);  // Temp in centigrade scale
	  		#else
	  				blockSleepMode(ADC_EM);
	  				ADC_Start(ADC0, adcStartSingle);
	  		#endif

	  	#else
	  			blockSleepMode(ADC_EM);
	  			DMA_ActivateBasic(CHANNEL_ADC_0,  // DMA channel 0 to activate DMA cycle for.
	  							true,             // primary descriptor ??
	  							false,            // The burst feature is only used on peripherals supporting DMA bursts
	  							(void *)ramBufferAdcData, // dst: Address to start location to transfer data to
	  							(void *)&(ADC0->SINGLEDATA),// src: Address to start location to transfer data from
	  							NO_OF_ADCSAMPLES - 1); // Number of DMA transfer elements (minus 1)
	  			ADC_Start(ADC0, adcStartSingle);
	  			// Implementing the call back routine


	  	#endif

	  		  	TempLed1_Control();      // LED 1 Control
//	  	#ifdef DISABLE_ONBOARD_PAS_LIGHTSENSOR
//	  		  	switch (selectExternalTimerOperation)
//	  		  	{
//	  		  	case 0:
//	  			  	//blockSleepMode(I2C_EM);
//	  		  		Enable_TSL2651();
//	  		  		selectExternalTimerOperation = 1;
//	  		  		break;
//	  		  	case 1:
//	  		  		selectExternalTimerOperation = 2;
//	  		  		break;
//	  		  	case 2:
//	  		  		Disable_TSL2651();
//	  		  		selectExternalTimerOperation = 0;
//	  		  		unblockSleepMode(I2C_EM);
//	  		  		break;
//	  		  	}
//	  	#endif

  }

  	// Determines the duty cycle
  	handle = (LED0_TURNON_COUNT + LED0_DUTYCYCLE_COUNT) - handle;
  	LETIMER_CompareSet(LETIMER0,0,handle);

}


/*****************************************************
 * self_Clibration(): Performs self calibration
 * 1. Enables LETIMER0 for 1 sec using LFXO and captures TIMER count
 * 2. Enables LETIMER0 for 1 sec using ULFRCO and captures TIMER count
 *****************************************************/
void self_Clibration()
{
	uint16_t timer0_Count=0,timer1_Count=0;
	uint64_t lfx0_count=0, ulfrco_count=0;

	lfxo_selfcalib();
	Timer_Config();

	//letimerCount = LETIMER0->CNT;
	while(LETIMER0->CNT!=0)
	{
		//letimerCount = LETIMER0->CNT;
	}

	timer0_Count=TIMER0->CNT;
	timer1_Count=TIMER1->CNT;
	lfx0_count =(timer1_Count*65536)+timer0_Count;
	TIMER0->CNT=0;
	TIMER1->CNT=0;

	LETIMER_Enable(LETIMER0,false);
	LETIMER0->CNT=0;

	ulfrxo_selfcalib();

	//letimerCount = LETIMER0->CNT;
	while(LETIMER0->CNT!=0)
	{
		//letimerCount = LETIMER0->CNT;
	}

	timer0_Count=TIMER0->CNT;
	timer1_Count=TIMER1->CNT;
	ulfrco_count =(timer1_Count*65536)+timer0_Count;
	TIMER0->CNT=0;
	TIMER1->CNT=0;

	LETIMER0->CNT=0;
	ULF_TIMEPeriod = 1000 * ((float)lfx0_count/ulfrco_count);
	uint16_t ULF_TIMEPeriod_int = ULF_TIMEPeriod;

	if (ULF_TIMEPeriod > ULF_TIMEPeriod_int)
	{
		ULF_TIMEPeriod = ULF_TIMEPeriod_int ++;
	}

	CMU_ClockEnable(cmuClock_TIMER0, false);
	CMU_ClockEnable(cmuClock_TIMER1, false);
}




/*****************************************************
 * LETIMER_Config(): Configures LETIMER0
 * 1. Sets count value for EM3 and the remaining modes
 * 2. Configures  the necessary registers for LETIMER0
 * 3. Starts the LETIMER0
 *****************************************************/
void LETIMER_Config()
{
	   blockSleepMode(ENERGY_MODE);  // Ensuring that board never enters EM4
	  // Select LETIMER0 parameters
	  LETIMER_Init_TypeDef letimerInit = LETIMER_INIT_DEFAULT;

	  // Enabling LETIMER0 overflow interrupt
	  LETIMER_IntEnable(LETIMER0, LETIMER_IF_COMP0);

	  // Enabling LETIMER0 interrupt vector in NVIC
	  NVIC_EnableIRQ(LETIMER0_IRQn);

	#ifdef ENABLE_EM3
	  // Set LETIMER Top  - Comp0  / (1000 Hz )
	  // For 4ms Seconds Comp0 = 4
	  LETIMER_CompareSet(LETIMER0,0,handle);
	#else

	  // Set LETIMER Top - Comp0  / (16384 Hz)
	  // For 4ms Seconds Comp0 = 1638
	  LETIMER_CompareSet(LETIMER0,0,handle);
	#endif
	  // Configure TIMER
	  LETIMER_Init(LETIMER0, &letimerInit);

	  // COMP0 to be TOP Value in Free Run Mode
	  LETIMER0->CTRL |= LETIMER_CTRL_COMP0TOP;
}


int main(void)
{
	// Initializing the chip
	CHIP_Init();

	// Used for Self calibration
	#ifdef ENABLE_SELF_CALIB
	  self_Clibration();

	#endif

	// Used depending upon Energy mode
	#ifdef ENABLE_EM3
		LED0_TURNON_COUNT = LED0_TURNON_TIME * ULF_TIMEPeriod;
		LED0_DUTYCYCLE_COUNT   = LED0_DUTYCYCLE_TIME * ULF_TIMEPeriod-1;
	#else
		LED0_TURNON_COUNT = LED0_TURNON_TIME * LF_TIMEPeriod;
		LED0_DUTYCYCLE_COUNT = LED0_DUTYCYCLE_TIME * LF_TIMEPeriod-1;
	#endif

	handle = LED0_TURNON_COUNT; // determines the on period of LED

#ifdef CIRC_BUFFER_ENABLE
	cbuffer_init(&tx_buffer, CIRC_BUFFER_SIZE);
	cbuffer_init(&rx_buffer, CIRC_BUFFER_SIZE);
#endif

	Letimer_clock();  // Sets clock for LETIMER0
	Gpio_clock_config(); // Configures various GPIO pins

	leuart_init();
#ifdef CIRC_BUFFER_ENABLE
#ifdef CIRC_BUFF_BACK2BACK
	int g = 0;
	int8_t sendChar = 'a';
	for (g=0; g< 50; g++)
	{
		cbuffer_add(&tx_buffer, &sendChar);
	}

	for (g=0; g< 50; g++)
	{
		cbuffer_remove(&tx_buffer, &sendChar);
	}

#endif
#endif
	i2c_config();     // Configures I2C1 with SDA: PC4 SCL: PC5
	adc_config();
	/* Initialize LCD */
	SegmentLCD_Init(false);
	SegmentLCD_Write("LEUART");
#ifndef DMA_TURN_OFF
	dma_config();
#endif
	dma_leuart0_init();
#ifndef DISABLE_ONBOARD_PAS_LIGHTSENSOR
	ACMP_Config();    // Configures ACMP0
#endif
	LETIMER_Config(); // Configures LETIMER0

	while(1)
	{
	//Go to Energy Modes as set in the #define ENERGY_MODE
	  sleep();
	}
}

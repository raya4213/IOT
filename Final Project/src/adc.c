/*
 * adc.c
 *
 *  Created on: Sep 20, 2016
 *      Author: Rahul Yamasani
 */

#include "main.h"
#include "adc.h"
#include "em_adc.h"
/*****************************************************
 * adc_config(): Configures ADC0
 *
 * 1. Initializes ADC0
 * 2. Sets the parameters for singleinit
 *****************************************************/
void adc_config()
{
	CMU_ClockEnable(cmuClock_HFPER, true);     // Enables the HFPER clock
	CMU_ClockEnable(cmuClock_ADC0, true);      // Enables the clock to ADC0 pheripheral
	ADC_Init_TypeDef       adc0_init       = ADC_INIT_DEFAULT;
	ADC_InitSingle_TypeDef adc0_singleInit = ADC_INITSINGLE_DEFAULT;
	adc0_init.timebase = ADC_TimebaseCalc(ADC_Select_HFPER_Freq); // calculating the time base in accordance with HFPERCLK
	adc0_init.prescale = ADC_PrescaleCalc(ADCFreq, ADC_Select_HFPER_Freq); // AdcFreq set to 130000
	ADC_Init(ADC0, &adc0_init);

	adc0_singleInit.input = ADC_Singleinit_Input;      // Single Init input as temp sensor
	adc0_singleInit.reference = ADC_Singleinit_Ref;    // Reference: 1.25 VDD
	adc0_singleInit.resolution = ADC_Singleinit_Resol; // Resolution: 12 bit
	adc0_singleInit.acqTime = ADC_Singleinit_AcqTime;  // Acquisition time: 1 ADC cycle
#ifdef DMA_TURN_OFF

	#ifdef ADC_POLLING
		adc0_singleInit.rep = false;                        // Rep: false
	#else
		adc0_singleInit.rep = true;
		// Clearing the interrupts
		int intFlags;
		intFlags = ADC_IntGet(ADC0);
		ADC_IntClear(ADC0, intFlags);

		ADC0->IEN |= ADC_IF_SINGLE;
		NVIC_EnableIRQ(ADC0_IRQn);
	#endif


#else
	adc0_singleInit.rep = true;
#endif

	ADC_InitSingle(ADC0, &adc0_singleInit);
	//I2C_TransferInit(I2C0, &i2cTransfer);

}

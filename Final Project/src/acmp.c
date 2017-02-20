/*
 * acmp.c
 *
 *  Created on: Sep 20, 2016
 *      Author: Rahul Yamasani
 */

#include "main.h"
#include "em_acmp.h"
#include "acmp.h"

/*****************************************************
 * ACMP_Config(): Configures ACMP0
 * 1. Enables clock to the ACMP0 peripheral
 * 2. Configures  the necessary registers for LETIMER0
 * 3. Initializes  ACMP0
 *****************************************************/
void ACMP_Config()
{
	ACMP_Init_TypeDef acmpInit1  = {
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
	CMU_ClockEnable(cmuClock_ACMP0, true); // Enabling clock to ACMP0

	ACMP_Init(ACMP0, &acmpInit1);  // initializes the ACMP0

	 // CLEARING THE INTERRUPTS
	ACMP_Channel_TypeDef negSel =  acmpChannelVDD;
	ACMP_Channel_TypeDef posSel =  acmpChannel6;
	ACMP_ChannelSet(ACMP0,negSel,posSel);

	// waits till warmup of ACMP0 is finished
	while (!(ACMP0->STATUS & ACMP_STATUS_ACMPACT));

}


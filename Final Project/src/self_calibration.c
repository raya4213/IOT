/*
 * self_calibration.c
 *
 *  Created on: Sep 20, 2016
 *      Author: Rahul Yamasani
 */


/*
 * self_calibration.c
 *
 *  Created on: Sep 20, 2016
 *      Author: Rahul Yamasani
 */

#include "main.h"
#include "self_calibtarion.h"
//extern int ULF_TIMEPeriod;

/*****************************************************
 * Timer_Config(): Configures Timer 0 and Timer 1
 * 1. Enables clock to Timer 0 and Timer 1
 * 2. Configures  the necessary registers for Timer 0 and Timer 1
 * 3. Cascades Timer 0 and Timer 1
 * 3. Initializes  Timer 0 and Timer 1
 *****************************************************/
void Timer_Config()
{

	// Enabling clocks for Timer 0 and 1
	CMU_ClockEnable(cmuClock_TIMER0, true);
	CMU_ClockEnable(cmuClock_TIMER1, true);

	// Setting initializing parameters for Timer 0
	TIMER_Init_TypeDef timer0Init = TIMER_INIT_DEFAULT;

	// Setting initializing parameters for Timer 1
	TIMER_Init_TypeDef timer1Init = TIMER_INIT_DEFAULT;
	timer1Init.clkSel = timerClkSelCascade;
	timer1Init.sync = true;

	// Start Timer 0 and 1
	TIMER_Init(TIMER0, &timer0Init);
	TIMER_Init(TIMER1, &timer1Init);
}


/*****************************************************
 * lfxo_selfcalib(): Used for Self calibration
 * 1. Enables LETIMER0 for 1 sec period using LFXO clock
 * 2. Initializes the LETIMER0
 *****************************************************/
void lfxo_selfcalib()
{
	CMU_OscillatorEnable(cmuOsc_LFXO,true,true);
	CMU_ClockSelectSet(cmuClock_LFA, cmuSelect_LFXO);
	CMU_ClockEnable(cmuClock_CORELE, true);
	LETIMER_CompareSet(LETIMER0, 0, 32768);
	CMU_ClockEnable(cmuClock_LETIMER0, true);
	LETIMER_Init_TypeDef letimerInit = LETIMER_INIT_DEFAULT;
	LETIMER0->CNT=32768;
	LETIMER_Init(LETIMER0, &letimerInit);
}

/*****************************************************
 * ulfrxo_selfcalib(): Used for Self calibration
 * 1. Enables LETIMER0 for 1 sec period using ULFRCO clock
 * 2. Initializes the LETIMER0
 *****************************************************/
void ulfrxo_selfcalib()
{
	CMU_OscillatorEnable(cmuOsc_LFXO,false,false);
	CMU_OscillatorEnable(cmuOsc_ULFRCO,true,true);
	CMU_ClockSelectSet(cmuClock_LFA, cmuSelect_ULFRCO);
	LETIMER_CompareSet(LETIMER0, 0, 1000);
	LETIMER_Init_TypeDef letimerInit = LETIMER_INIT_DEFAULT;
	letimerInit.comp0Top = true;
	LETIMER0->CNT = 1000;
	LETIMER_Init(LETIMER0, &letimerInit);
}


// Header Files

#include <stdint.h>
#include <stdbool.h>
#include "em_device.h"
#include "em_chip.h"
#include "em_cmu.h"
#include "em_letimer.h"
#include "em_gpio.h"
#include "em_timer.h"
#include "em_acmp.h"
#include "sleep.h"

// Energy Modes
#define EM0 0
#define EM1 1
#define EM2 2
#define EM3 3

// User can change the following variables

#define ENERGY_MODE 		  EM3   // defines the energy mode
#define LED0_TURNON_TIME      0.004 // Turn on time for GPIO_D Pin 6
#define LED0_DUTYCYCLE_TIME   3
#define ENABLE_SELF_CALIB           // Enables Self calibration

int ULF_TIMEPeriod =  1000;         // Ultra low frequency clock period
#define LF_TIMEPeriod  16384        // Low frequency clock period


#define AMBI_High_Thres 0x3D
#define AMBI_Low_Thres  0x2
// Used for Energy mode EM3
#if ENERGY_MODE == EM3
	#define ENABLE_EM3
#endif

// Used for storing count values
int LED0_TURNON_COUNT;
int LED0_DUTYCYCLE_COUNT;

// Function Prototypes
void LED0_Operation();
void LETIMER0_IRQHandler(void);
void Letimer_clock();
void Gpio_clock_config();
void ACMP_Config();
void LETIMER_Config();

int handle;   // Used for setting count on  LETIMER0_CompareSet

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

/*****************************************************
 * LETIMER0_IRQHandler: Interrupt handler for LETIMER0
 *
 * 1.Handles the interrupts for every 4ms and 3 sec alternativelt
 * 2.Initializes the Ambient light sensor based on the comparator value
 *
 *****************************************************/
int handleAcmp = 2;
int checkOutput = 0;
void LETIMER0_IRQHandler(void)
{
  // Clearing the interrupts
  int intFlags;
  intFlags = LETIMER_IntGet(LETIMER0);
  LETIMER_IntClear(LETIMER0, intFlags);

  blockSleepMode(ENERGY_MODE);  // enters into desired energy mode

  // This loop is executed when 3 sec cycle starts
  if (handle == LED0_DUTYCYCLE_COUNT)
  {
	// Enabling Ambient light sensor for 4 ms
	GPIO_PinOutSet(gpioPortD,6);

	int acmpStatusbit = (ACMP0->STATUS & ACMP_STATUS_ACMPOUT)>>_ACMP_STATUS_ACMPOUT_SHIFT;

	// comparator value is 0
	if (acmpStatusbit == 0)
	{
		GPIO_PinOutSet(gpioPortE,2);   // Enabling the LED 2
		acmpInit.vddLevel = AMBI_High_Thres;      // Setting higher threshold
		ACMP_Init(ACMP0, &acmpInit);   // Iniatilizing the ACMP timer
		// Setting parameters for channel 6
		ACMP_Channel_TypeDef negSel =  acmpChannelVDD;  // selecting parameters for negative select
		ACMP_Channel_TypeDef posSel =  acmpChannel6;    // selecting parameters for positive select
		ACMP_ChannelSet(ACMP0,negSel,posSel);
	}

	// comparator value is 0
	else if (acmpStatusbit == 1)
	{
		GPIO_PinOutClear(gpioPortE,2); // Disabling the LED 2
		acmpInit.vddLevel = AMBI_Low_Thres;       // Setting low threshold
		ACMP_Init(ACMP0, &acmpInit);
		ACMP_Channel_TypeDef negSel =  acmpChannelVDD; // selecting parameters for negative select
		ACMP_Channel_TypeDef posSel =  acmpChannel6;   // selecting parameters for positive select
		ACMP_ChannelSet(ACMP0,negSel,posSel);
	}
	else
	{
		GPIO_PinOutClear(gpioPortD,6); // Clears the pin when 3 sec cycle starts
	}

  }

  	// Determines the duty cycle
  	handle = (LED0_TURNON_COUNT + LED0_DUTYCYCLE_COUNT) - handle;
  	LETIMER_CompareSet(LETIMER0,0,handle);

}

/*****************************************************
 * Letimer_Gpio_clock: Enables clock for LETIMER0
 * 1. ULFRCO for EM3
 * 2. LFX0 for other modes
 * 3. Sets the pre scalar value to 1 for LFX0
 *****************************************************/
void Letimer_clock()
{

#ifdef ENABLE_EM3
	// Enabling the External Low Frequency oscillator
	CMU_OscillatorEnable(cmuOsc_ULFRCO,true,true);

	// Select the Low Frequency A clock for the LETIMER0
	CMU_ClockSelectSet(cmuClock_LFA, cmuSelect_ULFRCO);
#else
	// Enabling the External Low Frequency oscillator
	CMU_OscillatorEnable(cmuOsc_LFXO,true,true);

	// Select the Low Frequency A clock for the LETIMER0
	CMU_ClockSelectSet(cmuClock_LFA, cmuSelect_LFXO);

	CMU->LFAPRESC0 |= 0x1<<8;

#endif

	// HFLE clock to enable access to Low Power Domain registers.
	CMU_ClockEnable(cmuClock_CORELE, true);

	// Enabling clock for LETIMER0 module
	CMU_ClockEnable(cmuClock_LETIMER0, true);


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

/*****************************************************
 * Gpio_clock_config(): Configures various GPIO pins
 * 1. Configures the LED 0 pin as pushpull
 * 2. Configures PORT D pin 6 in pushpull mode that excites ambient light sensor
 *****************************************************/
void Gpio_clock_config()
{
	// Enabling clock for GPIO module
	CMU_ClockEnable(cmuClock_GPIO, true);

	// Selecting the drive strength to lowest
	GPIO_DriveModeSet(gpioPortE, gpioDriveModeLowest);
	GPIO_DriveModeSet(gpioPortD, gpioDriveModeLowest);

	// Initializing LED 0 (Port E pin 2) as GPIO Push Pull
	GPIO_PinModeSet(gpioPortE,2,gpioModePushPull,0);

	// Initializing LED 1 (Port E pin 3) as GPIO Push Pull
	GPIO_PinModeSet(gpioPortE,3,gpioModePushPull,0);

	// Initializing Channel 6 (Port D pin 6) as GPIO Push Pull
	GPIO_PinModeSet(gpioPortD,6,gpioModePushPull,1);

	// Initializing Channel 6 (Port D pin 6) as GPIO Push Pull
	GPIO_PinModeSet(gpioPortC,6,gpioModeDisabled,0);

	//GPIO_PinOutClear(gpioPortD,6);


}

/*****************************************************
 * ACMP_Config(): Configures ACMP0
 * 1. Enables clock to the ACMP0 peripheral
 * 2. Configures  the necessary registers for LETIMER0
 * 3. Initializes  ACMP0
 *****************************************************/
void ACMP_Config()
{

	CMU_ClockEnable(cmuClock_ACMP0, true); // Enabling clock to ACMP0

	ACMP_Init(ACMP0, &acmpInit);  // initializes the ACMP0

	 // CLEARING THE INTERRUPTS
	ACMP_Channel_TypeDef negSel =  acmpChannelVDD;
	ACMP_Channel_TypeDef posSel =  acmpChannel6;
	ACMP_ChannelSet(ACMP0,negSel,posSel);

	// waits till warmup of ACMP0 is finished
	while (!(ACMP0->STATUS & ACMP_STATUS_ACMPACT));

}

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

/*****************************************************
 * self_Clibration(): Performs self calibration
 * 1. Enables LETIMER0 for 1 sec using LFXO and captures TIMER count
 * 2. Enables LETIMER0 for 1 sec using ULFRCO and captures TIMER count
 *****************************************************/
void self_Clibration()
{
	uint16_t timer0_Count=0,timer1_Count=0;
	uint64_t lfx0_count=0, ulfrco_count=0;
	uint16_t letimerCount = 0;

	lfxo_selfcalib();
	Timer_Config();

	letimerCount = LETIMER0->CNT;
	while(letimerCount!=0)
	{
		letimerCount = LETIMER0->CNT;
	}

	timer0_Count=TIMER0->CNT;
	timer1_Count=TIMER1->CNT;
	lfx0_count =(timer1_Count*65536)+timer0_Count;
	TIMER0->CNT=0;
	TIMER1->CNT=0;

	LETIMER_Enable(LETIMER0,false);
	LETIMER0->CNT=0;

	ulfrxo_selfcalib();

	letimerCount = LETIMER0->CNT;
	while(letimerCount!=0)
	{
		letimerCount = LETIMER0->CNT;
	}

	timer0_Count=TIMER0->CNT;
	timer1_Count=TIMER1->CNT;
	ulfrco_count =(timer1_Count*65536)+timer0_Count;
	TIMER0->CNT=0;
	TIMER1->CNT=0;

	LETIMER0->CNT=0;
	ULF_TIMEPeriod = 1000 * ((float)lfx0_count/ulfrco_count);
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
		LED0_DUTYCYCLE_COUNT = LED0_DUTYCYCLE_TIME * LF_TIMEPeriod;
	#endif


	handle = LED0_TURNON_COUNT; // determines the on period of LED


	Letimer_clock();  // Sets clock for LETIMER0
	Gpio_clock_config(); // Configures various GPIO pins

	ACMP_Config();    // Configures ACMP0
	LETIMER_Config(); // Configures LETIMER0



	while(1)
	{
	//Go to Energy Modes as set in the #define ENERGY_MODE
	  sleep();
	}
}

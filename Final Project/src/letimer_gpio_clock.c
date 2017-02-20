/*
 * letimer_gpio_clock.c
 *
 *  Created on: Sep 20, 2016
 *      Author: Rahul Yamasani
 */

#include "letimer_gpio_clock.h"
#include "em_letimer.h"
#include "em_cmu.h"
#include "em_gpio.h"
#include "main.h"

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

	CMU->LFAPRESC0 |= 0x2<<8;

#endif

	// HFLE clock to enable access to Low Power Domain registers.
	CMU_ClockEnable(cmuClock_CORELE, true);

	// Enabling clock for LETIMER0 module
	CMU_ClockEnable(cmuClock_LETIMER0, true);


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
	GPIO_PinModeSet(Light_Excite_Port,Light_Excite_Pin,gpioModePushPull,1);

	// Initializing Channel 6 (Port D pin 6) as GPIO Push Pull
	GPIO_PinModeSet(Light_Sensor_Port,Light_Sensor_Pin,gpioModeDisabled,0);

	// Initializing (Port C pin 5) as GPIO gpioModeWiredAndPullUpFilter
	// Open-drain output with filter and pullup
	GPIO_PinModeSet(I2C1_PORT,I2C1_SCL,gpioModeWiredAndPullUpFilter,1);

	// Initializing (Port C pin 4) as GPIO gpioModeWiredAndPullUpFilter
	// Open-drain output with filter and pullup
	GPIO_PinModeSet(I2C1_PORT,I2C1_SDA,gpioModeWiredAndPullUpFilter,1);

	// Setting up PC0 to indicate transfer direction
	GPIO_PinModeSet(I2C1_INTERRUPT_PORT, I2C1_VDD, gpioModePushPull, 0);

	// Setting up PC3 to indicate interrupt direction
	GPIO_PinModeSet(I2C1_INTERRUPT_PORT, I2C1_InterruptPin, gpioModeInput, 0);
}

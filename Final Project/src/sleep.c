/***************************************************************************//**
* @file sleep.c
*******************************************************************************
* @section License
* <b>(C) Copyright 2015 Silicon Labs, http://www.silabs.com</b>
*******************************************************************************
*
* Permission is granted to anyone to use this software for any purpose,
* including commercial applications, and to alter it and redistribute it
* freely, subject to the following restrictions:
*
* 1. The origin of this software must not be misrepresented; you must not
* claim that you wrote the original software.
* 2. Altered source versions must be plainly marked as such, and must not be
* misrepresented as being the original software.
* 3. This notice may not be removed or altered from any source distribution.
*
* DISCLAIMER OF WARRANTY/LIMITATION OF REMEDIES: Silicon Labs has no
* obligation to support this Software. Silicon Labs is providing the
* Software "AS IS", with no express or implied warranties of any kind,
* including, but not limited to, any implied warranties of merchantability
* or fitness for any particular purpose or warranties against infringement
* of any proprietary rights of a third party.
*
* Silicon Labs will not be liable for any consequential, incidental, or
* special damages, or any other relief, or for any claim by any third party,
* arising from your use of this Software.
*
* I would like to credit silicon labs for the following functions
* - void sleep(void)
* - void blockSleepMode(uint32_t minimumMode)
* - void unblockSleepMode(uint32_t minimumMode)
* I completely agree with the above Terms and conditions of silicon labs
******************************************************************************/

#include "em_emu.h"
#include "sleep.h"
#define NUM_SLEEP_MODES 4

uint32_t sleep_block_counter[NUM_SLEEP_MODES] = {0};
/*****************************************************
 * sleep: configures the sleep mode based on the #define
 *****************************************************/
void sleep(void)
{
	if (sleep_block_counter[0]>0){
		return;
	}

	else if (sleep_block_counter[1]>0){
		EMU_EnterEM1();
	}

	else if (sleep_block_counter[2]>0){
		EMU_EnterEM2(true);
	}

	else if (sleep_block_counter[3]>0){
		EMU_EnterEM3(true);
	}

	else{
		EMU_EnterEM4();
	}

}

/*****************************************************
 * blockSleepMode: blocks entering to energy mode greater than minimumMode
 *****************************************************/
void blockSleepMode(uint32_t minimumMode)
{
	INT_Disable();
	sleep_block_counter[minimumMode]++;
	INT_Enable();
}

/*****************************************************
 * blockSleepMode: unblocks entering to energy mode greater than minimumMode
 *****************************************************/
void unblockSleepMode(uint32_t minimumMode)
{
	INT_Disable();
	if (sleep_block_counter[minimumMode] > 0){
		sleep_block_counter[minimumMode]--;
	}
	INT_Enable();

}


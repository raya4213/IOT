

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

	else{
		EMU_EnterEM3(true);
	}

//	else if (sleep_block_counter[3]>0){
//		EMU_EnterEM3(true);
//	}

//	else{
//		EMU_EnterEM4();
//	}

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
//	int rahul = ACMP1->STATUS;
//	if (rahul == 1)
//		blockSleepMode(0);
}


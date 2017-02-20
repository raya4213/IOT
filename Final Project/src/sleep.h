/*
 * sleep.h
 *
 *  Created on: Sep 6, 2016
 *      Author: Rahul Yamasani
 */

#ifndef SLEEP_H_
#define SLEEP_H_
	#include "em_emu.h"
	#include "em_int.h"

	void sleep(void);
	void blockSleepMode(uint32_t minimumMode);
	void unblockSleepMode(uint32_t minimumMode);

#endif /* SLEEP_H_ */

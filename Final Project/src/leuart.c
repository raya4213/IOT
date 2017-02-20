/*
 * leuart.c
 *
 *  Created on: Oct 22, 2016
 *      Author: Rahul Yamasani
 */

#include "leuart.h"
#include "main.h"

void LEUART0_IRQHandler()
{
	int leuart0_getinterr = LEUART_IntGet(LEUART0);
	LEUART_IntClear(LEUART0, leuart0_getinterr);

}
void leuart_init()
{

	CMU_OscillatorEnable(cmuOsc_LFXO,true,true);
	CMU_ClockSelectSet(cmuClock_LFB, cmuSelect_LFXO);
	CMU_ClockEnable(cmuClock_LEUART0, true);    /* Enable LEUART1 clock */
	LEUART_Reset(LEUART0);

	LEUART_Init_TypeDef leuart0Init =
	{
	  .enable   = leuartDisable,       /* Activate data reception on LEUn_TX pin. */
	  .refFreq  = 0,                    /* Inherit the clock frequenzy from the LEUART clock source */
	  .baudrate = 9600,                 /* Baudrate = 9600 bps */
	  .databits = leuartDatabits8,      /* Each LEUART frame containes 8 databits */
	  .parity   = leuartNoParity,       /* No parity bits in use */
	  .stopbits = leuartStopbits1,      /* Setting the number of stop bits in a frame to 2 bitperiods */
	};

	LEUART_Init(LEUART0, &leuart0Init);
	LEUART_IntClear(LEUART0, _LEUART_IF_MASK);

	LEUART0->ROUTE = LEUART_ROUTE_RXPEN |LEUART_ROUTE_TXPEN| LEUART_ROUTE_LOCATION_LOC0;

	GPIO_PinModeSet(LEUART0_GPIO,LEUART0_TX_PIN,gpioModePushPull,1);
	GPIO_PinModeSet(LEUART0_GPIO,LEUART0_RX_PIN,gpioModePushPull,1);

	LEUART0->IEN |= LEUART_IEN_TXC;
	//LEUART0->CTRL |= LEUART_CTRL_LOOPBK;
	LEUART_Enable(LEUART0, leuartEnable);
	NVIC_EnableIRQ(LEUART0_IRQn);

//	while(1)
//		LEUART_Tx(LEUART0, 0x30);
}

void LEUART0_putch()
{
	//LEUART_Tx(LEUART0, 10);
	//LEUART0->ROUTE |= LEUART_ROUTE_LOCATION_LOC0;
}

/*
 * dma_leuart.c
 *
 *  Created on: Nov 12, 2016
 *      Author: Rahul Yamasani
 */

#include "main.h"
#include "dmactrl.h"
#include "dma_leuart.h"

//DMA_CB_TypeDef LEUART_cb;

void LEUART0dmaTransferDone(unsigned int channel, bool primary, void *user)
{
	(void) channel;
	(void) primary;
	(void) user;

	/* Disable DMA wake-up from LEUART0 TX */
	LEUART0->CTRL &= ~LEUART_CTRL_TXDMAWU;

}

void dma_leuart0_init()
{
	//CMU_ClockEnable(cmuClock_DMA, true); // may be required

	//DMA_Init_TypeDef dmaInit;
	/* Initializing the DMA */

	/*HPROT signal state when accessing the primary/alternate
	 * descriptors. Normally set to 0 if protection is not an issue.*/

	/*
	 *  *   This function will reset and prepare the DMA controller for use. Although
 *   it may be used several times, it is normally only used during system
 *   init. If reused during normal operation, notice that any ongoing DMA
 *   transfers will be aborted. When completed, the DMA controller is in
 *   an enabled state*/
//	dmaInit.hprot        = 0;
//	dmaInit.controlBlock = dmaControlBlock;// giving the parameters for control block
//	DMA_Init(&dmaInit);

	//  Setting up call-back function
	callBack[LEUART0_CHANNEL0].cbFunc  = LEUART0dmaTransferDone;   // Associate call back function here
	callBack[LEUART0_CHANNEL0].userPtr = NULL;                 // User defined pointer to provide with call back function
	callBack[LEUART0_CHANNEL0].primary = true;

	DMA_CfgChannel_TypeDef  dmachnlCfg;
	// Setting up channel
	dmachnlCfg.highPri   = false;                 // Default priority
	dmachnlCfg.enableInt = true;                  // interrupt shall be enabled for channel
	dmachnlCfg.select    = LEUART0_Dma_ChannelCfgSelect;  // DMA channel select for ADC0_SINGLE
	dmachnlCfg.cb        = &callBack[LEUART0_CHANNEL0];               // Associating the call back function with DMA channel

	DMA_CfgChannel(LEUART0_CHANNEL0, &dmachnlCfg);
	DMA_CfgDescr_TypeDef    dmadescrCfg;

	// Setting up channel descriptor
	dmadescrCfg.dstInc  = LEUART0_Dma_DestCfg_dstInc;      // destination address: 2 bytes
	dmadescrCfg.srcInc  = LEUART0_Dma_DestCfg_srcInc;      // Source address:
	dmadescrCfg.size    = LEUART0_Dma_DestCfg_size;        // data size: 2 bytes
	dmadescrCfg.arbRate = LEUART0_Dma_DestCfg_arbRate;     // arbitrate:  set to zero
	dmadescrCfg.hprot   = 0;
	DMA_CfgDescr(LEUART0_CHANNEL0, true, &dmadescrCfg);

	/* Set new DMA destination address directly in the DMA descriptor */
	dmaControlBlock->DSTEND = &LEUART0->TXDATA;
}

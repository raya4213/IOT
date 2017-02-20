/*
 * dma_leuart.h
 *
 *  Created on: Nov 12, 2016
 *      Author: Rahul Yamasani
 */

#ifndef DMA_LEUART_H_
#define DMA_LEUART_H_

#include <stdbool.h>
void LEUART0dmaTransferDone(unsigned int channel, bool primary, void *user);
void dma_leuart0_init();

#endif /* DMA_LEUART_H_ */

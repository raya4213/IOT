/*
 * circBuffer.h
 *
 *  Created on: Nov 1, 2016
 *      Author: Rahul Yamasani
 */

#ifndef CIRCBUFFER_H_
#define CIRCBUFFER_H_
#include <stdio.h>
#include <stdint.h>
typedef struct cbuffer
{
    uint8_t* start;
    uint8_t* end;
    uint8_t* head;
    uint8_t* tail;
	uint8_t size;
    uint8_t count;
} cbuffer;


/*initializes a circular buffer of size sz on the heap*/
void cbuffer_init(cbuffer* buff, uint8_t size);


/*free a circular buffer from memory*/
void cbuffer_free(cbuffer* buff);


/*adds one element to the buffer*/
void cbuffer_add(cbuffer* buff, uint8_t* elem);


/*removes one element from the buffer*/
void cbuffer_remove(cbuffer* buff, uint8_t* elem);

#endif /* CIRCBUFFER_H_ */

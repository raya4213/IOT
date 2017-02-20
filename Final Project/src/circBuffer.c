/*
 * circBuffer.c
 *
 *  Created on: Nov 1, 2016
 *      Author: Rahul Yamasani
 */

#include "circBuffer.h"
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <malloc.h>

void cbuffer_init(cbuffer* buff, uint8_t buff_size)
{
    if(buff_size == 0)
    {
    	buff_size = 1;
    }
    buff->start = (uint8_t*)(malloc(buff_size));
    buff->end = buff->start + buff_size - 1;
    buff->head = buff->start;
    buff->tail = buff->start;
    buff->size = buff_size;
    buff->count = 0;
    return;
}


void cbuffer_free(cbuffer* buff)
{
    free(buff->start);
    return;
}


void cbuffer_add(cbuffer* buff, uint8_t* elem)
{
    if(buff->count == buff->size)
    	buff->count = 0;
        //return;
    if(buff->head == buff->end)
    {
        memcpy(buff->head, elem, sizeof(uint8_t));
        buff->head = buff->start;
    }
    else
    {
        memcpy(buff->head++, elem, sizeof(uint8_t));
    }
    buff->count++;
    return;
}


void cbuffer_remove(cbuffer* buff, uint8_t* elem)
{
    if(buff->count == 0)
    	return;
    if(buff->tail == buff-> end)
    {
        memcpy(elem, buff->tail, sizeof(uint8_t));
        buff->tail = buff->start;
    }
    else
    {
        memcpy(elem, buff->tail++, sizeof(uint8_t));
    }
    buff->count--;
    return;
}


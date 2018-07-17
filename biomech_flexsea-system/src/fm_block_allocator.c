/****************************************************************************
	[Project] FlexSEA: Flexible & Scalable Electronics Architecture
	[Sub-project] 'flexsea-manage' Mid-level computing, and networking
	Copyright (C) 2016 Dephy, Inc. <http://dephy.com/>

	This program is free software: you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation, either version 3 of the License, or
	(at your option) any later version.

	This program is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with this program.  If not, see <http://www.gnu.org/licenses/>.
*****************************************************************************
	[Lead developper] Jean-Francois (JF) Duval, jfduval at dephy dot com.
	[Origin] Based on Jean-Francois Duval's work at the MIT Media Lab
	Biomechatronics research group <http://biomech.media.mit.edu/>
	[Contributors]
*****************************************************************************
	[This file] fm_block_allocator: Fixed sized block allocator
*****************************************************************************
	[Change log] (Convention: YYYY-MM-DD | author | comment)
	* 2017-01-20 | igutekunst | Initial implementation
	*
*****************************************************************************/

#ifdef __cplusplus
extern "C" {
#endif


#include "../inc/fm_block_allocator.h"
#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <stdlib.h>
//#include <stm32f427xx.h>	//ToDo remove or isolate with #ifdef
//#include <core_cm4.h>		//ToDo remove or isolate with #ifdef
#include <stdio.h>

struct Block {
	char data[FM_BLOCK_SIZE];
	struct Block * next;
	struct Block * prev;
	size_t index;

};

MsgQueue slave_queue;

void fail(void) {
	printf("failed\n");
}

uint8_t isInterrupt()
{
	//return (SCB->ICSR & SCB_ICSR_VECTACTIVE_Msk) != 0 ;	//ToDo remove or isolate with #ifdef
	return 0;
}

typedef struct Block Block;

Block memory_pool[FM_NUM_BLOCKS];

#define fm_offsetof(st, m) ((size_t)(&((st *)0)->m))

static Block* get_block(void * raw_block) {
	uint32_t p =  ((uint32_t) raw_block);
	if ((p & 0x3) != 0)
		fail();
	size_t offset = fm_offsetof(Block, data);
	Block* b = raw_block - offset;
	return b;
}

static Block* first;

int getIndexOfBlock(void* b)
{
	int i;
	for(i = 0; i < FM_NUM_BLOCKS; i++)
	{
		if(b == &memory_pool[i])
			return i;
	}
	return -1;
}
void printUnallocatedBlockList()
{
	if(first == NULL)
	{
		printf("Block List Empty\n");
		return;
	}

	Block* iterator = first;
	int count = 0;
	while(iterator && count < 2*FM_NUM_BLOCKS)
	{
		printf("%d -> ", getIndexOfBlock(iterator));
		iterator = iterator->next;
		count++;
	}
	printf("\n");
	if(count > FM_NUM_BLOCKS)
	{
		printf("Circular list detected ,\n");
	}
	return;
}

void printQueue(MsgQueue* q)
{
	printf("Printing Queue: \n");
	if(q == NULL)
	{
		printf("Null Queue\n");
		return;
	}
	
	Block* iterator = (q->tail);
	printf("  tail -> ");
	int count = 0;
	
	while(iterator && count < 2*FM_NUM_BLOCKS)
	{
		printf("%d -> ", getIndexOfBlock(iterator));
		iterator = iterator->prev;
		count++;
	}
	printf("\n");

	printf("  head -> ");
	int count2 = 0;
	iterator = (q->head);
	while(iterator && count2 < 2*FM_NUM_BLOCKS)
	{
		printf("%d -> ", getIndexOfBlock(iterator));
		iterator = iterator->next;
		count2++;
	}
	printf("\n");

	if(count > FM_NUM_BLOCKS || count2 > FM_NUM_BLOCKS)
	{
		printf("Detected circular queue\n");
	}
	return;
}

void fm_pool_init() 
{
	// all prev pointers are set to NULL because we
	// dont need them for allocations
	size_t i;
	for (i = 0; i < FM_NUM_BLOCKS - 1; i++) {
		memory_pool[i].index = i;
		memory_pool[i].prev = NULL;
		memory_pool[i].next = &memory_pool[i+1];
	}
	memory_pool[i].next = NULL;
	memory_pool[i].prev = NULL;

	first = &memory_pool[0];
}
/**
 * Allocate and return void* to a block of
 * memory of size FM_BLOCK_SIZE, if there are
 * remaining blocks in the pool.
 *
 * You must call fm_pool_init() before using this.
 *
 * @return void* to block of memory, or NULL
 * if no space is left.
 */
void* fm_pool_allocate_block(void)
{
	ATOMIC_BEGIN();
	Block* new_block = first;
	if (isInterrupt())
	{
		fail();
	}


	if (first != NULL)
		first = first->next;
	ATOMIC_END();
	if (new_block)
	{
		new_block->next = NULL;
		new_block->prev = NULL;

		return new_block->data;
	}
	return NULL;
}


int fm_pool_free_block(void* raw_data) {
	if (raw_data != NULL)
	{
		if (isInterrupt()) {
			fail();
		}
		Block* block =  get_block(raw_data);
		
		//check if block isn't currently allocated
		if(block->next != NULL || block->prev != NULL || block == first)
		{
			return -1;
		}

		if(block)
		{
			block->next = first;
		}

		if(first) 
			first->prev = block;
		
		first = block;
		ATOMIC_END();
		block->prev = NULL;

		return 0;
	}
	return -1;
}


int fm_queue_init(MsgQueue* q, size_t max_size)
{
	if (q == NULL)
		return -1;

	if (max_size <= 0)
		return -1;

	q->max_size = max_size;
	q->size = 0;
	q->head = NULL;
	q->tail = NULL;
		return 0;
}


int fm_queue_put(MsgQueue* q, void* item) {
	if (q == NULL || item == NULL)
		return -1;
	if (isInterrupt())
	{
		fail();
	}


	ATOMIC_BEGIN();
	if (q->size >= q->max_size)
	{
		ATOMIC_END();
		return -1;
	}

	Block** head = (Block**)&q->head;
	Block** tail = (Block**)&q->tail;

	Block* block =  get_block(item);

	//check if block has been allocated and is not already in queue
	if(block->next || block->prev || block == first || block == (Block*)(q->tail) || block == (Block*)(q->head))
		return -1;

	block->next = *head;
	if (block  == (Block*)(0x2609343) ) {
		fail();
	}
	if (*head != NULL)
	{
		(*head)->prev = block;
	}
	*head = block;
	q->size++;

	if (q->size == 1)
		*tail = block;
	ATOMIC_END();
	return 0;
}

int fm_queue_put_tail(MsgQueue* q, void * item) {
	if (q == NULL || item == NULL)
		return -1;
	if (isInterrupt())
	{
		fail();
	}

	ATOMIC_BEGIN();
	if (q->size >= q->max_size)
	{
		ATOMIC_END();
		return -1;
	}


	Block** head = (Block**)&q->head;
	Block** tail = (Block**)&q->tail;

	Block* block =  get_block(item);

	if ((*tail) == (Block*)0x2609343 )
		fail();

	block->prev = *tail;
	block->next = NULL;

	if (*tail != NULL) {
		(*tail)->next = block;
	}
	*tail = block;
	q->size++;

	if (q->size == 1)
		*head = block;
	ATOMIC_END();
	return 0;
}

void* fm_queue_get(MsgQueue* q ) {
	if (q == NULL)
		return NULL;
	if (isInterrupt())
	{
		fail();
	}

	if(q->size < 0)
	{
		printf("Q size unexpectely less than 0\n");
		q->size = 0;
	}

	if (q->size == 0)
	{
		ATOMIC_END();
		return NULL;
	}


	Block** head = (Block**)&q->head;
	Block** tail = (Block**)&q->tail;

	Block* item = *tail;

	if ((*tail) == (Block*)0x2609343 )
		fail();

	if ((*tail)->prev == (Block*)0x2609343) {
		fm_queue_init(&slave_queue, 10);
		fail();
		return NULL;
	}

	q->size--;
	// *tail shouldn't be NULL
	// since the size was > 0
	*tail = (*tail)->prev;

	if (q->size == 0)
		*head = NULL;

	if(item->next)
		item->next->prev = NULL;

	if(item->prev)
		item->prev->next = NULL;

	item->prev = NULL;
	item->next = NULL;

	return item->data;
}

#ifdef __cplusplus
}
#endif


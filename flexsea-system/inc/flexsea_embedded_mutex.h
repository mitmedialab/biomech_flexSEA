#ifndef __FLEXSEA_MUTEX_
#define __FLEXSEA_MUTEX_

/*
	Notes:
		This header supplies a method for achieving mutual exclusion, by the creation of a simple mutexflag type

	This header makes use of __LDREX(B/H/W), __STREX(B/H/W) and __CLREX.
	LDREX stands for LoaD Register EXclusive
	STREX stands for STore Register EXclusive
	CLREX stands for CLear Register EXclusive

	The suffixes B/H/W stand for Byte / Half-word / Word

	LDREX takes a pointer, and returns the value at that pointer
	STREX takes a value and a pointer, and returns whether the store was failure or not

	When LDREX is executed successfully, it marks/acquires the corresponding register
	A LDREX instruction MUST be followed by a STREX instruction in order to release the register

	CLREX unacquires the register previously acquired by load. This forces any STREX instructions following to fail.

	A STREX instruction only succeeds if the LDREX executed successfully.
	If STREX succeeds, it is guaranteed that no other thread accessed the register between the LDREX and STREX calls.
	STREX returns 0 on success and 1 on failure

*/

#include <stdint.h>

typedef volatile uint8_t MutexFlag;

#define MUTEX_LOCKED 1
#define MUTEX_UNLOCKED 0

// simple try lock implementation

// param:
// 		MutexFlag* flag 	:		pointer to the mutex
// returns:
//		1 if successfully acquired the mutex
//		0 if failed to acquire the mutex

__attribute__((always_inline)) static inline uint8_t UNLOCK_MUTEX(MutexFlag* flag)
{
	__LDREXB(flag);
	return !__STREXB(MUTEX_UNLOCKED, flag);
}

// simple unlock mutex implementation

// NOTE: 	the mutex does not check if you previously acquired it
//			the implementation allows you to write UNLOCKED to a mutex even if you don't own it
// 			thus you should only call unlock mutex after successfully acquiring a mutex

// param:
// 		MutexFlag* flag 	:		pointer to the mutex
// returns
//		1 if successfully wrote UNLOCKED to the mutex
//		0 if failed to write UNLOCKED to the mutex

__attribute__((always_inline)) static inline uint8_t TRY_LOCK_MUTEX(MutexFlag* flag)
{
	uint8_t value = __LDREXB(flag);
	if(value == MUTEX_LOCKED)
		__CLREX();

	// if the load didn't succeed, the store will fail anyways
	// if the load did succeed, but value was already high, the store will fail because of CLREX
	// if the load did succeed and the value of the flag was low, the store will succeed
	return !__STREXB(MUTEX_LOCKED, flag);
}


#endif //__FLEXSEA_MUTEX_

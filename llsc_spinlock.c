#include "spin_lock.h"

void spin_lock( spinlock_t *lock)
{
    unsigned int tmp;

	asm volatile(
	"	sevl\n"
	"1:	wfe\n"

	"2:	ldaxr	%w0, %1\n"
	"	cbnz	%w0, 1b\n"
	"	stxr	%w0, %w2, %1\n"
	"	cbnz	%w0, 2b\n"
	: "=&r" (tmp), "+Q" (lock->lock)
	: "r" (1)
	: "cc", "memory");
}

void spin_unlock( spinlock_t *lock)
{
	asm volatile(
	"	stlr	%w1, %0\n"
	: "=Q" (lock->lock) : "r" (0) : "memory");	
}


void spin_lock_init(spinlock_t *lock)
{
	lock->lock=0;
}

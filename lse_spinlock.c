
#include "ticket_spinlock.h"


 void spin_lock(spinlock_t *lock)
{
	unsigned int tmp;
	spinlock_t value, new;

	asm volatile(
     /* get next ticket */
"	mov	%w2, %w5\n"
"	ldadda	%w2, %w0, %3\n"

    /*
	 * need to use load exclusive so that it could exit WFE standby mode when 
	 * the CPU that holds the lock does release the lock by an normal write
	*/
"	sevl\n"
"standby:	wfe\n"
"	ldaxrh	%w2, %4\n"
"	eor	%w1, %w2, %w0, lsr #16\n"
"	cbnz	%w1, standby\n"
	
"end:"
	: "=&r" (value), "=&r" (new), "=&r" (tmp), "+Q" (*lock)
	: "Q" (lock->owner), "I" (1 << TICKET_BIT_SHIFT)
	: "memory");
}


void spin_unlock(spinlock_t *lock)
{
	unsigned long tmp;

	asm volatile(
	"mov	%w1, #1\n"
	"staddlh	%w1, %0\n"

	: "=Q" (lock->owner), "=&r" (tmp)
	:
	: "memory");
}

void spin_lock_init(spinlock_t *lock)
{
    lock->owner=0;
    lock->next=0;
}

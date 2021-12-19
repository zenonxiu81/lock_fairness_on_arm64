/**
 * ticket_spinlock.c
 * Copyright (C) 2021 Zenon Xiu
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 **/

#include "ticket_spinlock.h"


 void spin_lock(spinlock_t *lock)
{
	unsigned int tmp;
	spinlock_t value, new;

	asm volatile(
	/* try to get next ticket */
"try_get_tick:	\n"
"   ldaxr	%w0, %3 \n"
"	add	%w1, %w0, %w5 \n"
"	stxr	%w2, %w1, %3 \n"
"	cbnz	%w2, try_get_tick \n"

	/* my turn? */
"	eor	%w1, %w0, %w0, ror #16 \n"
"	cbz	%w1, end \n"
	
	/* go to WFE standby mode if no luck*/
"	sevl \n"
"standby:	wfe \n"
   
    /*waken up by other CPU, check my ticket again to see if it is my turn
	 * need to use load exclusive so that it could exit WFE standby mode when 
	 * the CPU that holds the lock does release the lock by an normal write
	*/
"	ldaxrh	%w2, %4 \n"
"	eor	%w1, %w2, %w0, lsr #16 \n"
"	cbnz	%w1, standby \n"
	/* We got the lock. Critical section starts here. */
"end:"
	: "=&r" (value), "=&r" (new), "=&r" (tmp), "+Q" (*lock)
	: "Q" (lock->owner), "I" (1 << TICKET_BIT_SHIFT)
	: "memory");
}


void spin_unlock(spinlock_t *lock)
{
	unsigned long tmp;

	asm volatile(
	/* LL/SC */
	"	ldrh	%w1, %0\n"
	"	add	%w1, %w1, #1\n"
	"	stlrh	%w1, %0"

	: "=Q" (lock->owner), "=&r" (tmp)
	:
	: "memory");
}

void spin_lock_init(spinlock_t *lock)
{
	lock->owner=0;
    lock->next=0;
}

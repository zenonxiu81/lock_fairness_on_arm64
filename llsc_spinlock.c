/**
 * llsc_spinlock.c
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

#ifndef BPSLAB_ATOMIC_H
#define BPSLAB_ATOMIC_H

#include <stdint.h>

inline int atomic_exch(int32_t old_value, int32_t new_value, volatile int32_t *ptr)
{
	int32_t prev;
	__asm__ __volatile__ ("lock; cmpxchgl %1, %2"
						  : "=a" (prev)
						  : "q" (new_value), "m" (*ptr), "0" (old_value)
						  : "memory");
	return prev != old_value;
}

inline int32_t atomic_or(int32_t value, volatile int32_t *ptr)
{
	int32_t prev, status;
	do {
		prev = *ptr;
		status = atomic_exch(prev, prev | value, ptr);
	} while (__builtin_expect(status != 0, 0));
	return prev;
}

inline int32_t atomic_add(int32_t increment, volatile int32_t *ptr)
{
	__asm__ __volatile__ ("lock; xaddl %0, %1"
						  : "+r" (increment), "+m" (*ptr)
						  :
						  : "memory");
	return increment;
}

inline int32_t atomic_inc(volatile int32_t *addr)
{
	return atomic_add(1, addr);
}

inline int32_t atomic_dec(volatile int32_t *addr)
{
	return atomic_add(-1, addr);
}

#endif // BPSLAB_ATOMIC_H

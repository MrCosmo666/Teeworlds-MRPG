#ifndef BASE_TL_THREADING_H
#define BASE_TL_THREADING_H

#include "../system.h"

/*
	atomic_inc - should return the value after increment
	atomic_dec - should return the value after decrement
	atomic_compswap - should return the value before the eventual swap
	sync_barrier - creates a full hardware fence
*/

#if defined(__GNUC__)

	inline unsigned atomic_inc(volatile unsigned *pValue)
	{
		return __sync_add_and_fetch(pValue, 1);
	}

	inline unsigned atomic_dec(volatile unsigned *pValue)
	{
		return __sync_add_and_fetch(pValue, -1);
	}

	inline unsigned atomic_compswap(volatile unsigned *pValue, unsigned comperand, unsigned value)
	{
		return __sync_val_compare_and_swap(pValue, comperand, value);
	}

	inline void sync_barrier()
	{
		__sync_synchronize();
	}

#elif defined(_MSC_VER)
	#include <intrin.h>

	#define WIN32_LEAN_AND_MEAN 1
	#include <windows.h>

	inline unsigned atomic_inc(volatile unsigned *pValue)
	{
		return _InterlockedIncrement((volatile long *)pValue);
	}

	inline unsigned atomic_dec(volatile unsigned *pValue)
	{
		return _InterlockedDecrement((volatile long *)pValue);
	}

	inline unsigned atomic_compswap(volatile unsigned *pValue, unsigned comperand, unsigned value)
	{
		return _InterlockedCompareExchange((volatile long *)pValue, (long)value, (long)comperand);
	}

	inline void sync_barrier()
	{
		MemoryBarrier();
	}
#else
	#error missing atomic implementation for this compiler
#endif

#if defined(CONF_PLATFORM_MACOSX)
/*
	use semaphore provided by SDL on macosx
*/
#else
class semaphore
{
	SEMAPHORE sem;
public:
	semaphore() { sphore_init(&sem); }
	~semaphore() { sphore_destroy(&sem); }
	semaphore(const semaphore&) = delete;
	void wait() { sphore_wait(&sem); }
	void signal() { sphore_signal(&sem); }
};
#endif

class lock
{
	LOCK var;

public:
	lock()
	{
		var = lock_create();
	}

	~lock()
	{
		lock_destroy(var);
	}

	lock(const lock&) = delete;

	void take() { lock_wait(var); }
	void release() { lock_unlock(var); }
};

class scope_lock
{
	lock *var;
public:
	scope_lock(lock *l)
	{
		var = l;
		var->take();
	}

	~scope_lock()
	{
		var->release();
	}

	scope_lock(const scope_lock&) = delete;
};

#endif // BASE_TL_THREADING_H
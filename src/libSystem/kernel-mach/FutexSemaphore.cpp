#include "FutexSemaphore.h"
#include "Futex.h"
#include <linux/futex.h>
#include "../../util/log.h"

Darling::FutexSemaphore::FutexSemaphore(int value)
: m_counter(value), m_waiting(0)
{
}

void Darling::FutexSemaphore::signal()
{
	if (__sync_fetch_and_add(&m_counter, 1) == 0)
		Futex::futex(&m_counter, FUTEX_WAKE, 1);
}

void Darling::FutexSemaphore::signalAll()
{
	if (m_waiting > 0)
	{
		int count = m_waiting;
		
		m_counter = count;
		//LOG << "Signalling " << count << " threads\n";
		Futex::futex(&m_counter, FUTEX_WAKE, count);
	}
}

void Darling::FutexSemaphore::wait()
{
	while (true)
	{
		int value = m_counter;
		if (value > 0)
		{
			//LOG << "Value is " << value << ", trying to decrement\n";
			if (__sync_bool_compare_and_swap(&m_counter, value, value-1))
				break;
		}
		else
		{
			__sync_fetch_and_add(&m_waiting, 1);
			Futex::futex(&m_counter, FUTEX_WAIT, value);
			__sync_fetch_and_sub(&m_waiting, 1);
		}
	}
}



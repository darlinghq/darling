#include "config.h"
#include "Futex.h"
#include <unistd.h>
#include <sys/syscall.h>
#include <sys/time.h>
#include <linux/futex.h>
#include <cassert>
#include "../../util/log.h"

Darling::Futex::Futex()
: m_lock(1), m_owner(0), m_readyToHandOff(0), m_handOffTarget(0)
{
}

void Darling::Futex::acquire()
{
	if (m_owner == gettid())
		LOG << "Deadlock will now happen in Darling::Futex::acquire()\n";
	while (true)
	{
		int result = __sync_sub_and_fetch(&m_lock, 1);
		if (result < 0)
		{
			// contention case
			LOG << gettid() << ": Contention with value " << result << std::endl;
			futex(&m_lock, FUTEX_WAIT, result);
		}
		else
		{
			LOG << gettid() << ": Lock acquired\n";
			m_owner = gettid();
			break;
		}
	}
}

void Darling::Futex::release()
{
	assert(m_owner == gettid());
	m_owner = 0;
	
	int result = __sync_add_and_fetch(&m_lock, 1);
	
	if (result != 1)
	{
		// contention case
		LOG << gettid() << ": Lock released, contended\n";
		m_lock = 1;
		futex(&m_lock, FUTEX_WAKE, 2);
	}
	else
		LOG << gettid() << ": Lock released\n";
}

Darling::Futex::TryAcquireResult Darling::Futex::try_acquire()
{
	if (m_owner == gettid())
		return ResultAlreadyOwned;
	
	bool success = __sync_bool_compare_and_swap(&m_lock, 1, 0);
	if (success)
		m_owner = gettid();
	
	return success ? ResultOK : ResultLocked;
}

void Darling::Futex::handoff()
{
	assert(m_owner == gettid());
	
	// Wait for the handoff target
	while (true)
	{
		pid_t target = m_handOffTarget;
		if (target != 0)
			break;
		else
			futex(&m_handOffTarget, FUTEX_WAIT, target);
	}
	
	// Let the target know we're here
	m_readyToHandOff = 1;
	m_owner = m_handOffTarget; // Pass the ownership to the target
	futex(&m_readyToHandOff, FUTEX_WAKE, 1);
}

bool Darling::Futex::handoff_accept()
{
	assert(m_owner != gettid());
	
	if (m_handOffTarget != 0)
		return false;
	
	// Let the owner know we're here
	m_handOffTarget = gettid();
	futex(&m_handOffTarget, FUTEX_WAKE, 1);
	
	// Wait for the owner
	while (true)
	{
		int ready = m_readyToHandOff;
		if (ready != 0)
			break;
		else
			futex(&m_readyToHandOff, FUTEX_WAIT, ready);
	}
	
	// Reset the variables
	m_handOffTarget = 0;
	m_readyToHandOff = 0;
	
	return true;
}

pid_t Darling::Futex::gettid()
{
	return syscall(SYS_gettid);	
}

int Darling::Futex::futex(int *uaddr, int op, int val, const struct timespec *timeout, int *uaddr2, int val3)
{
	return syscall(SYS_futex, uaddr, op, val, timeout, uaddr2, val3);
}


#ifndef LINUX_FUTEX_H
#define LINUX_FUTEX_H
#include <sys/types.h>

namespace Darling
{

class Futex
{
public:
	Futex();
	
	// Lock the futex.
	void acquire();
	
	// Unlock the futex.
	void release();
	
	// Try lock the futex
	enum TryAcquireResult { ResultOK = 0, ResultAlreadyOwned = 1, ResultLocked = 2 };
	TryAcquireResult try_acquire();
	
	// Transfer the lock ownership to another thread calling handoff_accept().
	void handoff();
	
	// Accept the lock ownership transfer from another thread calling handoff().
	// Returns false if there is another thread waiting for a handoff.
	bool handoff_accept();
	
	// Returns the (thread) PID that currently owns the futex or zero
	inline pid_t owner() const { return m_owner; }
	
	// Performs the gettid system call.
	static pid_t gettid();
	
	// Performs the futex system call.
	static int futex(int *uaddr, int op, int val, const struct timespec *timeout = nullptr, int *uaddr2 = nullptr, int val3 = 0);
private:
	int m_lock;
	pid_t m_owner;
	int m_readyToHandOff;
	pid_t m_handOffTarget;
};

}

#endif

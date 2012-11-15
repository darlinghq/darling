#ifndef FUTEXSEMAPHORE_H
#define FUTEXSEMAPHORE_H

namespace Darling {

class FutexSemaphore
{
public:
	FutexSemaphore(int value);
	void signal();
	
	// This seems to be inherently broken
	void signalAll();
	void wait();
private:
	int m_counter;
	int m_waiting;
};

}

#endif

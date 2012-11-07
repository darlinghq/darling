#ifndef TIMER_H
#define TIMER_H
#include <ctime>

class timer
{
public:
	void start();
	int stop();
private:
	time_t m_start;
};

#endif


#ifndef DYLD_TIMER_H
#define DYLD_TIMER_H
#include <time.h>

#define FLAGS_PRINT_TIME	0

class Timer
{
public:
	Timer();
	void start();
	void print(const char* name);
private:
	clock_t start_time;
};

#endif

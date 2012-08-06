#include "Timer.h"

Timer g_timer;

static void initGlobalTimer() __attribute__((constructor));

Timer::Timer()
	: start_time(0)
{
}

void Timer::start()
{
	if (FLAGS_PRINT_TIME)
		start_time = clock();
}

void Timer::print(const char* name)
{
	if (FLAGS_PRINT_TIME)
	{
		double elapsed = ((double)clock() - start_time) / CLOCKS_PER_SEC;
		printf("Elapsed time (%s): %f sec\n", name, elapsed);
	}
}

void initGlobalTimer()
{
	g_timer.start();
}

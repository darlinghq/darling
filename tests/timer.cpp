#include "timer.h"

void timer::start()
{
	m_start = ::time(nullptr);
}

int timer::stop()
{
	return ::time(nullptr) - m_start;
}


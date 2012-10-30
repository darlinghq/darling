#include "termcolor.h"
#include <unistd.h>
#include <iostream>

const bool isTty = isatty(1) != 0;

void termcolor::set(int text, int bg, int attrib)
{
	if (!isTty)
		return;

	std::cout << "\x1b[" << attrib << ';' << (text + 30) << ';' << (bg + 40) << 'm' << std::flush;
}


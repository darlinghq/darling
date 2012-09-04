#include "stlutils.h"

bool string_startsWith(const std::string& str, const std::string& what)
{
	if (str.size() < what.size())
		return false;
	else
		return str.compare(0, what.size(), what) == 0;
}

bool string_endsWith(const std::string& str, const std::string& what)
{
	if (str.size() < what.size())
		return false;
	else
		return str.compare(str.size()-what.size(), what.size(), what) == 0;
}


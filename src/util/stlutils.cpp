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

std::vector<std::string> string_explode(const std::string& str, char delim, bool keepEmpty)
{
	std::vector<std::string> rv;
	size_t start = 0, end;

	do
	{
		std::string substr;

		end = str.find(delim, start);
		substr = str.substr(start, (end != std::string::npos) ? end-start : std::string::npos);

		if (keepEmpty || !substr.empty())
			rv.push_back(substr);
		
		start = end+1;
	}
	while (end != std::string::npos);

	return rv;
}


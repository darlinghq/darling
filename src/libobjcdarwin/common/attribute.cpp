#include "attribute.h"
#include <cassert>

bool nextAttribute(objc_property_attribute_t& next, const char*& pos, std::vector<std::string>& strings)
{
	if (!*pos)
		return false;
	else if (*pos == ',')
		pos++;

	if (*pos == '"')
	{
		const char* start = ++pos;
		while (*pos != '"' && *pos)
			pos++;

		assert(*pos != '\0');

		strings.push_back(std::string(start, pos-start));
		next.name = strings.back().c_str();

		start = ++pos;
		while (*pos != ',' && *pos)
			pos++;

		strings.push_back(std::string(start, pos-start));
		next.value = strings.back().c_str();
	}
	else
	{
		strings.push_back(std::string(1, *pos));
		next.name = strings.back().c_str();
		const char* start = ++pos;

		while (*pos != ',' && *pos)
			pos++;

		strings.push_back(std::string(start, pos-start));
		next.value = strings.back().c_str();
	}

	return true;
}

#include "stlutils.h"
#include <algorithm>

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



void string_escape(std::string& str, const std::string& escape_chars)
{
    size_t found;

    found=str.find_first_of(escape_chars);
    while (found!=std::string::npos)
    {
      str.insert(found, "\\");
      found=str.find_first_of(escape_chars,found+2);
    }
}
void string_lstrip(std::string& str){
    str.erase(str.begin(), std::find_if(str.begin(), str.end(), std::not1(std::ptr_fun<int, int>(std::isspace))));
}
void string_replaceAll(std::string& target, const std::string oldstr, const std::string newstr) {
    unsigned int x = target.find(oldstr);
    while(x != std::string::npos) {
		target.erase(x, oldstr.length());
		target.insert(x, newstr);
        x = target.find(oldstr, x+newstr.length());
	}
}



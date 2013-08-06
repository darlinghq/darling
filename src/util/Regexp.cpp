#include "Regexp.h"
#include <stdexcept>

namespace Darling {

Regexp::Regexp(const std::string& re, bool caseInsensitive)
{
	int flags, rv;

	flags = REG_EXTENDED;

	if (caseInsensitive)
		flags |= REG_ICASE;

	rv = ::regcomp(&m_regex, re.c_str(), flags);
	if (rv)
		throwError(rv);
}

Regexp::~Regexp()
{
	regfree(&m_regex);
}

void Regexp::throwError(int code)
{
	char buffer[512];

	::regerror(code, &m_regex, buffer, sizeof(buffer));

	throw std::invalid_argument(buffer);
}

bool Regexp::matches(const std::string& input)
{
	int rv;
	regmatch_t matches[50];

	m_matches.clear();

	rv = ::regexec(&m_regex, input.c_str(), sizeof(matches)/sizeof(matches[0]), matches, 0);

	if (rv == REG_NOMATCH)
		return false;
	else if (rv == REG_NOERROR)
	{
		for (int i = 0; i < sizeof(matches)/sizeof(matches[0]) && matches[i].rm_so != -1; i++)
		{
			m_matches.push_back(input.substr(matches[i].rm_so, matches[i].rm_eo - matches[i].rm_so));
		}
		return true;
	}
	else
		throwError(rv);
}

}


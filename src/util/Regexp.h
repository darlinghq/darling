#ifndef REGEXP_H
#define REGEXP_H
#include <regex.h>
#include <string>
#include <vector>

namespace Darling {

class Regexp
{
public:
	Regexp(const std::string& re, bool caseInsensitive = false);
	~Regexp();

	bool matches(const std::string& input);

	inline size_t groups() const { return m_matches.size(); }
	inline std::string group(size_t index) const { return m_matches.at(index); }
private:
	void throwError(int code) __attribute__((noreturn));
private:
	regex_t m_regex;
	std::vector<std::string> m_matches;
};

} // namespace Darling

#endif


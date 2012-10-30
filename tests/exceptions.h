#ifndef __EXCEPTIONS_H
#define __EXCEPTIONS_H
#include <stdexcept>

class compile_error : public std::runtime_error
{
public:
	compile_error(const std::string& err)
		: std::runtime_error(err)
	{
	}
};

class different_output_error : public std::exception
{
public:
	different_output_error(const std::string& remote, const std::string& local)
		: m_remote(remote), m_local(local)
	{
	}

	virtual const char* what() const throw() override
	{
		return "different_output_error";
	}

	const std::string& local() const { return m_local; }
	const std::string& remote() const { return m_remote; }
private:
	std::string m_local, m_remote;
};

#endif



#ifndef __EXCEPTIONS_H
#define __EXCEPTIONS_H
#include <stdexcept>
#include <sstream>

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

class nonzero_exit_error : public std::exception
{
public:
	nonzero_exit_error(bool remote, const std::string& output)
		: m_remote(remote), m_output(output)
	{
		std::stringstream ss;
		ss << "Non-zero exit status from ";
		if (m_remote)
			ss << "remotely";
		else
			ss << "locally";
		ss << " run binary";
		m_msg = ss.str();
	}

	bool remote() const { return m_remote; }
	const std::string& output() const { return m_output; }

	virtual const char* what() const throw() override
	{
		return m_msg.c_str();
	}
private:
	bool m_remote;
	std::string m_output, m_msg;
};

#endif



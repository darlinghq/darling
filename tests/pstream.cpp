#include "pstream.h"
#include <cstdio>


pstream::pstream(FILE* pipe)
: m_pipe(pipe)
{
}

pstream::~pstream()
{
	if (m_pipe)
		wait();
}

pstream* pstream::popen(const std::string& cmd)
{
	FILE* pipe = ::popen(cmd.c_str(), "r");
	if (!pipe)
		return nullptr;
	return new pstream(pipe);
}

std::istream* pstream::in()
{
	return new std::istream(this);
}

int pstream::wait()
{
	int rv = pclose(m_pipe);
	m_pipe = nullptr;
	return rv;
}

std::streamsize pstream::xsgetn(char* s, std::streamsize n)
{
	return fread(s, 1, n, m_pipe);
}

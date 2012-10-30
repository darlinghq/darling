#include "libsshcxx/ssh_streambuf.h"
#include <cstdio>
#include <istream>
#include <string>

class pstream : public ssh_streambuf
{
public:
	pstream(FILE* pipe);
	~pstream();
	
	static pstream* popen(const std::string& cmd);
	
	std::istream* in();
	int wait();
protected:
	virtual std::streamsize xsgetn(char* s, std::streamsize n) override;
private:
	FILE* m_pipe;
};

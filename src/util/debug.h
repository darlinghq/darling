#ifndef UTIL_DEBUG_H
#define UTIL_DEBUG_H

#include <cassert>
#include <cstdio>
#include <iostream>

extern "C" bool g_loggingEnabled;

namespace Darling {

enum class DebugChannel { Error, Trace, Debug };

bool debugChannelEnabled(DebugChannel channel);


#define LOG  if (g_loggingEnabled) std::cerr
#define LOGF(...) if (g_loggingEnabled) fprintf(stderr, __VA_ARGS__)

void logTrace(const char* funcName, ...);

struct ArgName
{
	ArgName(const char* p) : name(p) {}
	const char* name;
};

template<typename T> void logPrint(T value)
{
	std::cerr << value;
}
template<> void logPrint<const char*>(const char* value);
template<> void logPrint<std::string>(std::string value);
template<> void logPrint<ArgName>(ArgName value);

class TraceHelper
{
public:
	TraceHelper(const char* funcName, DebugChannel chan, const char* file = nullptr, int line = 0);
	TraceHelper(DebugChannel chan);
	~TraceHelper();
	
	template<typename T> TraceHelper& operator << (T value)
	{
		if (m_print)
			logPrint(value);
		return *this;
	}

	template<typename T> TraceHelper& write(T value)
	{
		if (m_print)
			std::cerr << value;
		return *this;
	}
private:
	bool m_print;
};

#define ARG(a) (Darling::ArgName(" " #a "=")) << a
#define ARGP(a) (Darling::ArgName(" " #a "=")) << ((void*)a)
#define TRACE() Darling::TraceHelper(__PRETTY_FUNCTION__, Darling::DebugChannel::Trace, __FILE__, __LINE__)
#define TRACE1(a) TRACE() << ARG(a)
#define TRACE2(a,b) TRACE() << ARG(a) << ARG(b)
#define TRACE3(a,b,c) TRACE() << ARG(a) << ARG(b) << ARG(c)
#define TRACE4(a,b,c,d) TRACE() << ARG(a) << ARG(b) << ARG(c) << ARG(d)
#define TRACE5(a,b,c,d,e) TRACE() << ARG(a) << ARG(b) << ARG(c) << ARG(d) << ARG(e)
#define TRACE6(a,b,c,d,e,f) TRACE() << ARG(a) << ARG(b) << ARG(c) << ARG(d) << ARG(e) << ARG(f)

#define ERROR() Darling::TraceHelper(__PRETTY_FUNCTION__, Darling::DebugChannel::Error, __FILE__, __LINE__)
#define ERROR1(a) ERROR() << ARG(a)
#define ERROR2(a,b) ERROR() << ARG(a) << ARG(b)
#define ERROR3(a,b,c) ERROR() << ARG(a) << ARG(b) << ARG(c)
#define ERROR4(a,b,c,d) ERROR() << ARG(a) << ARG(b) << ARG(c) << ARG(d)
#define ERROR5(a,b,c,d,e) ERROR() << ARG(a) << ARG(b) << ARG(c) << ARG(d) << ARG(e)
#define ERROR6(a,b,c,d,e,f) ERROR() << ARG(a) << ARG(b) << ARG(c) << ARG(d) << ARG(e) << ARG(f)

#define STUB() ERROR().write(" STUB!")

} // namespace Darling

#endif


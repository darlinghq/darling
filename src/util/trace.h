#ifndef DARLING_TRACE_H
#define DARLING_TRACE_H
#include <iostream>

extern bool g_enableTrace;
void logTrace(const char* funcName, ...);
void enableTrace(bool enable);

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
	TraceHelper(const char* funcName);
	~TraceHelper();
	
	template<typename T> TraceHelper& operator << (T value)
	{
		logPrint(value);
		return *this;
	}
};

#define ARG(a) (ArgName(" " #a "=")) << a
#define ARGP(a) (ArgName(" " #a "=")) << ((void*)a)
#define TRACE() TraceHelper(__PRETTY_FUNCTION__)
#define TRACE(a) TRACE() << ARG(a)
#define TRACE(a,b) TRACE() << ARG(a) << ARG(b)
#define TRACE(a,b,c) TRACE() << ARG(a) << ARG(b) << ARG(c)
#define TRACE(a,b,c,d) TRACE() << ARG(a) << ARG(b) << ARG(c) << ARG(d)
#define TRACE(a,b,c,d,e) TRACE() << ARG(a) << ARG(b) << ARG(c) << ARG(d) << ARG(e)
#define TRACE(a,b,c,d,e,f) TRACE() << ARG(a) << ARG(b) << ARG(c) << ARG(d) << ARG(e) << ARG(f)

#endif

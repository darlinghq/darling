#include "trace.h"
#include <stdarg.h>
#include <iostream>

bool g_enableTrace = false;

TraceHelper::TraceHelper(const char* funcName)
{
	std::cerr << "TRACE(): " << funcName << "";
}
TraceHelper::~TraceHelper()
{
	std::cerr << std::endl;
}

void enableTrace(bool enable)
{
	g_enableTrace = enable;
}

template<> void logPrint<std::string>(std::string value)
{
	std::cerr << '\"' << value << '\"';
}
template<> void logPrint<const char*>(const char* value)
{
	std::cerr << '\"' << value << '\"';
}
template<> void logPrint<ArgName>(ArgName value)
{
	std::cerr << value.name;
}

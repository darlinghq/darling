#include "trace.h"
#include <stdarg.h>
#include <iostream>

TraceHelper::TraceHelper(const char* funcName)
{
	if (g_loggingEnabled)
		std::cerr << "TRACE(): " << funcName << "";
}
TraceHelper::~TraceHelper()
{
	if (g_loggingEnabled)
		std::cerr << std::endl << std::flush;
}

template<> void logPrint<std::string>(std::string value)
{
	if (g_loggingEnabled)
		std::cerr << '\"' << value << '\"';
}
template<> void logPrint<const char*>(const char* value)
{
	if (g_loggingEnabled)
		std::cerr << '\"' << value << '\"';
}
template<> void logPrint<ArgName>(ArgName value)
{
	if (g_loggingEnabled)
		std::cerr << value.name;
}

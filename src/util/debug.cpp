#include "debug.h"
#include <cstring>
#include <cstdlib>
#include "stlutils.h"

extern "C" bool g_loggingEnabled = false; // "debug" channel
static bool m_traceEnabled = false;
static bool m_errorEnabled = true;

__attribute__((constructor)) static void initLogging()
{
	const char* v = getenv("DYLD_DEBUG");
	if (v != nullptr)
	{
		std::vector<std::string> specs = string_explode(v, ',', false);
		for (std::string str : specs)
		{
			bool enable = true, none = false;

			if (str[0] == '+')
				enable = true;
			else if (str[0] == '-')
				enable = false;
			else
				none = true;

			if (!none)
				str = str.substr(1);

			if (str == "debug")
				g_loggingEnabled = enable;
			else if (str == "trace")
				m_traceEnabled = enable;
			else if (str == "error")
				m_errorEnabled = enable;
		}
	}
}

bool Darling::debugChannelEnabled(DebugChannel channel)
{
	switch (channel)
	{
		case DebugChannel::Debug:
			return g_loggingEnabled;
		case DebugChannel::Trace:
			return m_traceEnabled;
		case DebugChannel::Error:
			return m_errorEnabled;
		default:
			return false;
	}
}

static const char* shortenFileName(const char* file)
{
	const char* s = strrchr(file, '/');
	if (!s)
		return file;
	else
		return s+1;
}

Darling::TraceHelper::TraceHelper(const char* funcName, DebugChannel chan, const char* file, int line)
{
	m_print = debugChannelEnabled(chan);

	if (m_print)
	{
		switch (chan)
		{
			case DebugChannel::Debug:
				std::cerr << "DEBUG"; break;
			case DebugChannel::Trace:
				std::cerr << "TRACE"; break;
			case DebugChannel::Error:
				std::cerr << "ERROR"; break;
		}

		if (!file)
			std::cerr << " in " << funcName << ':';
		else
			std::cerr << " @" << shortenFileName(file) << ':' << line << " in " << funcName << ':';
	}
}

Darling::TraceHelper::TraceHelper(DebugChannel chan)
{
	m_print = debugChannelEnabled(chan);
}

Darling::TraceHelper::~TraceHelper()
{
	if (m_print)
		std::cerr << std::endl << std::flush;
}

template<> void Darling::logPrint<std::string>(std::string value)
{
	//if (g_loggingEnabled)
		std::cerr << '\"' << value << '\"';
}
template<> void Darling::logPrint<const char*>(const char* value)
{
	//if (g_loggingEnabled)
		std::cerr << '\"' << value << '\"';
}
template<> void Darling::logPrint<Darling::ArgName>(Darling::ArgName value)
{
	//if (g_loggingEnabled)
		std::cerr << value.name;
}


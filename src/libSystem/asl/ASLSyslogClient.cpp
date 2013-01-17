#include "ASLSyslogClient.h"
#include "ASLMsg.h"
#include <syslog.h>
#include <map>
#include <string>
#include "asl.h"

static const std::map<std::string, int> g_levels {
	std::make_pair<std::string,int>(ASL_STRING_EMERG, LOG_EMERG),
	std::make_pair<std::string,int>(ASL_STRING_ALERT, LOG_ALERT),
	std::make_pair<std::string,int>(ASL_STRING_CRIT, LOG_CRIT),
	std::make_pair<std::string,int>(ASL_STRING_ERR, LOG_ERR),
	std::make_pair<std::string,int>(ASL_STRING_WARNING, LOG_WARNING),
	std::make_pair<std::string,int>(ASL_STRING_NOTICE, LOG_NOTICE),
	std::make_pair<std::string,int>(ASL_STRING_INFO, LOG_INFO),
	std::make_pair<std::string,int>(ASL_STRING_DEBUG, LOG_DEBUG)
};

ASLSyslogClient::ASLSyslogClient(const char* ident, const char* facility, uint32_t opts)
{
	int sopts = 0;

	if (opts & ASL_OPT_STDERR)
		sopts |= LOG_PERROR;
	if (opts & ASL_OPT_NO_DELAY)
		sopts |= LOG_NDELAY;

	// This is totally poor man's implementation
	::openlog(ident, sopts, LOG_USER);
}

ASLSyslogClient::~ASLSyslogClient()
{
	::closelog();
}

bool ASLSyslogClient::log(ASLMsg* msg)
{
	const char* text = msg->get(ASL_KEY_MSG);
	const char* level = msg->get(ASL_KEY_LEVEL);
	int nlevel = LOG_ERR;

	if (!msg)
		return false;

	if (!level)
		level = ASL_STRING_ERR;

	// convert level string to int
	auto it = g_levels.find(level);
	if (it != g_levels.end())
		nlevel = it->second;

	syslog(nlevel, "%s", text);

	// TODO: use ASLFileClient to log to a file

	return true;
}

bool ASLSyslogClient::logAuxiliary(ASLMsg* msg, const char* title, const char* uti, const char* url)
{
	// TODO
	return false;
}

ASLResponse* ASLSyslogClient::search(ASLMsg* query)
{
	// Not supported
	return nullptr;
}


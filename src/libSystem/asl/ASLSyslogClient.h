#ifndef ASLSYSLOGCLIENT_H
#define ASLSYSLOGCLIENT_H
#include "ASLClient.h"
#include <stdint.h>

class ASLSyslogClient : public ASLClient
{
public:
	ASLSyslogClient(const char* ident, const char* facility, uint32_t opts);
	virtual ~ASLSyslogClient();

	virtual bool log(ASLMsg* msg) override;
	virtual bool logAuxiliary(ASLMsg* msg, const char* title, const char* uti, const char* url) override;
	virtual ASLResponse* search(ASLMsg* query) override;
private:
};

#endif


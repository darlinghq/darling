#ifndef ASLCLIENT_H
#define ASLCLIENT_H

class ASLMsg;
class ASLResponse;

class ASLClient
{
public:
	ASLClient();
	virtual ~ASLClient() {}
	virtual bool log(ASLMsg* msg) = 0;
	virtual bool logAuxiliary(ASLMsg* msg, const char* title, const char* uti, const char* url) = 0;
	virtual int setFilter(int filter);
	bool willBeFilteredOut(int level);
	virtual ASLResponse* search(ASLMsg* query) = 0;
protected:
	int m_filter;
};

#endif


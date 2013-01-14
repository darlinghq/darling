#ifndef ASLRESPONSE_H
#define ASLRESPONSE_H
#include "ASLMsg.h"
#include <vector>

class ASLResponse
{
public:
	// Takes ownership of these ASLMsg's
	ASLResponse(const std::vector<ASLMsg*>& data);
	~ASLResponse();
	ASLMsg* next();
private:
	std::vector<ASLMsg*> m_data;
	int m_pos;
};

#endif


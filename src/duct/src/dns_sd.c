#include <stddef.h>

// We don't have Bonjour (libsystem_dnssd) yet.

int DNSServiceCreateConnection(void** ptr)
{
	*ptr = NULL;
	return 1;
}

int DNSServiceQueryRecord()
{
	return 1;
}

int DNSServiceProcessResult()
{
	return 1;
}

int DNSServiceRefDeallocate()
{
	return 1;
}

int DNSServiceRefSockFD()
{
	return 1;
}


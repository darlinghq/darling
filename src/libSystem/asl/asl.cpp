#include "asl.h"
#include "ASLMsg.h"
#include "ASLClient.h"
#include "ASLSyslogClient.h"
#include "ASLResponse.h"
#include <cstdlib>

static const int DEFAULT_BUFFER_SIZE = 1024;
static const char* ASL_LEVELS[] = { ASL_STRING_EMERG, ASL_STRING_ALERT, ASL_STRING_CRIT, ASL_STRING_ERR,
	ASL_STRING_WARNING, ASL_STRING_NOTICE, ASL_STRING_INFO, ASL_STRING_DEBUG };

int _asl_lib_log(aslclient asl, uint32_t, aslmsg msg, const char *format, ...)
{
	va_list ap;
	int rv;

	va_start(ap, format);

	rv = asl_vlog(asl, msg, -1, format, ap);
	va_end(ap);

	return rv;
}

uint32_t _asl_evaluate_send(aslclient asl, aslmsg msg, int level)
{
	return !asl->willBeFilteredOut(level);
}

aslclient asl_open(const char *ident, const char *facility, uint32_t opts)
{
	return new ASLSyslogClient(ident, facility, opts);
}

void asl_close(aslclient asl)
{
	delete asl;
}

int asl_add_log_file(aslclient asl, int descriptor)
{
	return !asl->addLogFile(descriptor);
}

int asl_remove_log_file(aslclient asl, int descriptor)
{
	return !asl->removeLogFile(descriptor);
}

int asl_set_filter(aslclient asl, int f)
{
	return !asl->setFilter(f);
}

const char *asl_key(aslmsg msg, uint32_t n)
{
	return msg->get(n);
}

aslmsg asl_new(uint32_t type)
{
	return new ASLMsg(type);
}

int asl_set(aslmsg msg, const char *key, const char *value)
{
	return !msg->set(key, value);
}

int asl_unset(aslmsg msg, const char *key)
{
	return !msg->unset(key);
}

const char *asl_get(aslmsg msg, const char *key)
{
	return msg->get(key);
}

int asl_log(aslclient asl, aslmsg msg, int level, const char *format, ...)
{
	va_list ap;
	int rv;

	va_start(ap, format);

	rv = asl_vlog(asl, msg, level, format, ap);
	va_end(ap);

	return rv;
}

int asl_vlog(aslclient asl, aslmsg msg, int level, const char *format, va_list ap)
{
	char* buffer;
	int curSize = DEFAULT_BUFFER_SIZE;

	if (level > 7)
		return 1;

	buffer = new char[DEFAULT_BUFFER_SIZE];
	if (!buffer)
		return 1;

	while (true)
	{
		int needed = vsnprintf(buffer, curSize, format, ap);
		if (needed < curSize)
			break;

		curSize = needed+1;
		delete [] buffer;
		buffer = new char[curSize];

		if (!buffer)
			return 1;
	}

	ASLMsg msg2(*msg);

	msg2.set(ASL_KEY_MSG, buffer);
	free(buffer);

	if (level > 0) // non-standard feature to avoid code duplication
		msg2.set(ASL_KEY_LEVEL, ASL_LEVELS[level]);

	return !asl->log(&msg2);
}

int asl_send(aslclient asl, aslmsg msg)
{
	return !asl->log(msg);
}

void asl_free(aslmsg msg)
{
	delete msg;
}

int asl_set_query(aslmsg msg, const char *key, const char *value, uint32_t op)
{
	return !msg->set(key, op, value);
}

aslresponse asl_search(aslclient asl, aslmsg msg)
{
	return asl->search(msg);
}

aslmsg aslresponse_next(aslresponse r)
{
	return r->next();
}

void aslresponse_free(aslresponse r)
{
	delete r;
}

int asl_create_auxiliary_file(aslmsg msg, const char *title, const char *uti, int *out_descriptor)
{
	return 1; // TODO
}

int asl_close_auxiliary_file(int descriptor)
{
	return 1; // TODO
}

int asl_log_auxiliary_location(aslmsg msg, const char *title, const char *uti, const char *url)
{
	return 1; // TODO
}

aslclient asl_open_from_file(int descriptor, const char *ident, const char *facility)
{
	return nullptr; // TODO
}

int asl_log_descriptor(aslclient asl, aslmsg msg, int level, int descriptor, uint32_t fd_type)
{
	return 1; // TODO
}


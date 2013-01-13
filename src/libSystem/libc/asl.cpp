#include "asl.h"
#include <string>
#include <map>
#include <iostream>

struct __aslmsg : std::map<std::string, std::string>
{
};

int _asl_lib_log(aslclient asl, uint32_t eval, aslmsg msg, const char *format, ...)
{
	std::cerr << "_asl_lib_log" << std::endl;
	return 0; // TODO
}

uint32_t _asl_evaluate_send(aslclient asl, aslmsg msg, int level)
{
	std::cerr << "_asl_evaluate_send" << std::endl;
	return 0; // TODO
}

aslclient asl_open(const char *ident, const char *facility, uint32_t opts)
{
	return nullptr; // TODO
}

void asl_close(aslclient asl)
{
	// TODO
}

int asl_add_log_file(aslclient asl, int descriptor)
{
	std::cerr << "asl_add_log_file" << std::endl;
	return 0; // TODO
}

int asl_remove_log_file(aslclient asl, int descriptor)
{
	std::cerr << "asl_remove_log_file" << std::endl;
	return 0; // TODO
}

int asl_set_filter(aslclient asl, int f)
{
	return 0; // TODO
}

const char *asl_key(aslmsg msg, uint32_t n)
{
	auto i = msg->begin();
	while (n > 0)
	{
		if (i == msg->end()) return nullptr;
		i++;
	}
	return i->first.c_str();
}

aslmsg asl_new(uint32_t type)
{
	return new struct __aslmsg;
}

int asl_set(aslmsg msg, const char *key, const char *value)
{
	(*msg)[key] = (value) ? value : "";
	std::cerr << "asl_set \"" << key << "\" = \"" << (*msg)[key] << "\"" << std::endl;
	return 0;
}

int asl_unset(aslmsg msg, const char *key)
{
	return 0; // TODO
}

const char *asl_get(aslmsg msg, const char *key)
{
	auto i = msg->find(key);
	if (i == msg->end()) return nullptr;
	return i->second.c_str();
}

int asl_log(aslclient asl, aslmsg msg, int level, const char *format, ...)
{
	std::cerr << "asl_log" << std::endl;
	return 0; // TODO
}

int asl_vlog(aslclient asl, aslmsg msg, int level, const char *format, va_list ap)
{
	std::cerr << "asl_vlog" << std::endl;
	return 0; // TODO
}

int asl_send(aslclient asl, aslmsg msg)
{
	std::cerr << "asl_send" << std::endl;
	return 0; // TODO
}

void asl_free(aslmsg msg)
{
	delete msg;
}

int asl_set_query(aslmsg msg, const char *key, const char *value, uint32_t op)
{
	return 0; // TODO
}

aslresponse asl_search(aslclient asl, aslmsg msg)
{
	return nullptr; // TODO
}

aslmsg aslresponse_next(aslresponse r)
{
	return nullptr; // TODO
}

void aslresponse_free(aslresponse r)
{
	delete r;
}

int asl_create_auxiliary_file(aslmsg msg, const char *title, const char *uti, int *out_descriptor)
{
	std::cerr << "asl_create_auxiliary_file" << std::endl;
	return 0; // TODO
}

int asl_close_auxiliary_file(int descriptor)
{
	std::cerr << "asl_close_auxiliary_file" << std::endl;
	return 0; // TODO
}

int asl_log_auxiliary_location(aslmsg msg, const char *title, const char *uti, const char *url)
{
	std::cerr << "asl_log_auxiliary_location" << std::endl;
	return 0; // TODO
}

aslclient asl_open_from_file(int descriptor, const char *ident, const char *facility)
{
	std::cerr << "asl_open_from_file" << std::endl;
	return 0; // TODO
}

int asl_log_descriptor(aslclient asl, aslmsg msg, int level, int descriptor, uint32_t fd_type)
{
	std::cerr << "asl_log_descriptor" << std::endl;
	return 0; // TODO
}


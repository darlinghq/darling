#ifndef _ASL_H
#define _ASL_H

#include <stdint.h>
#include <stdarg.h>

#define ASL_LEVEL_EMERG   0
#define ASL_LEVEL_ALERT   1
#define ASL_LEVEL_CRIT    2
#define ASL_LEVEL_ERR     3
#define ASL_LEVEL_WARNING 4
#define ASL_LEVEL_NOTICE  5
#define ASL_LEVEL_INFO    6
#define ASL_LEVEL_DEBUG   7

#define ASL_STRING_EMERG	"Emergency"
#define ASL_STRING_ALERT	"Alert"
#define ASL_STRING_CRIT		"Critical"
#define ASL_STRING_ERR		"Error"
#define ASL_STRING_WARNING  "Warning"
#define ASL_STRING_NOTICE   "Notice"
#define ASL_STRING_INFO		"Info"
#define ASL_STRING_DEBUG	"Debug"

#define ASL_QUERY_OP_CASEFOLD      0x0010
#define ASL_QUERY_OP_PREFIX		   0x0020
#define ASL_QUERY_OP_SUFFIX		   0x0040
#define ASL_QUERY_OP_SUBSTRING     0x0060
#define ASL_QUERY_OP_NUMERIC       0x0080
#define ASL_QUERY_OP_REGEX         0x0100

#define ASL_QUERY_OP_EQUAL         0x0001
#define ASL_QUERY_OP_GREATER       0x0002
#define ASL_QUERY_OP_GREATER_EQUAL 0x0003
#define ASL_QUERY_OP_LESS          0x0004
#define ASL_QUERY_OP_LESS_EQUAL    0x0005
#define ASL_QUERY_OP_NOT_EQUAL     0x0006
#define ASL_QUERY_OP_TRUE          0x0007

#define ASL_KEY_TIME        "Time"
#define ASL_KEY_TIME_NSEC   "TimeNanoSec"
#define ASL_KEY_HOST        "Host"
#define ASL_KEY_SENDER      "Sender"
#define ASL_KEY_FACILITY    "Facility"
#define ASL_KEY_PID         "PID"
#define ASL_KEY_UID         "UID"
#define ASL_KEY_GID         "GID"
#define ASL_KEY_LEVEL       "Level"
#define ASL_KEY_MSG         "Message"
#define ASL_KEY_READ_UID    "ReadUID"
#define ASL_KEY_READ_GID    "ReadGID"
#define ASL_KEY_EXPIRE_TIME "ASLExpireTime"
#define ASL_KEY_MSG_ID      "ASLMessageID"
#define ASL_KEY_SESSION     "Session"
#define ASL_KEY_REF_PID     "RefPID"
#define ASL_KEY_REF_PROC    "RefProc"
#define ASL_KEY_AUX_TITLE   "ASLAuxTitle"
#define ASL_KEY_AUX_UTI     "ASLAuxUTI"
#define ASL_KEY_AUX_URL     "ASLAuxURL"
#define ASL_KEY_AUX_DATA    "ASLAuxData"
#define ASL_KEY_OPTION      "ASLOption"
#define ASL_KEY_SENDER_INSTANCE	"SenderInstance"

#define ASL_TYPE_MSG    0
#define ASL_TYPE_QUERY  1

#define ASL_FILTER_MASK_EMERG   0x01
#define ASL_FILTER_MASK_ALERT   0x02
#define ASL_FILTER_MASK_CRIT    0x04
#define ASL_FILTER_MASK_ERR     0x08
#define ASL_FILTER_MASK_WARNING 0x10
#define ASL_FILTER_MASK_NOTICE  0x20
#define ASL_FILTER_MASK_INFO    0x40
#define ASL_FILTER_MASK_DEBUG   0x80

#define ASL_OPT_STDERR		0x00000001
#define ASL_OPT_NO_DELAY    0x00000002
#define ASL_OPT_NO_REMOTE   0x00000004

#define ASL_LOG_DESCRIPTOR_READ  1
#define ASL_LOG_DESCRIPTOR_WRITE 2

extern "C" {

class ASLClient;
class ASLMsg;
class ASLResponse;

typedef ASLClient* aslclient;
typedef ASLMsg* aslmsg;
typedef ASLResponse* aslresponse;

int _asl_lib_log(aslclient asl, uint32_t eval, aslmsg msg, const char *format, ...);
uint32_t _asl_evaluate_send(aslclient asl, aslmsg msg, int level);

aslclient asl_open(const char* ident, const char* facility, uint32_t opts);

void asl_close(aslclient asl);

int asl_add_log_file(aslclient asl, int descriptor);

int asl_remove_log_file(aslclient asl, int descriptor);

int asl_set_filter(aslclient asl, int f);

const char* asl_key(aslmsg msg, uint32_t n);

aslmsg asl_new(uint32_t type);

int asl_set(aslmsg msg, const char* key, const char* value);

int asl_unset(aslmsg msg, const char* key);

const char* asl_get(aslmsg msg, const char* key);

int asl_log(aslclient asl, aslmsg msg, int level, const char* format, ...);

int asl_vlog(aslclient asl, aslmsg msg, int level, const char* format, va_list ap);

int asl_send(aslclient asl, aslmsg msg);

void asl_free(aslmsg msg);

int asl_set_query(aslmsg msg, const char* key, const char* value, uint32_t op);

aslresponse asl_search(aslclient asl, aslmsg msg);

aslmsg aslresponse_next(aslresponse r);

void aslresponse_free(aslresponse r);

int asl_create_auxiliary_file(aslmsg msg, const char* title, const char* uti, int* out_descriptor);

int asl_close_auxiliary_file(int descriptor);

int asl_log_auxiliary_location(aslmsg msg, const char* title, const char* uti, const char* url);

aslclient asl_open_from_file(int descriptor, const char* ident, const char* facility);

int asl_log_descriptor(aslclient asl, aslmsg msg, int level, int descriptor, uint32_t fd_type);

}

#endif

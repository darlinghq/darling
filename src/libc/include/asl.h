/*
 * Copyright (c) 2004-2010 Apple Inc. All rights reserved.
 *
 * @APPLE_LICENSE_HEADER_START@
 *
 * This file contains Original Code and/or Modifications of Original Code
 * as defined in and that are subject to the Apple Public Source License
 * Version 2.0 (the 'License'). You may not use this file except in
 * compliance with the License. Please obtain a copy of the License at
 * http://www.opensource.apple.com/apsl/ and read it before using this
 * file.
 * 
 * The Original Code and all software distributed under the License are
 * distributed on an 'AS IS' basis, WITHOUT WARRANTY OF ANY KIND, EITHER
 * EXPRESS OR IMPLIED, AND APPLE HEREBY DISCLAIMS ALL SUCH WARRANTIES,
 * INCLUDING WITHOUT LIMITATION, ANY WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE, QUIET ENJOYMENT OR NON-INFRINGEMENT.
 * Please see the License for the specific language governing rights and
 * limitations under the License.
 * 
 * @APPLE_LICENSE_HEADER_END@
 */

#ifndef __ASL_H__
#define __ASL_H__

#include <stdint.h>
#include <stdarg.h>
#include <sys/cdefs.h>
#include <Availability.h>

typedef struct __aslclient *aslclient;
typedef struct __aslmsg *aslmsg;
typedef struct __aslresponse *aslresponse;

/*! @header
 * These routines provide an interface to the Apple System Log facility.
 * The API allows client applications to create flexible, structured messages
 * and send them to the syslogd server.  Messages received by the server are
 * saved in a data store, subject to input filtering constraints.
 * This API also permits clients to create queries and search the message
 * data store for matching messages.
 */

/*
 * NOTE FOR HeaderDoc
 *
 * These are added to allow headerdoc2html to process 
 * the prototypes of asl_log and asl_vlog correctly.
 * The "-p" option to headerdoc2html is required.
 */
#ifndef __printflike
/*! @parseOnly */
#define __printflike(a,b)
#endif

/*! @defineblock Log Message Priority Levels
 * Log levels of the message.
 */
#define ASL_LEVEL_EMERG   0
#define ASL_LEVEL_ALERT   1
#define ASL_LEVEL_CRIT    2
#define ASL_LEVEL_ERR     3
#define ASL_LEVEL_WARNING 4
#define ASL_LEVEL_NOTICE  5
#define ASL_LEVEL_INFO    6
#define ASL_LEVEL_DEBUG   7
/*! @/defineblock */

/*! @defineblock Log Message Priority Level Strings
 * Strings corresponding to log levels.
 */
#define ASL_STRING_EMERG	"Emergency"
#define ASL_STRING_ALERT	"Alert"
#define ASL_STRING_CRIT		"Critical"
#define ASL_STRING_ERR		"Error"
#define ASL_STRING_WARNING  "Warning"
#define ASL_STRING_NOTICE   "Notice"
#define ASL_STRING_INFO		"Info"
#define ASL_STRING_DEBUG	"Debug"
/*! @/defineblock */

/*! @defineblock Attribute Matching
 * Attribute value comparison operations.
 */
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
/*! @/defineblock */

/*! @defineblock Message Attributes
 *
 * These attributes are known by ASL, and are generally
 * associated with all log messages.
 * Additional attributes may be added as desired.
 */
#define ASL_KEY_TIME        "Time"          /* Timestamp.  Set automatically */
#define ASL_KEY_TIME_NSEC   "TimeNanoSec"   /* Nanosecond time. */
#define ASL_KEY_HOST        "Host"          /* Sender's address (set by the server). */
#define ASL_KEY_SENDER      "Sender"        /* Sender's identification string.  Default is process name. */
#define ASL_KEY_FACILITY    "Facility"      /* Sender's facility.  Default is "user". */
#define ASL_KEY_PID         "PID"           /* Sending process ID encoded as a string.  Set automatically. */
#define ASL_KEY_UID         "UID"           /* UID that sent the log message (set by the server). */
#define ASL_KEY_GID         "GID"           /* GID that sent the log message (set by the server). */
#define ASL_KEY_LEVEL       "Level"         /* Log level number encoded as a string.  See levels above. */
#define ASL_KEY_MSG         "Message"       /* Message text. */
#define ASL_KEY_READ_UID    "ReadUID"       /* User read access (-1 is any user). */
#define ASL_KEY_READ_GID    "ReadGID"       /* Group read access (-1 is any group). */
#define ASL_KEY_EXPIRE_TIME "ASLExpireTime" /* Expiration time for messages with long TTL. */
#define ASL_KEY_MSG_ID      "ASLMessageID"  /* 64-bit message ID number (set by the server). */
#define ASL_KEY_SESSION     "Session"       /* Session (set by the launchd). */
#define ASL_KEY_REF_PID     "RefPID"        /* Reference PID for messages proxied by launchd */
#define ASL_KEY_REF_PROC    "RefProc"       /* Reference process for messages proxied by launchd */
#define ASL_KEY_AUX_TITLE   "ASLAuxTitle"   /* Auxiliary title string */
#define ASL_KEY_AUX_UTI     "ASLAuxUTI"     /* Auxiliary Uniform Type ID */
#define ASL_KEY_AUX_URL     "ASLAuxURL"     /* Auxiliary Uniform Resource Locator */
#define ASL_KEY_AUX_DATA    "ASLAuxData"    /* Auxiliary in-line data */
#define ASL_KEY_OPTION      "ASLOption"     /* Internal */
#define ASL_KEY_SENDER_INSTANCE	"SenderInstance"	/* Sender instance UUID. */
/*! @/defineblock */

/*! @defineblock aslmsg Types
 * Message type argument passed to asl_new().
 */
#define ASL_TYPE_MSG    0
#define ASL_TYPE_QUERY  1
/*! @/defineblock */

/*! @defineblock Filter Masks
 * Used in client-side filtering, which determines which
 * messages are sent by the client to the syslogd server.
 */
#define ASL_FILTER_MASK_EMERG   0x01
#define ASL_FILTER_MASK_ALERT   0x02
#define ASL_FILTER_MASK_CRIT    0x04
#define ASL_FILTER_MASK_ERR     0x08
#define ASL_FILTER_MASK_WARNING 0x10
#define ASL_FILTER_MASK_NOTICE  0x20
#define ASL_FILTER_MASK_INFO    0x40
#define ASL_FILTER_MASK_DEBUG   0x80
/*! @/defineblock */

/*! @defineblock Filter Mask Macros
 * Macros to create bitmasks for filter settings - see asl_set_filter().
 */
#define	ASL_FILTER_MASK(level) (1 << (level))
#define	ASL_FILTER_MASK_UPTO(level) ((1 << ((level) + 1)) - 1)
/*! @/defineblock */

/*! @defineblock Client Creation Options
 * Options for asl_open().
 */
#define ASL_OPT_STDERR		0x00000001
#define ASL_OPT_NO_DELAY    0x00000002
#define ASL_OPT_NO_REMOTE   0x00000004
/*! @/defineblock */

/*! @defineblock File Descriptor Types
 * Instructions on how to treat the file descriptor in asl_log_descriptor().
 */
#define ASL_LOG_DESCRIPTOR_READ  1
#define ASL_LOG_DESCRIPTOR_WRITE 2

/*!
 * ASL_PREFILTER_LOG is a macro similar to asl_log(), but it first checks
 * if the message will simply be ignored due to local filter settings.
 * This prevents the variable argument list from being evaluated.
 * Note that the message may still be processed if it will be written
 * to a file or stderr.
 *
 * @param asl
 *    (input) An ASL client handle
 * @param msg
 *    (input) An aslmsg (default attributes will be supplied if msg is NULL)
 * @param level
 *    (input) Log level (ASL_LEVEL_DEBUG to ASL_LEVEL_EMERG)
 * @param format
 *    (input) A printf() - style format string followed by a list of arguments
 */
#define ASL_PREFILTER_LOG(asl, msg, level, format, ...) \
	do { \
		aslclient _asl = (asl); \
		aslmsg _msg = (msg); \
		uint32_t _asl_eval = _asl_evaluate_send(_asl, _msg, (level)); \
		if (_asl_eval != 0) _asl_lib_log(_asl, _asl_eval, _msg, (format), ## __VA_ARGS__); \
	} while (0)

__BEGIN_DECLS

/* ASL Library SPI - do not call directly */
int _asl_lib_log(aslclient asl, uint32_t eval, aslmsg msg, const char *format, ...) __printflike(4, 5);

uint32_t _asl_evaluate_send(aslclient asl, aslmsg msg, int level);

/*!
 * Initialize a connection to the ASL server.
 *
 * This call is optional in most cases.  The library will perform any
 * necessary initializations on the fly.  A call to asl_open() is required
 * if optional settings must be made before messages are sent to the server.
 * These include setting the client filter and managing additional output
 * file descriptors.  Note that the default setting of the client filter is
 * ASL_FILTER_MASK_UPTO(ASL_LEVEL_NOTICE), so ASL_LEVEL_DEBUG and ASL_LEVEL_INFO
 * messages are not sent to the server by default.
 * 
 * Options (defined above) may be set using the opts parameter. They are:
 *
 *   ASL_OPT_STDERR    - adds stderr as an output file descriptor
 *
 *   ASL_OPT_NO_DELAY  - connects to the server immediately
 *
 *   ASL_OPT_NO_REMOTE - disables the remote-control mechanism for adjusting
 *                       filter levers for processes using e.g. syslog -c ...
 *
 * @param ident
 *    (input) Sender name
 * @param facility
 *    (input) Facility name
 * @param opts
 *    (input) Options (see asl_open Options)
 * @result Returns an ASL client handle
 */
aslclient asl_open(const char *ident, const char *facility, uint32_t opts);

/*!
 * Shuts down a connection to the server.
 *
 * @param asl
 *    (input) An ASL client handle
 */
void asl_close(aslclient asl);

/*!
 * Write log messages to the given file descriptor.
 *
 * Log messages will be written to this file as well as to the server.
 *
 * @param asl
 *    (input) An ASL client handle
 * @param descriptor
 *    (input) A file descriptor
 * @result Returns 0 on success, non-zero on failure
*/
int asl_add_log_file(aslclient asl, int descriptor);

/*!
 * Stop writing log messages to the given file descriptor.
 * The file descripter is not closed by this routine.
 *
 * @param asl
 *    (input) An ASL client handle
 * @param descriptor
 *    (input) A file descriptor
 * @result Returns 0 on success, non-zero on failure
 */
int asl_remove_log_file(aslclient asl, int descriptor);

/*!
 * Set a filter for messages being sent to the server.
 * The filter is a bitmask representing priorities.  The ASL_FILTER_MASK
 * macro may be used to convert a priority level into a bitmask for that
 * level.  The ASL_FILTER_MASK_UPTO macro creates a bitmask for all
 * priorities up to and including a given priority.
 * Messages with priority levels that do not have a corresponding bit 
 * set in the filter are not sent to the server, although they will be
 * sent to any file descripters added with asl_add_log_file().
 * The default setting is ASL_FILTER_MASK_UPTO(ASL_LEVEL_NOTICE).
 * Returns the previous filter value.
 *
 * @param asl
 *    (input) An ASL client handle
 * @param f
 *    (input) A filter value
 * @result Returns the previous filter value
 */
int asl_set_filter(aslclient asl, int f);

/*
 * Examine attribute keys.
 *
 * @param msg
 *    (input) An ASL message
 * @param n
 *    (input) An index value
 * @result Returns the key of the nth attribute in a message (beginning at zero),
 * or NULL if n is greater than the largest message index.
 */
const char *asl_key(aslmsg msg, uint32_t n);

/*!
 * Create a new log message or query message.
 *
 * @param type
 *    (input) Message type (see aslmsg Types)
 * @result Returns a newly allocated asmsg of the specified type
 */
aslmsg asl_new(uint32_t type);

/*!
 * Set or re-set a message attribute.
 *
 * @param msg
 *    (input) An aslmsg
 * @param key
 *    (input) Attribute key 
 * @param value
 *    (input) Attribute value
 * @result returns 0 for success, non-zero for failure
 */
int asl_set(aslmsg msg, const char *key, const char *value);

/*!
 * Remove a message attribute.
 *
 * @param msg
 *    (input) An aslmsg
 * @param key
 *    (input) Attribute key 
 * returns 0 for success, non-zero for failure
 */
int asl_unset(aslmsg msg, const char *key);

/*!
 * Get the value of a message attribute.
 *
 * @param msg
 *    (input) An aslmsg
 * @param key
 *    (input) Attribute key 
 * @result Returns the attribute value, or NULL if the message does not contain the key
 */
const char *asl_get(aslmsg msg, const char *key);

/*!
 * Log a message with a particular log level.
 *
 * @param asl
 *    (input) An ASL client handle
 * @param msg
 *    (input) An aslmsg (default attributes will be supplied if msg is NULL)
 * @param level
 *    (input) Log level (ASL_LEVEL_DEBUG to ASL_LEVEL_EMERG)
 * @param format
 *    (input) A printf() - style format string followed by a list of arguments
 * @result Returns 0 for success, non-zero for failure
 */
int asl_log(aslclient asl, aslmsg msg, int level, const char *format, ...) __printflike(4, 5);

/*!
 * Log a message with a particular log level.
 * Similar to asl_log, but takes a va_list argument.
 *
 * @param asl
 *    (input) An ASL client handle
 * @param msg
 *    (input) An aslmsg (default attributes will be supplied if msg is NULL)
 * @param level
 *    (input) Log level (ASL_LEVEL_DEBUG to ASL_LEVEL_EMERG)
 * @param format
 *    (input) A printf() - style format string followed by a list of arguments
 * @param ap
 *    (input) A va_list containing the values for the format string
 * @result Returns 0 for success, non-zero for failure
 */
int asl_vlog(aslclient asl, aslmsg msg, int level, const char *format, va_list ap) __printflike(4, 0);

/*!
 * Log a message.
 *
 * This routine may be used instead of asl_log() or asl_vlog() if asl_set() 
 * has been used to set all of a message's attributes.
 *
 * @param asl
 *    (input) An ASL client handle
 * @param msg
 *    (input) An aslmsg
 * @result Returns 0 for success, non-zero for failure
 */
int asl_send(aslclient asl, aslmsg msg);

/*!
 * Free a message.  Frees all the attribute keys and values.
 *
 * @param msg
 *    (input) An aslmsg to free
 */
void asl_free(aslmsg msg);

/*!
 * Set arbitrary parameters of a query.
 * This is similar to asl_set, but allows richer query operations.
 * See ASL_QUERY_OP_* above.
 *
 * @param msg
 *    (input) An aslmsg
 * @param key
 *    (input) Attribute key 
 * @param value
 *    (input) Attribute value
 * @param op
 *    (input) An operation (ASL_QUERY_OP_*)
 * @result Returns 0 for success, non-zero for failure
 */
int asl_set_query(aslmsg msg, const char *key, const char *value, uint32_t op);

/*!
 * Search for messages matching the criteria described by the aslmsg.
 * The caller should set the attributes to match using asl_set_query() or asl_set().
 * The operatoin ASL_QUERY_OP_EQUAL is used for attributes set with asl_set().
 *
 * @param msg
 *    (input) An aslmsg to match
 * @result Returns a set of messages accessable using aslresponse_next(),
 */
aslresponse asl_search(aslclient asl, aslmsg msg);

/*!
 * Iterate over responses returned from asl_search().
 *
 * @param r
 *    (input) An aslresponse returned by asl_search()
 * @result Returns the next message (an aslmsg) in the response, or NULL when there are no more messages
 */
aslmsg aslresponse_next(aslresponse r);

/*!
 * Free a response returned from asl_search().
 * @param r
 *    (input) An aslresponse returned by asl_search()
 */
void aslresponse_free(aslresponse r);

/*!
 * Creates an auxiliary file that may be used to save arbitrary data.  The ASL message msg
 * will be saved at the time that the auxiliary file is closed with asl_close_auxiliary_file().
 * The log entry will include any keys and values found in msg, and it will include the title
 * and Uniform Type Identifier specified.  If NULL is supplied as a value for the uti parameter,
 * the type "public.data" is used.  Console.app will display a hyperlink to the file.
 * Output parameter out_descriptor will contain a readable and writable file descriptor for the new
 * auxiliary file. 
 *
 * By default, the file will be world-readable.  If the message contains a ReadUID and/or a
 * ReadGID key, then the values for those keys will determine read access to the file.
 *
 * The file will be deleted at the same time that the message expires from the ASL data store.
 * The aslmanager utility manages message expiry.  If msg contains a value for ASLExpireTime,
 * then the message and the file will not be deleted before that time.  The value may be in
 * seconds after the Epoch, or it may be ctime() format, e.g "Thu Jun 24 18:22:48 2010".
 * 
 * @param msg
 *    (input) An aslmsg
 * @param tite
 *    (input) A title string for the file
 * @param uti
 *    (input) Uniform Type Identifier for the file
 * @param out_descriptor
 *    (output) A writable file descriptor
 * @result Returns 0 for success, non-zero for failure
 */
int asl_create_auxiliary_file(aslmsg msg, const char *title, const char *uti, int *out_descriptor)
__OSX_AVAILABLE_STARTING(__MAC_10_7,__IPHONE_5_0);

/*!
 * Close an auxiliary file opened by asl_create_auxiliary_file() when writing is complete.
 * syslogd will log the message provided to asl_create_auxiliary_file() when this routine
 * is called.
 *
 * @param descriptor
 *    (input) The file descriptor
 * @result Returns 0 for success, non-zero for failure
 */
int asl_close_auxiliary_file(int descriptor)
__OSX_AVAILABLE_STARTING(__MAC_10_7,__IPHONE_5_0);

/*!
 * Sends an ASL message to syslogd along with a title string, Uniform Resource Locator, 
 * and Uniform Type Identifier specified.  Console.app will hyperlink the title string to
 * the specified URL.  If NULL is supplied as a value for the uti parameter, the default
 * type "public.data" is used.
 *
 * @param msg
 *    (input) An aslmsg
 * @param title
 *    (input) A title string for the file
 * @param uti
 *    (input) Uniform Type Identifier for the file
 * @param url
 *    (input) Uniform Type Locator
 * @result Returns 0 for success, non-zero for failure
 */
int asl_log_auxiliary_location(aslmsg msg, const char *title, const char *uti, const char *url)
__OSX_AVAILABLE_STARTING(__MAC_10_7,__IPHONE_5_0);

/*!
 * Creates an aslclient for logging to a file descriptor.  The file must be opened for read and
 * write access.  This routine may be used in conjunction with asl_create_auxiliary_file() to
 * save ASL format log messages to an auxiliary file.
 *
 * The file will be truncated if it is not empty.  When logging to the auxiliary file is complete,
 * aslclient should be closed using asl_close().  The file should be closed using
 * asl_close_auxiliary_file() if it was returned by asl_create_auxiliary_file(), or close()
 * otherwise.
 *
 * The returned aslclient is thread-safe.
 *
 * Note that per-message read access controls (ReadUID and ReadGID) and message expire
 * times (ASLExpireTime) keys have no effect for messages written to this file.
 *
 * @param descriptor
 *    (input) A file descriptor
 * @param ident
 *    (input) Sender name
 * @param facility
 *    (input) Facility name
 * @result An aslclient
 */
aslclient asl_open_from_file(int descriptor, const char *ident, const char *facility)
__OSX_AVAILABLE_STARTING(__MAC_10_7,__IPHONE_5_0);

/*!
 * This API provides functionality to use file descriptors to send logging
 * data to ASL.
 *
 * asl is retained by ASL and must still be closed by the caller by calling
 * asl_close() if the caller loses reference to it.  msg is copied by ASL and
 * similarly must still be freed by the caller by calling asl_free() if the
 * caller loses reference to it.  Any changes made to it after calling
 * asl_log_descriptor() are not applicable to the message used. descriptor
 * is treated differentlty based on the value of fd_type.
 *
 * If fd_type is ASL_LOG_DESCRIPTOR_READ, the descriptor must be open for read
 * access.  ASL uses GCD to read from the descriptor as data becomes available.
 * These data are line buffered and passed to asl_log.  When EOF is read, the
 * descriptor is closed.
 *
 * Example:
 * asl_log_descriptor(c, m, ASL_LEVEL_NOTICE, STDIN_FILENO, ASL_LOG_DESCRIPTOR_READ);
 *
 * If fd_type is ASL_LOG_DESCRIPTOR_WRITE, the descriptor is closed and a new
 * writable descriptor is created with the same fileno.  Any data written to
 * this new descriptor are line buffered and passed to asl_log.  When EOF is
 * sent, no further data are read.  The caller is responsible for closing the
 * new descriptor.  One common use for this API is to redirect writes to stdout
 * or stderr to ASL by passing STDOUT_FILENO or STDERR_FILENO as descriptor.
 *
 * Example:
 * asl_log_descriptor(c, m, ASL_LEVEL_NOTICE, STDOUT_FILENO, ASL_LOG_DESCRIPTOR_WRITE);
 * asl_log_descriptor(c, m, ASL_LEVEL_ERR, STDERR_FILENO, ASL_LOG_DESCRIPTOR_WRITE);
 *
 * @param asl
 *    (input) An ASL client handle
 * @param msg
 *    (input) An aslmsg (default attributes will be supplied if msg is NULL)
 * @param level
 *    (input) Log level (ASL_LEVEL_DEBUG to ASL_LEVEL_EMERG)
 * @param descriptor
 *    (input) An open file descriptor to read from
 * @param fd_type
 *    (input) Either ASL_LOG_DESCRIPTOR_READ or ASL_LOG_DESCRIPTOR_WRITE
 * @result Returns 0 for success, non-zero for failure
 */
int asl_log_descriptor(aslclient asl, aslmsg msg, int level, int descriptor, uint32_t fd_type)
__OSX_AVAILABLE_STARTING(__MAC_10_8,__IPHONE_5_1);

__END_DECLS

#endif /* __ASL_H__ */

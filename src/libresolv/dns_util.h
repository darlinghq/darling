/*
 * Copyright (c) 2003-2006 Apple Computer, Inc. All rights reserved.
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

#ifndef __DNS_UTIL_H__
#define __DNS_UTIL_H__

#include <sys/cdefs.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <dns.h>

/*
 * Status returned in a dns_reply_t
 */
#define DNS_STATUS_OK					0
#define DNS_STATUS_BAD_HANDLE			1
#define DNS_STATUS_MALFORMED_QUERY		2
#define DNS_STATUS_TIMEOUT				3
#define DNS_STATUS_SEND_FAILED			4
#define DNS_STATUS_RECEIVE_FAILED		5
#define DNS_STATUS_CONNECTION_FAILED	6
#define DNS_STATUS_WRONG_SERVER			7
#define DNS_STATUS_WRONG_XID			8
#define DNS_STATUS_WRONG_QUESTION		9

/*
 * dns_print_reply mask
 */
#define DNS_PRINT_XID        0x0001
#define DNS_PRINT_QR         0x0002
#define DNS_PRINT_OPCODE     0x0004
#define DNS_PRINT_AA         0x0008
#define DNS_PRINT_TC         0x0010
#define DNS_PRINT_RD         0x0020
#define DNS_PRINT_RA         0x0040
#define DNS_PRINT_PR         0x0080
#define DNS_PRINT_RCODE      0x0100
#define DNS_PRINT_QUESTION   0x0200
#define DNS_PRINT_ANSWER     0x0400
#define DNS_PRINT_AUTHORITY  0x0800
#define DNS_PRINT_ADDITIONAL 0x1000
#define DNS_PRINT_SERVER     0x2000

/*
 * DNS query / reply header
 */
typedef struct {
	uint16_t xid;
	uint16_t flags;
	uint16_t qdcount;
	uint16_t ancount;
	uint16_t nscount;
	uint16_t arcount;
} dns_header_t;

/*
 * DNS query
 */
typedef struct
{
	char *name;
	uint16_t dnstype;
	uint16_t dnsclass;
} dns_question_t;

/*
 * Resource Record types
 * dns_parse_packet() creates resourse records of these types.
 */
typedef struct
{
	uint16_t length;
	char *data;
} dns_raw_resource_record_t;

typedef struct
{
	struct in_addr addr;
} dns_address_record_t;

typedef struct
{
	struct in6_addr addr;
} dns_in6_address_record_t;

typedef struct
{
	char *name;
} dns_domain_name_record_t;

typedef struct
{
	char *mname;
	char *rname;
	uint32_t serial;
	uint32_t refresh;
	uint32_t retry;
	uint32_t expire;
	uint32_t minimum;
} dns_SOA_record_t;

typedef struct
{
	char *cpu;
	char *os;
} dns_HINFO_record_t;

typedef struct
{
	char *rmailbx;
	char *emailbx;
} dns_MINFO_record_t;

typedef struct
{
	uint16_t preference;
	char *name;
} dns_MX_record_t;

typedef struct
{
	uint32_t string_count;
	char **strings;
} dns_TXT_record_t;

typedef struct
{
	struct in_addr addr;
	uint8_t protocol;
	uint32_t maplength;
	uint8_t *map;
} dns_WKS_record_t;

typedef struct
{
	char *mailbox;
	char *txtdname;
} dns_RP_record_t;

typedef struct
{
	uint32_t subtype;
	char *hostname;
} dns_AFSDB_record_t;

typedef struct
{
	char *psdn_address;
} dns_X25_record_t;

typedef struct
{
	char *isdn_address;
	char *subaddress;
} dns_ISDN_record_t;

typedef struct
{
	uint16_t preference;
	char * intermediate;
} dns_RT_record_t;

typedef struct
{
	uint8_t version;
	uint8_t size;
	uint8_t horizontal_precision;
	uint8_t vertical_precision;
	uint32_t latitude;
	uint32_t longitude;
	uint32_t altitude;
} dns_LOC_record_t;

typedef struct
{
	uint16_t priority;
	uint16_t weight;
	uint16_t port;
	char *target;
} dns_SRV_record_t;

/*
 * DNS Resource Record
 *
 * Data contained in unsupported or obsolete Resource Record types
 * may be accessed via DNSNULL as a dns_raw_resource_record_t. 
 */
typedef struct
{
	char *name;
	uint16_t dnstype;
	uint16_t dnsclass;
	uint32_t ttl;
	union
	{
		dns_address_record_t *A;
		dns_domain_name_record_t *NS;
		dns_domain_name_record_t *MD;		/* Obsolete */
		dns_domain_name_record_t *MF;		/* Obsolete */
		dns_domain_name_record_t *CNAME;
		dns_SOA_record_t *SOA;
		dns_domain_name_record_t *MB;
		dns_domain_name_record_t *MG;
		dns_domain_name_record_t *MR;
		dns_raw_resource_record_t *DNSNULL;
		dns_WKS_record_t *WKS;
		dns_domain_name_record_t *PTR;
		dns_HINFO_record_t *HINFO;
		dns_MINFO_record_t *MINFO;
		dns_MX_record_t *MX;
		dns_TXT_record_t *TXT;
		dns_RP_record_t *RP;
		dns_AFSDB_record_t *AFSDB;
		dns_X25_record_t *X25;
		dns_ISDN_record_t *ISDN;
		dns_RT_record_t *RT;
		dns_in6_address_record_t *AAAA;
		dns_LOC_record_t *LOC;
		dns_SRV_record_t *SRV;
	} data;
} dns_resource_record_t;

/*
 * A parsed DNS record.  Returned by dns_parse_packet() and dns_lookup().
 * The contents may be printed using dns_print_reply().
 */
typedef struct
{
	uint32_t status;
	struct sockaddr *server;
	dns_header_t *header;
	dns_question_t **question;
	dns_resource_record_t **answer;
	dns_resource_record_t **authority;
	dns_resource_record_t **additional;
} dns_reply_t;


__BEGIN_DECLS

/*
 * High-level lookup performs a search (using dns_search), parses the 
 * reply and returns a dns_reply_t structure.
 *
 * The DNS handle contains an internal buffer used for fetching replies.
 * The buffer is reused for each query, and is released with the DNS client
 * handle when dns_free() is called.  The default buffer size is 1024 bytes.
 * The size may be changed with dns_set_buffer_size.
 *
 * Note that in a multithreaded application, each thread using this API must
 * open a separate handle.
 */
extern dns_reply_t *dns_lookup(dns_handle_t dns, const char *name, uint32_t dnsclass, uint32_t dnstype);

/*
 * Get / Set the size of the internal receive buffer used by dns_lookup()
 */
extern uint32_t dns_get_buffer_size(dns_handle_t d);
extern void dns_set_buffer_size(dns_handle_t d, uint32_t len);

/*
 * Parse a reply packet into a reply structure.
 */
extern dns_reply_t *dns_parse_packet(const char *buf, uint32_t len);

/*
 * Free a reply structure.
 */
extern void dns_free_reply(dns_reply_t *r);

/*
 * Parse a query packet into a question structure.
 */
extern dns_question_t *dns_parse_question(const char *buf, uint32_t len);

/*
 * Free a question structure.
 */
extern void dns_free_question(dns_question_t *q);

/*
 * Parse a resource record into a structure.
 */
extern dns_resource_record_t *dns_parse_resource_record(const char *buf, uint32_t len);

/*
 * Free a resource record structure.
 */
extern void dns_free_resource_record(dns_resource_record_t *rr);

/*
 * String / number representation of a DNS class
 * dns_class_number returns 0 if the string is recognized,
 * non-zero if the class string is unknown.
 */
extern const char *dns_class_string(uint16_t dnsclass);
extern int32_t dns_class_number(const char *c, uint16_t *n);

/*
 * String / number representation of a DNS type
 * dns_type_number returns 0 if the string is recognized,
 * non-zero if the class string is unknown.
 */
extern const char *dns_type_string(uint16_t dnstype);
extern int32_t dns_type_number(const char *t, uint16_t *n);

/*
 * Print a dns handle.
 */
extern void dns_print_handle(dns_handle_t d, FILE *f);

/*
 * Print the contents of a question structure.
 */
extern void dns_print_question(const dns_question_t *q, FILE *f);

/*
 * Print the contents of a resource record structure.
 */
extern void dns_print_resource_record(const dns_resource_record_t *r, FILE *f);

/*
 * Print the contents of a reply structure.
 */
extern void dns_print_reply(const dns_reply_t *r, FILE *f, uint16_t mask);

__END_DECLS

#endif /* __DNS_UTIL_H__ */

/*
 * Copyright (c) 2017 Apple Inc. All rights reserved.
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

/*
 * Modification History
 *
 * May 18, 2017		Allan Nathanson <ajn@apple.com>
 * - initial revision
 */

#define	_LIBLOG_SYSTEMCONFIGURATION_

#import <Foundation/Foundation.h>
#import <os/log_private.h>
#import <os/state_private.h>
#import <string.h>

#import <dnsinfo.h>
#import "dnsinfo_internal.h"
#import <network_information.h>

#define	my_log(__level, __format, ...)	[string appendFormat:@(__format "\n"), ## __VA_ARGS__]
#define my_log_context_type	NSMutableString *
#define	my_log_context_name	string
#import "dnsinfo_logging.h"
#import "network_state_information_logging.h"
#undef	my_log_context_name
#undef	my_log_context_type
#undef	my_log

#define SCAS(str)			\
	[[NSAttributedString alloc] initWithString:str]

#define SCASWithFormat(format, ...)	\
	SCAS(([[NSString alloc] initWithFormat:format, ##__VA_ARGS__]))

#pragma mark -
#pragma mark os_log formatting entry point

struct SC_OSLog_Formatters {
	const char *type;
	NS_RETURNS_RETAINED NSAttributedString * (*function)(id value);
};

NS_RETURNS_RETAINED
NSAttributedString *
OSLogCopyFormattedString(const char *type, id value, os_log_type_info_t info)
{
#pragma	unused(info)
	// add functions for each type into this list
	static const struct SC_OSLog_Formatters formatters[] = {
//		{ .type = "???",  .function = _SC_OSLogCopyFormattedString_??? },
	};

	for (int i = 0; i < (int)(sizeof(formatters) / sizeof(formatters[0])); i++) {
		if (strcmp(type, formatters[i].type) == 0) {
			return formatters[i].function(value);
		}
	}

	return SCASWithFormat(@"liblog_SystemConfiguration: Not yet supported os_log formatting type: %s", type);
}

#pragma mark -
#pragma mark os_state formatting entry point

#define SCNS(str)			\
	[[NSString alloc] initWithString:(str)]

#define SCNSWithFormat(format, ...)			\
	[[NSString alloc] initWithFormat:format, ##__VA_ARGS__]

static NS_RETURNS_RETAINED NSString *
_SC_OSStateCopyFormattedString_dnsinfo(uint32_t data_size, void *data)
{
	dns_config_t		*dns_config	= NULL;
	_dns_config_buf_t	*dns_config_buf;
	NSMutableString		*string;

	// os_state_add_handler w/
	//	osd_type                 = OS_STATE_DATA_CUSTOM
	//	osd_decoder.osdd_library = "SystemConfiguration
	//	osd_decoder.osdd_type    = "dnsinfo"

	if ((data_size == 0) || (data == NULL)) {
		return @"No DNS configuration";
	} else if (data_size < sizeof(_dns_config_buf_t)) {
		return SCNSWithFormat(@"DNS configuration: size error (%d < %zd)",
				      data_size,
				      sizeof(_dns_config_buf_t));
	}

	dns_config_buf = _dns_configuration_buffer_create(data, data_size);
	if (dns_config_buf == NULL) {
		return @"DNS configuration: data error";
	}

	dns_config = _dns_configuration_buffer_expand(dns_config_buf);
	if (dns_config == NULL) {
		// if we were unable to expand the configuration
		_dns_configuration_buffer_free(&dns_config_buf);
		return @"DNS configuration: expansion error";
	}

	string = [NSMutableString string];
	_dns_configuration_log(dns_config, TRUE, string);
	if (string.length == 0) {
		[string appendString:@"DNS configuration: not available"];
	}
	free(dns_config);

	return string;
}

static NS_RETURNS_RETAINED NSString *
_SC_OSStateCopyFormattedString_nwi(uint32_t data_size, void *data)
{
	nwi_state_t		state	= (nwi_state_t)data;
	NSMutableString		*string;

	// os_state_add_handler w/
	//	osd_type                 = OS_STATE_DATA_CUSTOM
	//	osd_decoder.osdd_library = "SystemConfiguration
	//	osd_decoder.osdd_type    = "nwi"

	if ((data_size == 0) || (data == NULL)) {
		return @"No network information";
	} else if (data_size < sizeof(nwi_state)) {
		return SCNSWithFormat(@"Network information: size error (%d < %zd)",
				      data_size,
				      sizeof(_dns_config_buf_t));
	} else if (state->version != NWI_STATE_VERSION) {
		return SCNSWithFormat(@"Network information: version error (%d != %d)",
				      state->version,
				      NWI_STATE_VERSION);
	}

	string = [NSMutableString string];
	_nwi_state_log(state, TRUE, string);
	if (string.length == 0) {
		[string appendString:@"Network information: not available"];
	}

	return string;
}

struct SC_OSState_Formatters {
	const char *type;
	NS_RETURNS_RETAINED NSString * (*function)(uint32_t data_size, void *data);
};

NS_RETURNS_RETAINED
NSString *
OSStateCreateStringWithData(const char *type, uint32_t data_size, void *data)
{
	// add functions for each type into this list
	static const struct SC_OSState_Formatters formatters[] = {
		{ .type = "dnsinfo",	.function = _SC_OSStateCopyFormattedString_dnsinfo	},
		{ .type = "nwi",	.function = _SC_OSStateCopyFormattedString_nwi		},
	};

	for (int i = 0; i < (int)(sizeof(formatters) / sizeof(formatters[0])); i++) {
		if (strcmp(type, formatters[i].type) == 0) {
			return formatters[i].function(data_size, data);
		}
	}

	return SCNSWithFormat(@"liblog_SystemConfiguration: Not yet supported os_state formatting type: %s", type);
}


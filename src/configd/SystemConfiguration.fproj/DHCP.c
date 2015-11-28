/*
 * Copyright (c) 2001, 2003-2005, 2011, 2013 Apple Inc. All rights reserved.
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

#include <stdio.h>
#include <unistd.h>


#include "SystemConfiguration.h"
#include "SCValidation.h"
#include "SCDynamicStoreCopyDHCPInfo.h"
#include "DHCPClientPreferences.h"

#define DHCPCLIENT_PREFERENCES_ID		"DHCPClient.xml"
#define DHCPCLIENT_APPLICATION_PREF		"Application"

#define DHCP_REQUESTED_PARAMETER_LIST		"DHCPRequestedParameterList"

/**
 ** DHCPClientPreferences{Set,Get}ApplicationOptions()
 **/
static UInt8 *
S_get_char_array(CFArrayRef arr, CFIndex * len)
{
    UInt8 *	buf = NULL;
    CFIndex	count = 0;
    CFIndex 	i;
    CFIndex 	real_count;

    if (arr) {
	count = CFArrayGetCount(arr);
    }
    if (count == 0) {
	goto done;
    }
    buf = malloc(count);
    if (buf == NULL) {
	goto done;
    }
    for (i = 0, real_count = 0; i < count; i++) {
	CFNumberRef	n = isA_CFNumber(CFArrayGetValueAtIndex(arr, i));
	int 		val;

	if (n && CFNumberGetValue(n, kCFNumberIntType, &val)) {
	    buf[real_count++] = (UInt8) val;
	}
    }
    count = real_count;
 done:
    *len = count;
    if (count == 0 && buf) {
	free(buf);
	buf = NULL;
    }
    return (buf);
}

static void
my_CFArrayAppendUniqueValue(CFMutableArrayRef arr, CFTypeRef new)
{
    CFIndex i;
    CFIndex n = CFArrayGetCount(arr);

    for (i = 0; i < n; i++) {
	CFStringRef element = CFArrayGetValueAtIndex(arr, i);
	if (CFEqual(element, new)) {
	    return;
	}
    }
    CFArrayAppendValue(arr, new);
    return;
}

static __inline__ CF_RETURNS_RETAINED CFStringRef
S_application_path(CFStringRef applicationID)
{
    return (CFStringCreateWithFormat(NULL, NULL,
				     CFSTR("/" DHCPCLIENT_APPLICATION_PREF
					   "/%@"),
				     applicationID));
}

Boolean
DHCPClientPreferencesSetApplicationOptions(CFStringRef applicationID,
					   UInt8 * options,
					   CFIndex count)
{
    CFMutableDictionaryRef	dict = NULL;
    CFStringRef			path = NULL;
    SCPreferencesRef		prefs = NULL;
    Boolean			success = FALSE;

    if (applicationID == NULL) {
	goto done;
    }
    path = S_application_path(applicationID);
    if (path == NULL) {
	goto done;
    }
    prefs = SCPreferencesCreate(NULL, CFSTR("DHCPClientSetAppReqParams"),
				CFSTR(DHCPCLIENT_PREFERENCES_ID));
    if (prefs == NULL) {
	goto done;
    }
    dict = (CFMutableDictionaryRef)SCPreferencesPathGetValue(prefs, path);
    if (dict == NULL) {
	dict = CFDictionaryCreateMutable(NULL, 0,
					 &kCFTypeDictionaryKeyCallBacks,
					 &kCFTypeDictionaryValueCallBacks);
    }
    else {
	dict = CFDictionaryCreateMutableCopy(NULL, 0, dict);
    }
    if (dict == NULL) {
	goto done;
    }
    if (options && count > 0) {
	int 			i;
	CFMutableArrayRef	array;

	array = CFArrayCreateMutable(NULL, 0, &kCFTypeArrayCallBacks);
	if (array == NULL) {
	    goto done;
	}
	for (i = 0; i < count; i++) {
	    int			val;
	    CFNumberRef		number;

	    if (options[i] == 0 || options[i] == 255) {
		/* ignore pads and end */
		continue;
	    }
	    val = options[i];
	    number = CFNumberCreate(NULL, kCFNumberIntType, &val);
	    if (number == NULL) {
		CFRelease(array);
		goto done;
	    }
	    my_CFArrayAppendUniqueValue(array, number);
	    CFRelease(number);
	}
	CFDictionarySetValue(dict, CFSTR(DHCP_REQUESTED_PARAMETER_LIST),
			     array);
	CFRelease(array);
    }
    else {
	CFDictionaryRemoveValue(dict, CFSTR(DHCP_REQUESTED_PARAMETER_LIST));
    }
    if (SCPreferencesLock(prefs, TRUE)) {
	success = SCPreferencesPathSetValue(prefs, path, dict);
	if (success) {
	    success = SCPreferencesCommitChanges(prefs);
	    if (success) {
		(void)SCPreferencesApplyChanges(prefs);
	    }
	}
	(void)SCPreferencesUnlock(prefs);
    }
 done:
    if (prefs) {
	CFRelease(prefs);
    }
    if (path) {
	CFRelease(path);
    }
    if (dict) {
	CFRelease(dict);
    }
    return (success);
}

UInt8 *
DHCPClientPreferencesCopyApplicationOptions(CFStringRef applicationID,
					    CFIndex * count)
{
    CFDictionaryRef		dict = NULL;
    UInt8 *			options = NULL;
    CFArrayRef			parms;
    CFStringRef			path = NULL;
    SCPreferencesRef		prefs = NULL;

    if (applicationID == NULL) {
	goto done;
    }
    path = S_application_path(applicationID);
    if (path == NULL) {
	goto done;
    }
    prefs = SCPreferencesCreate(NULL, CFSTR("DHCPClientCopyAppReqParams"),
				CFSTR(DHCPCLIENT_PREFERENCES_ID));
    if (prefs == NULL) {
	goto done;
    }
    dict = SCPreferencesPathGetValue(prefs, path);
    if (dict == NULL) {
	goto done;
    }
    parms = CFDictionaryGetValue(dict,
				 CFSTR(DHCP_REQUESTED_PARAMETER_LIST));
    if (isA_CFArray(parms) == NULL) {
	goto done;
    }
    options = S_get_char_array(parms, count);

 done:
    if (prefs) {
	CFRelease(prefs);
    }
    if (path) {
	CFRelease(path);
    }
    return (options);
}

/**
 ** DHCPClientInfo*()
 **/

CFDictionaryRef
SCDynamicStoreCopyDHCPInfo(SCDynamicStoreRef store, CFStringRef serviceID)
{
    CFDictionaryRef	dhcp_dict = NULL;
    CFStringRef		key = NULL;
    CFDictionaryRef	primary_dict = NULL;

    if (serviceID == NULL) {
	/* get the primary service name */
	key = SCDynamicStoreKeyCreateNetworkGlobalEntity(NULL,
							 kSCDynamicStoreDomainState,
							 kSCEntNetIPv4);
	if (key) {
	    primary_dict = SCDynamicStoreCopyValue(store, key);
	    if (primary_dict) {
		serviceID = CFDictionaryGetValue(primary_dict,
						 kSCDynamicStorePropNetPrimaryService);
	    }
	    CFRelease(key);
	}
    }
    if (serviceID == NULL) {
	goto done;
    }
    key = SCDynamicStoreKeyCreateNetworkServiceEntity(NULL,
						      kSCDynamicStoreDomainState,
						      serviceID,
						      kSCEntNetDHCP);
    if (key) {
	dhcp_dict = SCDynamicStoreCopyValue(store, key);
	if (dhcp_dict != NULL
	    && isA_CFDictionary(dhcp_dict) == NULL) {
	    CFRelease(dhcp_dict);
	    dhcp_dict = NULL;
	}
	CFRelease(key);
    }
 done:
    if (primary_dict) {
	CFRelease(primary_dict);
    }
    return (dhcp_dict);
}

CFDataRef
DHCPInfoGetOptionData(CFDictionaryRef dhcp_dict, UInt8 code)
{
    CFDataRef		data = NULL;
    CFStringRef		option_code_str = NULL;

    option_code_str = CFStringCreateWithFormat(NULL, NULL,
					       CFSTR("Option_%d"), code);
    if (option_code_str == NULL) {
	goto done;
    }

    data = CFDictionaryGetValue(dhcp_dict, option_code_str);
    data = isA_CFData(data);
 done:
    if (option_code_str)
	CFRelease(option_code_str);
    return (data);
}

CFDateRef
DHCPInfoGetLeaseStartTime(CFDictionaryRef dhcp_dict)
{
    return (CFDictionaryGetValue(dhcp_dict, CFSTR("LeaseStartTime")));
}

CFDateRef
DHCPInfoGetLeaseExpirationTime(CFDictionaryRef dhcp_dict)
{
    return (CFDictionaryGetValue(dhcp_dict, CFSTR("LeaseExpirationTime")));
}

#ifdef TEST_DHCPCLIENT_PREFERENCES
void
print_data(u_char * data_p, int n_bytes)
{
#define CHARS_PER_LINE 	16
    char		line_buf[CHARS_PER_LINE + 1];
    int			line_pos;
    int			offset;

    for (line_pos = 0, offset = 0; offset < n_bytes; offset++, data_p++) {
	if (line_pos == 0)
	    printf("%04x ", offset);

	line_buf[line_pos] = isprint(*data_p) ? *data_p : '.';
	printf(" %02x", *data_p);
	line_pos++;
	if (line_pos == CHARS_PER_LINE) {
	    line_buf[CHARS_PER_LINE] = '\0';
	    printf("  %s\n", line_buf);
	    line_pos = 0;
	}
	else if (line_pos == (CHARS_PER_LINE / 2))
	    printf(" ");
    }
    if (line_pos) { /* need to finish up the line */
	for (; line_pos < CHARS_PER_LINE; line_pos++) {
	    printf("   ");
	    line_buf[line_pos] = ' ';
	}
	line_buf[CHARS_PER_LINE] = '\0';
	printf("  %s\n", line_buf);
    }
}

#define CMDSTR_GETOPTION	"getoption"
#define CMDSTR_LEASE		"leaseinfo"
#define CMDSTR_GETPARAMS	"getparams"
#define CMDSTR_SETPARAMS	"setparams"

static __inline__ void
S_print_char_array(UInt8 * params, int n_params)
{
    int i;

    for (i = 0; i < n_params; i++) {
	if (i == 0)
	    printf("%d", params[i]);
	else
	    printf(", %d", params[i]);
    }
    return;
}

void
usage(char * prog)
{
    printf("%s " CMDSTR_GETOPTION " <serviceID> <opt> [ <type> ]\n"
	   "%s " CMDSTR_LEASE " <serviceID>\n"
	   "%s " CMDSTR_GETPARAMS " <app_id>\n"
	   "%s " CMDSTR_SETPARAMS " <app_id> [ <opt> [ <opt> ] ... [ <opt> ] ] ]\n"
	   "where:\n"
	   "  <serviceID>     : service ID string | \"\"\n"
	   "  <opt>           : DHCP/BOOTP option code\n"
	   "                    (e.g. 1 == subnet mask, 3 == router, 6 = dns, 15 = domain)\n"
	   "  <type>          : type of option: string, ip\n"
	   "  <app_id>        : application id (e.g. com.apple.ntpd, com.thursby.Dave)\n",
	   prog, prog, prog, prog);
    exit(0);
}

static void
dump_gregorian_date(CFGregorianDate d)
{
    printf("%d/%d/%d %02d:%02d:%02d\n",
	   (int)d.year, d.month, d.day, d.hour, d.minute, (int)d.second);
    return;
}

static void
show_date(CFAbsoluteTime t)
{
    CFGregorianDate d;
    static CFTimeZoneRef tz = NULL;

    if (tz == NULL) {
	tz = CFTimeZoneCopySystem();
    }

    d = CFAbsoluteTimeGetGregorianDate(t, tz);
    dump_gregorian_date(d);
    return;
}

#define IP_FORMAT	"%d.%d.%d.%d"
#define IP_CH(ip, i)	(((u_char *)(ip))[i])
#define IP_LIST(ip)	IP_CH(ip,0),IP_CH(ip,1),IP_CH(ip,2),IP_CH(ip,3)

typedef enum {
    command_none_e,
    command_getoption_e,
    command_lease_e,
    command_setparams_e,
    command_getparams_e,
} command_t;

int
main(int argc, char * argv[])
{
    CFStringRef		app_id;
    command_t		command = command_none_e;
    char *		command_str;
    CFIndex		count;
    CFDictionaryRef	info;
    UInt8 * 		params;
    CFStringRef		serviceID = NULL;

    command_str = argv[1];
    if (argc < 2)
	usage(argv[0]);
    if (strcmp(command_str, CMDSTR_GETOPTION) == 0) {
	if (argc < 4 || argc > 5) {
	    usage(argv[0]);
	}
	command = command_getoption_e;
    }
    else if (strcmp(command_str, CMDSTR_LEASE) == 0) {
	if (argc != 3) {
	    usage(argv[0]);
	}
	command = command_lease_e;
    }
    else if (strcmp(command_str, CMDSTR_SETPARAMS) == 0) {
	command = command_setparams_e;
	if (argc < 3) {
	    usage(argv[0]);
	}
    }
    else if (strcmp(command_str, CMDSTR_GETPARAMS) == 0) {
	command = command_getparams_e;
	if (argc != 3) {
	    usage(argv[0]);
	}
    }
    else {
	usage(argv[0]);
    }

    switch (command) {
      case command_getoption_e: {
	  UInt8 	code;
	  char *	code_str;
	  CFDataRef	option;
	  boolean_t	printed = FALSE;
	  CFIndex	len;
	  char *	type = NULL;

	  if (argv[2][0]) {
	      serviceID = CFStringCreateWithFormat(NULL, NULL,
						   CFSTR("%s"), argv[2]);
	  }

	  info = SCDynamicStoreCopyDHCPInfo(NULL, serviceID);
	  if (info == NULL) {
	      exit(1);
	  }

	  code_str = argv[3];
	  if (argc > 4) {
	      type = argv[4];
	  }
	  code = atoi(code_str);

	  option = DHCPInfoGetOptionData(info, code);
	  if (option == NULL) {
	      exit(1);
	  }
	  len = CFDataGetLength(option);
	  if (type) {
	      printed = TRUE;
	      if (strcmp(type, "ip") == 0) {
		  int i = 0;
		  const void * ptr = CFDataGetBytePtr(option);

		  while (len >= 4) {
		      if (i == 0) {
			  printf(IP_FORMAT, IP_LIST(ptr));
		      }
		      else {
			  printf(" " IP_FORMAT, IP_LIST(ptr));
		      }
		      i++;
		      len -= 4;
		      ptr += 4;
		  }
		  printf("\n");
	      }
	      else if (strcmp(type, "string") == 0) {
		  printf("%.*s\n", (int)len, (char *)CFDataGetBytePtr(option));
	      }
	      else {
		  printed = FALSE;
	      }
	  }
	  if (printed == FALSE) {
	      print_data((void *)CFDataGetBytePtr(option), len);
	  }
	  if (serviceID)
	      CFRelease(serviceID);
	  CFRelease(info);
	  break;
      }
      case command_lease_e: {
	  CFDateRef	start;

	  if (argv[2][0]) {
	      serviceID = CFStringCreateWithFormat(NULL, NULL,
						   CFSTR("%s"), argv[2]);
	  }

	  info = SCDynamicStoreCopyDHCPInfo(NULL, serviceID);
	  if (info == NULL) {
	      exit(1);
	  }
	  start = DHCPInfoGetLeaseStartTime(info);

	  if (start) {
	      CFDataRef		option;
	      int32_t		lease;

#define OPTION_LEASE_TIME	51
#define SERVER_ID		54
	      option = DHCPInfoGetOptionData(info, OPTION_LEASE_TIME);
	      if (option == NULL) {
		  fprintf(stderr, "what, no lease time?\n");
		  exit(1);
	      }
	      printf("Lease start: ");
	      show_date(CFDateGetAbsoluteTime(start));
	      lease = ntohl(*((int32_t *)CFDataGetBytePtr(option)));
	      if (lease == 0xffffffff) {
		  printf("Lease is infinite\n");
	      }
	      else {
		  printf("Lease expires: ");
		  show_date(lease + CFDateGetAbsoluteTime(start));
	      }
	      option = DHCPInfoGetOptionData(info, SERVER_ID);
	      if (option) {
		  printf("Server IP: " IP_FORMAT "\n",
			 IP_LIST(CFDataGetBytePtr(option)));
	      }
	  }
	  else {
	      printf("no lease\n");
	  }
	  if (serviceID)
	      CFRelease(serviceID);
	  CFRelease(info);
	  break;
      }
      case command_getparams_e: {
	  app_id = CFStringCreateWithFormat(NULL, NULL,
					    CFSTR("%s"), argv[2]);
	  params = DHCPClientPreferencesCopyApplicationOptions(app_id, &count);
	  if (params) {
	      printf("%s params = {", argv[2]);
	      S_print_char_array(params, count);
	      printf("}\n");
	      free(params);
	  }
	  break;
      }
      case command_setparams_e: {
	  int		count = 0;
	  UInt8 * 	options = NULL;

	  if (argc > 3) {
	      int i;

	      count = argc - 3;
	      options = malloc(count);
	      if (options == NULL) {
		  fprintf(stderr, "malloc failed %s\n",
			  strerror(errno));
		  exit(1);
	      }
	      for (i = 0; i < count; i++) {
		  options[i] = atoi(argv[3 + i]);
	      }
	  }
	  app_id = CFStringCreateWithFormat(NULL, NULL,
					    CFSTR("%s"), argv[2]);
	  if (DHCPClientPreferencesSetApplicationOptions(app_id, options,
							 count) == FALSE) {
	      printf("operation failed\n");
	  }
	  if (options) {
	      free(options);
	  }
	  break;
      }
      default:
	  break;
    }
    exit(0);
    return(0);
}
#endif	// TEST_DHCPCLIENT_PREFERENCES


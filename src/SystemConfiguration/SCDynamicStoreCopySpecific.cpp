#include <unistd.h>
#include <sys/types.h>
#include <stdlib.h>
#include <pwd.h>
#include <sys/utsname.h>
#include "log.h"

#include <CoreFoundation/CFString.h>
#include <SystemConfiguration/SCDynamicStoreCopySpecific.h>

#define MAX_PASSENTRY_LENGTH 4096

const CFStringRef kSCPropNetProxiesExceptionsList 	= CFSTR("ExceptionsList");
const CFStringRef kSCPropNetProxiesHTTPEnable 		= CFSTR("HTTPEnable");
const CFStringRef kSCPropNetProxiesHTTPPort 		= CFSTR("HTTPPort");
const CFStringRef kSCPropNetProxiesHTTPSProxy 		= CFSTR("HTTPSProxy");
const CFStringRef kSCPropNetProxiesHTTPSPort 		= CFSTR("HTTPSPort");
const CFStringRef kSCPropNetProxiesFTPEnable 		= CFSTR("FTPEnable");
const CFStringRef kSCPropNetProxiesFTPProxy 		= CFSTR("FTPProxy");
const CFStringRef kSCPropNetProxiesFTPPort 		= CFSTR("FTPPort");
const CFStringRef kSCPropNetProxiesFTPPassive 		= CFSTR("FTPPassive");

CFStringRef SCDynamicStoreCopyComputerName (SCDynamicStoreRef store, CFStringEncoding *nameEncoding){
	LOG << "fixme: SCDynamicStoreCopyComputerName() - stub" << std::endl;
	return NULL;
}

CFStringRef SCDynamicStoreCopyConsoleUser (SCDynamicStoreRef store, uid_t *uid, gid_t *gid){
	struct passwd pwent, *pwentp;
	char buffer[MAX_PASSENTRY_LENGTH];
	char *user;

	if (uid) *uid = getuid();
	if (gid) *gid = getgid();

	user = getenv("USER");
	if (user)
		return CFStringCreateWithCString(NULL, user, kCFStringEncodingUTF8);

	if (getpwuid_r(getuid(), &pwent, buffer, MAX_PASSENTRY_LENGTH, &pwentp))
		return CFStringCreateWithCString(NULL, pwentp->pw_name, kCFStringEncodingUTF8);

	return NULL;
}

CFStringRef SCDynamicStoreCopyLocalHostName (SCDynamicStoreRef store){
	struct utsname info;

	if (uname(&info) != 0)
		return NULL;

	return CFStringCreateWithCString(NULL, info.nodename, kCFStringEncodingUTF8);
}

CFStringRef SCDynamicStoreCopyLocation (SCDynamicStoreRef store){
	LOG << "fixme: SCDynamicStoreCopyLocation() - stub" << std::endl;
	return NULL;
}

CFDictionaryRef SCDynamicStoreCopyProxies (SCDynamicStoreRef store){
	LOG << "fixme: SCDynamicStoreCopyComputerName() - stub" << std::endl;
	/* No Proxy support yet */
	return NULL;
}

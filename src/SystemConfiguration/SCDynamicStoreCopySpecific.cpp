#include <unistd.h>
#include <sys/types.h>
#include <stdlib.h>
#include <pwd.h>
#include <sys/utsname.h>
//#include "log.h"

#include <CoreFoundation/CFString.h>
#include <SystemConfiguration/SCDynamicStoreCopySpecific.h>
#include <SystemConfiguration/SCSchemaDefinitions.h>
#include <vector>

#define MAX_PASSENTRY_LENGTH 4096



CFStringRef SCDynamicStoreCopyComputerName (SCDynamicStoreRef store, CFStringEncoding *nameEncoding)
{
	char hostname[100];
	if (gethostname(hostname, sizeof(hostname)) != 0)
		return nullptr;
	hostname[sizeof(hostname)-1] = '\0';

	if (nameEncoding)
		*nameEncoding = kCFStringEncodingUTF8;
	
	return CFStringCreateWithCString(nullptr, hostname, kCFStringEncodingUTF8);
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
	//LOG << "fixme: SCDynamicStoreCopyLocation() - stub" << std::endl;
	return NULL;
}

static CFNumberRef cfNumber(int n)
{
	return CFNumberCreate(nullptr, kCFNumberSInt32Type, &n);
}

static void fillProxy(std::vector<CFStringRef>& keys, std::vector<CFTypeRef>& values,
	const char* envVarName,
	CFStringRef enabledKey, CFStringRef hostKey, CFStringRef portKey)
{
	const char* var = getenv(envVarName);
	if (!var)
	{
		keys.push_back(enabledKey);
		values.push_back(cfNumber(0));
	}
	else
	{
		keys.push_back(enabledKey);
		values.push_back(cfNumber(1));

		CFURLRef url = CFURLCreateWithBytes(nullptr, (const UInt8*) var, strlen(var), kCFStringEncodingUTF8, nullptr);

		if (url != nullptr)
		{
			keys.push_back(hostKey);
			values.push_back(CFURLCopyHostName(url));

			int port = CFURLGetPortNumber(url);
			if (port == -1)
				port = 80;

			keys.push_back(portKey);
			values.push_back(cfNumber(port));

			CFRelease(url);
		}
	}
}

CFDictionaryRef SCDynamicStoreCopyProxies (SCDynamicStoreRef store)
{
	// NOTE: Proxy username and password are supposed to be accessed via Security.framework
	// https://stackoverflow.com/a/15085066/479753

	std::vector<CFStringRef> keys;
	std::vector<CFTypeRef> values;

	keys.push_back(kSCPropNetProxiesGopherEnable);
	values.push_back(cfNumber(0));

	fillProxy(keys, values, "http_proxy", kSCPropNetProxiesHTTPEnable, kSCPropNetProxiesHTTPProxy, kSCPropNetProxiesHTTPPort);
	fillProxy(keys, values, "https_proxy", kSCPropNetProxiesHTTPSEnable, kSCPropNetProxiesHTTPSProxy, kSCPropNetProxiesHTTPSPort);
	fillProxy(keys, values, "ftp_proxy", kSCPropNetProxiesFTPEnable, kSCPropNetProxiesFTPProxy, kSCPropNetProxiesFTPPort);

	CFDictionaryRef rv = CFDictionaryCreate(nullptr, (const void**) keys.data(), (const void**) values.data(), keys.size(),
		&kCFTypeDictionaryKeyCallBacks, &kCFTypeDictionaryValueCallBacks);

	for (CFTypeRef cf : values)
		CFRelease(cf);
	return rv;
}

CFDictionaryRef SCDynamicStoreCopyProxiesWithOptions(SCDynamicStoreRef store, CFDictionaryRef options)
{
	return SCDynamicStoreCopyProxies(store);
}

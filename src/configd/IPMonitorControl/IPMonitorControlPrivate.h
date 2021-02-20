
#ifndef _IPMONITOR_CONTROL_PRIVATE_H
#define _IPMONITOR_CONTROL_PRIVATE_H

#define kIPMonitorControlServerName \
    "com.apple.SystemConfiguration.IPMonitorControl"

typedef CF_ENUM(uint32_t, IPMonitorControlRequestType) {
    kIPMonitorControlRequestTypeNone			= 0,
    kIPMonitorControlRequestTypeSetInterfaceRank	= 1,
    kIPMonitorControlRequestTypeGetInterfaceRank	= 2,
    kIPMonitorControlRequestTypeSetInterfaceAdvisory	= 3,
    kIPMonitorControlRequestTypeInterfaceAdvisoryIsSet	= 4,
    kIPMonitorControlRequestTypeAnyInterfaceAdvisoryIsSet = 5,
};

/*
 * kIPMonitorControlRequestKey*
 * - keys used to communicate a request to the server
 */
#define kIPMonitorControlRequestKeyType			"Type"
#define kIPMonitorControlRequestKeyProcessName		"ProcessName"
#define kIPMonitorControlRequestKeyInterfaceName	"InterfaceName"
#define kIPMonitorControlRequestKeyPrimaryRank		"PrimaryRank"
#define kIPMonitorControlRequestKeyAdvisory		"Advisory"
#define kIPMonitorControlRequestKeyReason		"Reason"

/*
 * kIPMonitorControlResponseKey*
 * - keys used to communicate the response from the server
 */
#define kIPMonitorControlResponseKeyError		"Error"
#define kIPMonitorControlResponseKeyPrimaryRank		"PrimaryRank"
#define kIPMonitorControlResponseKeyAdvisoryIsSet	"AdvisoryIsSet"

static inline CFStringRef
_IPMonitorControlCopyInterfaceAdvisoryNotificationKey(CFStringRef ifname)
{
    return SCDynamicStoreKeyCreateNetworkInterfaceEntity(NULL,
							 kSCDynamicStoreDomainState,
							 ifname,
							 CFSTR("Advisory"));
}

static inline void
my_CFRelease(void * t)
{
    void * * obj = (void * *)t;
    if (obj && *obj) {
	CFRelease(*obj);
	*obj = NULL;
    }
    return;
}

#endif /* _IPMONITOR_CONTROL_PRIVATE_H */

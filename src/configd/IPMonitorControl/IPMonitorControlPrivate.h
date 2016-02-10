
#ifndef _IPMONITOR_CONTROL_PRIVATE_H
#define _IPMONITOR_CONTROL_PRIVATE_H

#define kIPMonitorControlServerName \
    "com.apple.SystemConfiguration.IPMonitorControl"

typedef CF_ENUM(uint32_t, IPMonitorControlRequestType) {
    kIPMonitorControlRequestTypeNone			= 0,
    kIPMonitorControlRequestTypeSetInterfaceRank	= 1,
    kIPMonitorControlRequestTypeGetInterfaceRank	= 2,
};

/*
 * kIPMonitorControlRequestKey*
 * - keys used to communicate a request to the server
 */
#define kIPMonitorControlRequestKeyType			"Type"
#define kIPMonitorControlRequestKeyInterfaceName	"InterfaceName"
#define kIPMonitorControlRequestKeyPrimaryRank		"PrimaryRank"

/*
 * kIPMonitorControlResponseKey*
 * - keys used to communicate the response from the server
 */
#define kIPMonitorControlResponseKeyError		"Error"
#define kIPMonitorControlResponseKeyPrimaryRank		"PrimaryRank"

#endif /* _IPMONITOR_CONTROL_PRIVATE_H */

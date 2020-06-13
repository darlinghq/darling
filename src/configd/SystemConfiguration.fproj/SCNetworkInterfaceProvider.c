/*
 * Copyright (c) 2018 Apple Inc. All rights reserved.
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
 * January 17, 2018		Dieter Siegmund (dieter@apple.com)
 * - initial revision
 */

/*
 * SCNetworkInterfaceProvider.c
 */

#include <CoreFoundation/CoreFoundation.h>
#include <CoreFoundation/CFRuntime.h>
#include <libkern/OSAtomic.h>
#include "SCNetworkConfigurationPrivate.h"
#include "SCNetworkConfigurationInternal.h"
#include "SCNetworkInterfaceProvider.h"

static void
my_CFRelease(void * t)
{
    void * * obj = (void * *)t;
    if (obj && *obj) {
	CFRelease(*obj);
	*obj = NULL;
    }
    return;
}

/**
 ** ObjectWrapper
 **/

typedef struct {
    const void *	obj;
    int32_t		retain_count;
} ObjectWrapper, * ObjectWrapperRef;

static const void *
ObjectWrapperRetain(const void * info)
{
    ObjectWrapperRef 	wrapper = (ObjectWrapperRef)info;

    (void)OSAtomicIncrement32(&wrapper->retain_count);
    return (info);
}

static ObjectWrapperRef
ObjectWrapperAllocate(const void * obj)
{
    ObjectWrapperRef	wrapper;

    wrapper = (ObjectWrapperRef)malloc(sizeof(*wrapper));
    wrapper->obj = obj;
    wrapper->retain_count = 1;
    return (wrapper);
}

static void
ObjectWrapperRelease(const void * info)
{
    int32_t		new_val;
    ObjectWrapperRef 	wrapper = (ObjectWrapperRef)info;

    new_val = OSAtomicDecrement32(&wrapper->retain_count);
    if (new_val == 0) {
	free(wrapper);
    }
    else {
	assert(new_val > 0);
    }
    return;
}

static void
ObjectWrapperSetObject(ObjectWrapperRef wrapper, const void * obj)
{
    wrapper->obj = obj;
}

static const void *
ObjectWrapperGetObject(ObjectWrapperRef wrapper)
{
    return (wrapper->obj);
}

static SCDynamicStoreRef
StoreObjectWrapperAllocate(const void * obj,
			   CFStringRef name,
			   SCDynamicStoreCallBack handler,
			   CFArrayRef keys,
			   CFArrayRef patterns,
			   dispatch_queue_t queue,
			   ObjectWrapperRef * ret_wrapper)
{
    SCDynamicStoreContext	context = {
	.version = 0,
	.info = NULL,
	.retain = ObjectWrapperRetain,
	.release = ObjectWrapperRelease,
	.copyDescription = NULL
    };
    SCDynamicStoreRef		store;
    ObjectWrapperRef		wrapper;

    wrapper = ObjectWrapperAllocate(obj);
    context.info = wrapper;
    store = SCDynamicStoreCreate(NULL, name, handler, &context);
    if (store == NULL) {
	SC_log(LOG_NOTICE,
	       "%@: SCDynamicStoreCreate failed", name);
    }
    else if (!SCDynamicStoreSetNotificationKeys(store, keys, patterns)) {
	SC_log(LOG_NOTICE,
	       "%@: SCDynamicStoreSetNoticationKeys failed", name);
	CFRelease(store);
	store = NULL;
    }
    else if (queue != NULL
	     && !SCDynamicStoreSetDispatchQueue(store, queue)) {
	SC_log(LOG_NOTICE,
	       "%@: SCDynamicStoreSetDispatchQueue failed", name);
	CFRelease(store);
	store = NULL;
    }
    if (store == NULL) {
	ObjectWrapperRelease(wrapper);
	wrapper = NULL;
    }
    *ret_wrapper = wrapper;
    return (store);
}

/**
 ** CF object glue code
 **/
static CFStringRef	__SCNetworkInterfaceProviderCopyDebugDesc(CFTypeRef cf);
static void		__SCNetworkInterfaceProviderDeallocate(CFTypeRef cf);

static CFTypeID __kSCNetworkInterfaceProviderTypeID	= _kCFRuntimeNotATypeID;

static const CFRuntimeClass __SCNetworkInterfaceProviderClass = {
    0,						/* version */
    "SCNetworkInterfaceProvider",		/* className */
    NULL,					/* init */
    NULL,					/* copy */
    __SCNetworkInterfaceProviderDeallocate,	/* deallocate */
    NULL,					/* equal */
    NULL,					/* hash */
    NULL,					/* copyFormattingDesc */
    __SCNetworkInterfaceProviderCopyDebugDesc	/* copyDebugDesc */
};

struct __SCNetworkInterfaceProvider {
    CFRuntimeBase				cf_base;

    IPMonitorControlRef				control;
    SCDynamicStoreRef				store;
    ObjectWrapperRef				wrapper;
    dispatch_queue_t				queue;

    SCNetworkInterfaceProviderEventHandler	handler;
    CFStringRef					if_name;
    SCNetworkInterfaceRef			if_type;
    Boolean					enabled;
    Boolean					needed;
};


static CFStringRef
__SCNetworkInterfaceProviderCopyDebugDesc(CFTypeRef cf)
{
    CFAllocatorRef		allocator = CFGetAllocator(cf);
    SCNetworkInterfaceProviderRef provider = (SCNetworkInterfaceProviderRef)cf;
    CFMutableStringRef		result;

    result = CFStringCreateMutable(allocator, 0);
    CFStringAppendFormat(result, NULL,
			 CFSTR("<SCNetworkInterfaceProvider %@ %@ <%p>"),
			 provider->if_type, provider->if_name, cf);
    return (result);
}

static void
SCNetworkInterfaceProviderDeallocate(SCNetworkInterfaceProviderRef provider)
{
    provider->enabled = FALSE;
    my_CFRelease(&provider->control);
    if (provider->wrapper != NULL) {
	ObjectWrapperSetObject(provider->wrapper, NULL);
	ObjectWrapperRelease(provider->wrapper);
	provider->wrapper = NULL;
    }
    if (provider->store != NULL) {
	SCDynamicStoreSetDispatchQueue(provider->store, NULL);
	my_CFRelease(&provider->store);
    }
    if (provider->queue != NULL) {
	dispatch_release(provider->queue);
	provider->queue = NULL;
    }
    if (provider->handler != NULL) {
	Block_release(provider->handler);
	provider->handler = NULL;
    }
    my_CFRelease(&provider->if_name);
    my_CFRelease(&provider->if_type);
}

static void
__SCNetworkInterfaceProviderDeallocate(CFTypeRef cf)
{
    SCNetworkInterfaceProviderRef provider = (SCNetworkInterfaceProviderRef)cf;

    if (provider->queue != NULL) {
	dispatch_sync(provider->queue, ^{
		SCNetworkInterfaceProviderDeallocate(provider);
	    });
    }
    else {
	SCNetworkInterfaceProviderDeallocate(provider);
    }
    return;
}

/**
 ** Supporting Functions
 **/
static void
__SCNetworkInterfaceProviderRegisterClass(void)
{
    static dispatch_once_t	once;
    dispatch_block_t		once_block;

    once_block = ^{
	__kSCNetworkInterfaceProviderTypeID
	= _CFRuntimeRegisterClass(&__SCNetworkInterfaceProviderClass);
    };
    dispatch_once(&once, once_block);
    return;
}

static SCNetworkInterfaceProviderRef
__SCNetworkInterfaceProviderAllocate(CFAllocatorRef allocator)
{
    SCNetworkInterfaceProviderRef	provider;
    int					size;

    __SCNetworkInterfaceProviderRegisterClass();
    size = sizeof(*provider) - sizeof(CFRuntimeBase);
    provider = (SCNetworkInterfaceProviderRef)
	_CFRuntimeCreateInstance(allocator,
				 __kSCNetworkInterfaceProviderTypeID,
				 size, NULL);
    memset(((void *)provider) + sizeof(CFRuntimeBase), 0, size);
    return (provider);
}

static void
SCNetworkInterfaceProviderCheck(SCNetworkInterfaceProviderRef provider)
{
    Boolean	advisory_set;

    if (!provider->enabled || provider->handler == NULL) {
	return;
    }
    advisory_set
	= IPMonitorControlAnyInterfaceAdvisoryIsSet(provider->control);
    if (provider->needed != advisory_set) {
	SCNetworkInterfaceProviderEvent		event;

	event = advisory_set
	    ? kSCNetworkInterfaceProviderEventActivationRequested
	    : kSCNetworkInterfaceProviderEventActivationNoLongerRequested;
	(provider->handler)(event, NULL);
	provider->needed = advisory_set;
    }
    return;
}

static void
StoreHandleChanges(SCDynamicStoreRef store, CFArrayRef changes, void * info)
{
#pragma unused(store)
#pragma unused(changes)
    SCNetworkInterfaceProviderRef	provider;
    ObjectWrapperRef			wrapper = (ObjectWrapperRef)info;

    provider = (SCNetworkInterfaceProviderRef)ObjectWrapperGetObject(wrapper);
    if (provider == NULL) {
	/* provider has been deallocated */
	return;
    }
    SCNetworkInterfaceProviderCheck(provider);
    return;
}


/**
 ** SCNetworkInterfaceProvider SPI
 **/
SCNetworkInterfaceProviderRef
SCNetworkInterfaceProviderCreate(CFStringRef type,
				 CFStringRef ifname,
				 CFDictionaryRef options)
{
    IPMonitorControlRef			control;
    CFStringRef				pattern;
    CFArrayRef				patterns;
    SCNetworkInterfaceProviderRef	provider;
    dispatch_queue_t			queue;
    SCDynamicStoreRef			store = NULL;
    ObjectWrapperRef			wrapper = NULL;

    if (options != NULL || ifname == NULL || type == NULL) {
	_SCErrorSet(kSCStatusInvalidArgument);
	return (NULL);
    }
    control = IPMonitorControlCreate();
    if (control == NULL) {
	_SCErrorSet(kSCStatusFailed);
	return (NULL);
    }
    pattern
	= IPMonitorControlCopyInterfaceAdvisoryNotificationKey(kSCCompAnyRegex);
    patterns = CFArrayCreate(NULL, (const void * *)&pattern, 1,
			     &kCFTypeArrayCallBacks);
    CFRelease(pattern);
#define OUR_NAME	"SCNetworkInterfaceProvider"
    queue = dispatch_queue_create(OUR_NAME, NULL);
    provider = __SCNetworkInterfaceProviderAllocate(NULL);
    store = StoreObjectWrapperAllocate(provider,
				       CFSTR(OUR_NAME),
				       StoreHandleChanges,
				       NULL,
				       patterns,
				       queue,
				       &wrapper);
    CFRelease(patterns);
    if (store == NULL) {
	dispatch_release(queue);
	CFRelease(provider);
	provider = NULL;
	CFRelease(control);
    }
    else {
	provider->control = control;
	provider->store = store;
	provider->wrapper = wrapper;
	provider->queue = queue;
	provider->if_name = CFRetain(ifname);
	provider->if_type = CFRetain(type);
    }
    return (provider);
}

void
SCNetworkInterfaceProviderSetEventHandler(SCNetworkInterfaceProviderRef provider,
					  SCNetworkInterfaceProviderEventHandler handler)
{
    if (handler == NULL) {
	/* can't clear handler once set */
	return;
    }
    dispatch_sync(provider->queue, ^{
	    if (provider->enabled) {
		/* enabling before setting the handler isn't allowed */
		SC_log(LOG_NOTICE,
		       "%s: call SCNetworkInterfaceSetEventHandler before "
		       " SCNetworkInterfaceProviderResume", __FUNCTION__);
		return;
	    }
	    if (provider->handler != NULL) {
		/* can't change the handler once set */
		SC_log(LOG_NOTICE,
		       "%s: ignoring second invocation of "
		       "SCNetworkInterfaceSetEventHandler", __FUNCTION__);
		return;
	    }
	    provider->handler = Block_copy(handler);
	});
    return;
}

void
SCNetworkInterfaceProviderResume(SCNetworkInterfaceProviderRef provider)
{
    dispatch_async(provider->queue, ^{
	    if (!provider->enabled) {
		provider->enabled = TRUE;
		SCNetworkInterfaceProviderCheck(provider);
	    }
	});
    return;
}

#if TEST_SCNetworkInterfaceProvider

/*
  xcrun -sdk iphoneos.internal cc -o scnip SCNetworkInterfaceProvider.c -DTEST_SCNetworkInterfaceProvider -framework CoreFoundation -framework SystemConfiguration -arch arm64 -I ../IPMonitorControl ../IPMonitorControl/IPMonitorControl.c -DSC_LOG_HANDLE=__log_SCNetworkInterfaceProvider
*/

__private_extern__ os_log_t
__log_SCNetworkInterfaceProvider(void)
{
	static os_log_t	log	= NULL;

	if (log == NULL) {
		log = os_log_create("com.apple.SystemConfiguration", "SCNetworkConfiguration");
	}

	return log;
}

static void
event_handler(SCNetworkInterfaceProviderRef provider,
	      SCNetworkInterfaceProviderEvent event,
	      CFDictionaryRef event_data)
{
    printf("<%p> event %d\n", provider, event);
}

int
main(int argc, char * argv[])
{
    SCNetworkInterfaceProviderEventHandler	handler;
    SCNetworkInterfaceProviderRef		provider;

    provider
	= SCNetworkInterfaceProviderCreate(kSCNetworkInterfaceTypeWWAN,
					   CFSTR("pdp_ip10"),
					   NULL);
    if (provider == NULL) {
	fprintf(stderr, "SCNetworkInterfaceProviderCreate failed\n");
	exit(1);
    }
    handler = ^(SCNetworkInterfaceProviderEvent event,
		CFDictionaryRef event_data) {
	event_handler(provider, event, event_data);
    };
    SCNetworkInterfaceProviderSetEventHandler(provider, handler);
    SCNetworkInterfaceProviderResume(provider);
    dispatch_main();
    exit(0);
    return (0);
}
#endif

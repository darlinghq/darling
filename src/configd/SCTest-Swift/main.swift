/*
* Copyright (c) 2015, 2018 Apple Inc. All rights reserved.
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
* April 21, 2015		Sushant Chavan
* - initial revision
*/

/*
*  A Swift test target to test SC APIs
*/

import Foundation
import SystemConfiguration
import SystemConfiguration_Private

let target_host = "www.apple.com"
var application = "SCTest-Swift" as CFString

#if	!targetEnvironment(simulator)
func
	test_SCDynamicStore ()
{
	//SCDynamicStore APIs
	NSLog("\n\n*** SCDynamicStore ***\n\n")
	let key:CFString
	let store:SCDynamicStore?
	let dict:[String:String]?
	let primaryIntf:String?

	store = SCDynamicStoreCreate(nil, application, nil, nil)
	if store == nil {
		NSLog("Error creating session: %s", SCErrorString(SCError()))
		return
	}

	key =	SCDynamicStoreKeyCreateNetworkGlobalEntity(nil, kSCDynamicStoreDomainState, kSCEntNetIPv4)
	dict =	SCDynamicStoreCopyValue(store, key) as? [String:String]
	primaryIntf = dict?[kSCDynamicStorePropNetPrimaryInterface as String]
	if (primaryIntf != nil) {
		NSLog("Primary interface is %@", primaryIntf!)
	} else {
		NSLog("Primary interface is unavailable")
	}
}
#endif	// !targetEnvironment(simulator)

#if	!targetEnvironment(simulator)
func
test_SCNetworkConfiguration ()
{
	//SCNetworkConfiguration APIs
	NSLog("\n\n*** SCNetworkConfiguration ***\n\n")
	let interfaceArray:[CFArray]

	NSLog("Network Interfaces:")
	interfaceArray = SCNetworkInterfaceCopyAll() as! [CFArray]
	for idx in 0...interfaceArray.count {
		let interface = interfaceArray[idx]
		if let bsdName = SCNetworkInterfaceGetBSDName(interface as! SCNetworkInterface) {
			NSLog("- %@", bsdName as String)
		}
	}
}
#endif	// !targetEnvironment(simulator)

func
test_SCNetworkReachability ()
{
	//SCNetworkReachability APIs
	NSLog("\n\n*** SCNetworkReachability ***\n\n")
	let target:SCNetworkReachability?
	var flags:SCNetworkReachabilityFlags = SCNetworkReachabilityFlags.init(rawValue: 0)
	
	target = SCNetworkReachabilityCreateWithName(nil, target_host)
	if target == nil {
		NSLog("Error creating target: %s", SCErrorString(SCError()))
		return
	}
	
	SCNetworkReachabilityGetFlags(target!, &flags)
	NSLog("SCNetworkReachability flags for %@ is %#x", String(target_host), flags.rawValue)
}

#if	!targetEnvironment(simulator)
func
test_SCPreferences ()
{
	//SCPreferences APIs
	NSLog("\n\n*** SCPreferences ***\n\n")
	let prefs:SCPreferences?
	let networkServices:[CFArray]?

	prefs = SCPreferencesCreate(nil, application, nil)
	if prefs == nil {
		NSLog("Error creating prefs: %s", SCErrorString(SCError()))
		return
	}
	
	if let model = SCPreferencesGetValue(prefs!, "Model" as CFString) {
		NSLog("Current system model is %@", model as! String)
	}
	
	networkServices	= SCNetworkServiceCopyAll(prefs!) as? [CFArray]
	if networkServices == nil {
		NSLog("Error retrieving network services", SCErrorString(SCError()))
		return
	}
	
	NSLog("Network Services:")
	for idx in 0...(networkServices?.count)! {
		let service	= networkServices?[idx]
		if let serviceName = SCNetworkServiceGetName(service as! SCNetworkService) {
			NSLog("- %@", serviceName as String)
		}
		
	}
}
#endif	// !targetEnvironment(simulator)

func
my_main ()
{

#if	!targetEnvironment(simulator)
	test_SCDynamicStore()
#endif	// !targetEnvironment(simulator)

#if	!targetEnvironment(simulator)
	test_SCNetworkConfiguration()
#endif	// !targetEnvironment(simulator)

	test_SCNetworkReachability()

#if	!targetEnvironment(simulator)
	test_SCPreferences()
#endif	// !targetEnvironment(simulator)

}

// Run the test
my_main()

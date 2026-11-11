/*
 This file is part of Darling.

 Copyright (C) 2026 Darling Team

 Darling is free software: you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation, either version 3 of the License, or
 (at your option) any later version.

 Darling is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details.

 You should have received a copy of the GNU General Public License
 along with Darling.  If not, see <http://www.gnu.org/licenses/>.
*/

#ifndef __SERVICE_MANAGEMENT_APP_SERVICE__
#define __SERVICE_MANAGEMENT_APP_SERVICE__

#ifdef __OBJC__

#import <xpc/xpc.h>
#import <Foundation/Foundation.h>

NS_ASSUME_NONNULL_BEGIN

typedef NS_ENUM(NSInteger, SMAppServiceStatus) {
	SMAppServiceStatusNotRegistered,
	SMAppServiceStatusEnabled,
	SMAppServiceStatusRequiresApproval,
	SMAppServiceStatusNotFound,
} NS_SWIFT_NAME(SMAppService.Status) API_AVAILABLE(macos(13.0));

XPC_EXPORT
API_AVAILABLE(macos(15.0))
NSString * const SMAppServiceErrorDomain;

XPC_EXPORT
API_AVAILABLE(macos(13.0))
@interface SMAppService : NSObject {
	int _kind;
	NSString *_name;
}

+ (instancetype)loginItemServiceWithIdentifier:(NSString *)identifier NS_SWIFT_NAME(loginItem(identifier:));

@property (class, readonly) SMAppService *mainAppService NS_SWIFT_NAME(mainApp);

+ (instancetype)agentServiceWithPlistName:(NSString *)plistName NS_SWIFT_NAME(agent(plistName:));

+ (instancetype)daemonServiceWithPlistName:(NSString *)plistName NS_SWIFT_NAME(daemon(plistName:));

- (BOOL)registerAndReturnError:(NSError * _Nullable __autoreleasing * _Nullable)error;

- (BOOL)unregisterAndReturnError:(NSError * _Nullable __autoreleasing * _Nullable)error;

- (void)unregisterWithCompletionHandler:(void (^)(NSError * _Nullable error))handler;

@property (readonly) SMAppServiceStatus status;

+ (SMAppServiceStatus)statusForLegacyURL:(NSURL *)url NS_SWIFT_NAME(statusForLegacyPlist(at:));

+ (void)openSystemSettingsLoginItems;

@end

NS_ASSUME_NONNULL_END

#endif // __OBJC__

#endif

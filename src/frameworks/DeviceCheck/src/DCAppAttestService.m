/*
 This file is part of Darling.

 Copyright (C) 2026 Lubos Dolezel

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

#import <DeviceCheck/DCAppAttestService.h>
#import <dispatch/dispatch.h>

NSErrorDomain const DCErrorDomain = @"com.apple.devicecheck.error";

static NSError *DCMakeError(DCError code)
{
    return [NSError errorWithDomain:DCErrorDomain code:code userInfo:nil];
}

@implementation DCAppAttestService

+ (DCAppAttestService *)sharedService {
    static DCAppAttestService *service = nil;
    static dispatch_once_t onceToken;
    dispatch_once(&onceToken, ^{
        service = [[DCAppAttestService alloc] init];
    });
    return service;
}

- (BOOL)isSupported {
    return NO;
}

// The handlers run asynchronously on a global queue, never on the calling thread.
- (void)generateKeyWithCompletionHandler:(void (^)(NSString *, NSError *))completionHandler {
    void (^handler)(NSString *, NSError *) = [completionHandler copy];
    dispatch_async(dispatch_get_global_queue(DISPATCH_QUEUE_PRIORITY_DEFAULT, 0), ^{
        handler(nil, DCMakeError(DCErrorFeatureUnsupported));
        [handler release];
    });
}

// QUIRK: attestation and assertion requests fail with DCErrorInvalidInput, not
// DCErrorFeatureUnsupported, whatever key identifier and hash they carry (even
// well-formed ones), because no key can exist on a system where App Attest is
// unsupported. Not documented. Evidence: black-box observation on macOS 26.7.3
// (tier 2), on a machine where isSupported is NO. Test: the attest and assertion checks in
// (devicecheck_appattest.m).
- (void)attestKey:(NSString *)keyId clientDataHash:(NSData *)clientDataHash completionHandler:(void (^)(NSData *, NSError *))completionHandler {
    void (^handler)(NSData *, NSError *) = [completionHandler copy];
    dispatch_async(dispatch_get_global_queue(DISPATCH_QUEUE_PRIORITY_DEFAULT, 0), ^{
        handler(nil, DCMakeError(DCErrorInvalidInput));
        [handler release];
    });
}

- (void)generateAssertion:(NSString *)keyId clientDataHash:(NSData *)clientDataHash completionHandler:(void (^)(NSData *, NSError *))completionHandler {
    void (^handler)(NSData *, NSError *) = [completionHandler copy];
    dispatch_async(dispatch_get_global_queue(DISPATCH_QUEUE_PRIORITY_DEFAULT, 0), ^{
        handler(nil, DCMakeError(DCErrorInvalidInput));
        [handler release];
    });
}

@end

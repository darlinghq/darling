/*
  This file is part of Darling.

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

#import <Foundation/Foundation.h>

// Declared inline to match the other private frameworks here: none of them import their own
// header, and the include/ copy is the consumer-facing surface.
extern NSString *const CPAnalyticsPayloadClassNameKey;
extern NSString *const CPAnalyticsPayloadViewIDKey;

void CPAnalyticsEventViewDidAppear(id view, id payload);
void CPAnalyticsEventViewDidDisappear(id view, id payload);

@interface CPAnalytics : NSObject
+ (instancetype) sharedInstance;
@end

NSString *const CPAnalyticsPayloadClassNameKey = @"CPAnalyticsPayloadClassNameKey";
NSString *const CPAnalyticsPayloadViewIDKey = @"CPAnalyticsPayloadViewIDKey";

// The argument lists are the shape Photo Booth binds; nothing here dereferences them, so a
// caller that passes a different arity cannot be hurt by the mismatch.
void CPAnalyticsEventViewDidAppear(id view, id payload)
{
}

void CPAnalyticsEventViewDidDisappear(id view, id payload)
{
}

@implementation CPAnalytics

+ (instancetype) sharedInstance
{
    static CPAnalytics *sharedInstance;
    static dispatch_once_t onceToken;
    dispatch_once(&onceToken, ^{
        sharedInstance = [[CPAnalytics alloc] init];
    });
    return sharedInstance;
}

// Photo Booth only needs the class to exist, but it may call anything on it. Answer with a
// catch-all rather than letting an unimplemented class method abort the app during nib
// loading, and log so the gap stays visible.
+ (NSMethodSignature *) methodSignatureForSelector: (SEL) aSelector
{
    return [NSMethodSignature signatureWithObjCTypes: "v@:"];
}

+ (void) forwardInvocation: (NSInvocation *) anInvocation
{
    NSLog(@"Stub called: %@ in %@ (class method)",
          NSStringFromSelector([anInvocation selector]), [self class]);
}

- (NSMethodSignature *) methodSignatureForSelector: (SEL) aSelector
{
    return [NSMethodSignature signatureWithObjCTypes: "v@:"];
}

- (void) forwardInvocation: (NSInvocation *) anInvocation
{
    NSLog(@"Stub called: %@ in %@", NSStringFromSelector([anInvocation selector]), [self class]);
}

@end
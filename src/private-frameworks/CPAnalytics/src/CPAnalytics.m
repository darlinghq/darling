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

// The argument lists follow the usual shape of these calls; nothing here dereferences them, so a
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

// Callers may send any message to this class. Answer every selector with a no-op rather than
// letting an unimplemented method raise an exception.
+ (NSMethodSignature *) methodSignatureForSelector: (SEL) aSelector
{
    return [NSMethodSignature signatureWithObjCTypes: "v@:"];
}

+ (void) forwardInvocation: (NSInvocation *) anInvocation
{
}

- (NSMethodSignature *) methodSignatureForSelector: (SEL) aSelector
{
    return [NSMethodSignature signatureWithObjCTypes: "v@:"];
}

- (void) forwardInvocation: (NSInvocation *) anInvocation
{
}

@end
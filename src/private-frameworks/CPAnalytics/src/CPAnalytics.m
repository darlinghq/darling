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
#include <stdlib.h>
#include <objc/runtime.h>

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

// Callers may send any message to this class. Every selector is answered with a no-op that
// returns nil/0, rather than letting an unimplemented method raise an exception. The selectors
// are unknown, so they are given an object result and one object argument per colon in the name;
// floating-point and large-struct results are not supported.
static NSMethodSignature *CPAnalyticsGuessSignature(SEL aSelector)
{
    char types[64] = "@@:";
    size_t length = 3;
    for (const char *name = sel_getName(aSelector); *name != '\0' && length < sizeof(types) - 1; name++) {
        if (*name == ':')
            types[length++] = '@';
    }
    types[length] = '\0';
    return [NSMethodSignature signatureWithObjCTypes: types];
}

static void CPAnalyticsAnswerWithZero(NSInvocation *anInvocation)
{
    NSUInteger length = [[anInvocation methodSignature] methodReturnLength];
    void *zero = length ? calloc(1, length) : NULL;
    if (zero != NULL) {
        [anInvocation setReturnValue: zero];
        free(zero);
    }
}

+ (NSMethodSignature *) methodSignatureForSelector: (SEL) aSelector
{
    return [super methodSignatureForSelector: aSelector] ?: CPAnalyticsGuessSignature(aSelector);
}

+ (void) forwardInvocation: (NSInvocation *) anInvocation
{
    CPAnalyticsAnswerWithZero(anInvocation);
}

- (NSMethodSignature *) methodSignatureForSelector: (SEL) aSelector
{
    return [super methodSignatureForSelector: aSelector] ?: CPAnalyticsGuessSignature(aSelector);
}

- (void) forwardInvocation: (NSInvocation *) anInvocation
{
    CPAnalyticsAnswerWithZero(anInvocation);
}

@end
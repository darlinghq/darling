/*
 This file is part of Darling.

 Copyright (C) 2026 Darling Developers

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

#import <SystemAdministration/SystemAdministration.h>
#include <stdlib.h>
#include <objc/runtime.h>

/*
 * The stubs answer every message they do not implement with a zeroed return
 * value, so that an object, pointer, BOOL or integer result reads as nil/NO/0.
 * The selectors are unknown, so they are all given an object result; a caller
 * that expects a floating-point or large-struct result from such a message is
 * not supported.
 */
/*
 * An unknown selector is given an object result and one object argument per
 * colon in its name, so the forwarding machinery accepts the call as the
 * caller made it.
 */
static NSMethodSignature *SAGuessSignature(SEL sel)
{
	char types[64] = "@@:";
	size_t length = 3;
	for (const char *name = sel_getName(sel); *name != '\0' && length < sizeof(types) - 1; name++) {
		if (*name == ':') {
			types[length++] = '@';
		}
	}
	types[length] = '\0';
	return [NSMethodSignature signatureWithObjCTypes:types];
}

static void SAAnswerWithZero(NSInvocation *invocation)
{
	NSUInteger length = [[invocation methodSignature] methodReturnLength];
	if (length == 0) {
		return;
	}
	void *zero = calloc(1, length);
	if (zero == NULL) {
		return;
	}
	[invocation setReturnValue:zero];
	free(zero);
}

#define SA_STUB_CLASS(name) \
@implementation name \
+ (NSMethodSignature *)methodSignatureForSelector:(SEL)sel \
{ \
	return [super methodSignatureForSelector:sel] ?: SAGuessSignature(sel); \
} \
+ (void)forwardInvocation:(NSInvocation *)invocation \
{ \
	SAAnswerWithZero(invocation); \
} \
- (NSMethodSignature *)methodSignatureForSelector:(SEL)sel \
{ \
	return [super methodSignatureForSelector:sel] ?: SAGuessSignature(sel); \
} \
- (void)forwardInvocation:(NSInvocation *)invocation \
{ \
	SAAnswerWithZero(invocation); \
} \
@end

SA_STUB_CLASS(ADMUser)
SA_STUB_CLASS(AdminAuthenticator)
SA_STUB_CLASS(UserUtilities)

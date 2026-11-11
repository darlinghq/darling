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

#ifndef DARLING_STUB_FORWARDING_H
#define DARLING_STUB_FORWARDING_H

#import <Foundation/Foundation.h>
#import <objc/runtime.h>

/*
 Support for placeholder classes that answer every message they do not implement.

 The Objective-C runtime builds an NSInvocation from the method signature that
 -methodSignatureForSelector: returns, and rejects the message if the signature
 does not describe the arguments the caller passed. The number of arguments of
 a method is the number of colons in its selector, so a signature with that many
 arguments is all the runtime needs to hand any message to -forwardInvocation:.

 Nothing is known about the real types, so the result and every argument are
 described as object pointers. That is right for objects, and for results and
 arguments of integer or pointer size on the supported architectures, and it
 is wrong for floating-point and structure values: a caller expecting such a
 result from a placeholder gets zero in the integer result register, and the
 forwarding code never reads the arguments. Anything that needs to use an
 argument or return a real value must implement that selector itself.
*/

// Selectors with more colons than this get a signature with this many arguments.
#define DARLING_STUB_MAX_ARGUMENTS 60

static inline NSMethodSignature *DarlingStubMethodSignature(SEL selector)
{
	// "@@:" is an object result, self and _cmd; one object argument per colon follows.
	char types[3 + DARLING_STUB_MAX_ARGUMENTS + 1] = "@@:";
	size_t length = 3;
	const char *name = sel_getName(selector);

	for (; name != NULL && *name != '\0' && length < 3 + DARLING_STUB_MAX_ARGUMENTS; name++) {
		if (*name == ':') {
			types[length++] = '@';
		}
	}
	types[length] = '\0';

	return [NSMethodSignature signatureWithObjCTypes: types];
}

// Answers a forwarded message with a zero result: nil, NULL, NO or 0.
static inline void DarlingStubForwardInvocation(id receiver, NSInvocation *invocation)
{
	NSLog(@"Stub called: %@ in %@", NSStringFromSelector([invocation selector]), [receiver class]);

	void *result = NULL;
	[invocation setReturnValue: &result];
}

#endif

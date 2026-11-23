// CFLAGS: -framework Foundation -framework Security -framework SecurityFoundation
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

// SFAuthorization: an Objective-C interface to Authorization Services. The documented contract is
// that the object owns an AuthorizationRef that works in calls to the Authorization Services
// functions, and that its obtain methods return YES when the rights are granted and otherwise NO
// with the result code in the error, so the expectations compare the methods with the C calls on
// the same reference. Nothing here asks for a right that needs the user to authenticate.
// https://developer.apple.com/documentation/securityfoundation/sfauthorization
// Prints PASS when everything holds; the exit code is the number of failed checks.

#import <Foundation/Foundation.h>
#import <Security/Authorization.h>
#import <SecurityFoundation/SFAuthorization.h>
#include <stdio.h>

static int failures;

#define CHECK(cond) do { if (!(cond)) { failures++; printf("FAIL line %d: %s\n", __LINE__, #cond); } } while (0)

int main(void)
{
	@autoreleasepool {
		// the convenience constructors and initialisers give an object holding a reference
		SFAuthorization *a = [SFAuthorization authorization];
		SFAuthorization *b = [[SFAuthorization alloc] init];
		SFAuthorization *c = [SFAuthorization authorizationWithFlags:kAuthorizationFlagDefaults rights:NULL environment:kAuthorizationEmptyEnvironment];
		SFAuthorization *d = [[SFAuthorization alloc] initWithFlags:kAuthorizationFlagDefaults rights:NULL environment:kAuthorizationEmptyEnvironment];
		CHECK(a != nil && b != nil && c != nil && d != nil);
		CHECK([a authorizationRef] != NULL);
		CHECK([a authorizationRef] == [a authorizationRef]);
		CHECK([a authorizationRef] != [b authorizationRef]);
		CHECK([c authorizationRef] != NULL && [d authorizationRef] != NULL);

		// the reference works in the C API, and asking for no rights is granted (a variable, because Darling's
		// SDK header marks the parameter nonnull while the documentation says to pass nil for no rights)
		const AuthorizationRights *noRights = NULL;
		CHECK(AuthorizationCopyRights([a authorizationRef], noRights, kAuthorizationEmptyEnvironment, kAuthorizationFlagDefaults, NULL) == errAuthorizationSuccess);
		NSError *error = nil;
		CHECK([a obtainWithRights:NULL flags:kAuthorizationFlagDefaults environment:kAuthorizationEmptyEnvironment authorizedRights:NULL error:&error]);
		// observed on macOS 26.7.3 and 10.14.4: the error is set on success too, with code 0
		CHECK(error != nil && [[error domain] isEqualToString:NSOSStatusErrorDomain] && [error code] == errAuthorizationSuccess);
		AuthorizationRights *granted = NULL;
		CHECK([a obtainWithRights:NULL flags:kAuthorizationFlagDefaults environment:kAuthorizationEmptyEnvironment authorizedRights:&granted error:NULL]);
		if (granted != NULL)
			AuthorizationFreeItemSet(granted);

		// a named right: the method agrees with the C call on the same reference, and a failure carries its code
		const char *right = "org.darlinghq.test.sfauthorization.no-such-right";
		AuthorizationItem item = { right, 0, NULL, 0 };
		AuthorizationRights rights = { 1, &item };
		OSStatus status = AuthorizationCopyRights([b authorizationRef], &rights, kAuthorizationEmptyEnvironment, kAuthorizationFlagDefaults, NULL);
		error = nil;
		BOOL obtained = [b obtainWithRight:right flags:kAuthorizationFlagDefaults error:&error];
		CHECK(obtained == (status == errAuthorizationSuccess));
		if (!obtained) {
			CHECK(error != nil);
			CHECK([[error domain] isEqualToString:NSOSStatusErrorDomain]);
			CHECK([error code] == status);
		}
		error = nil;
		obtained = [b obtainWithRights:&rights flags:kAuthorizationFlagDefaults environment:kAuthorizationEmptyEnvironment authorizedRights:NULL error:&error];
		CHECK(obtained == (status == errAuthorizationSuccess));
		CHECK(obtained || (error != nil && [error code] == status));
		// a NULL error pointer is allowed
		CHECK([b obtainWithRight:right flags:kAuthorizationFlagDefaults error:NULL] == obtained);

		// invalidating one object does not touch another one
		[a invalidateCredentials];
		// observed on both Macs: the invalidated object has no reference and its obtain methods fail with errAuthorizationInvalidRef
		CHECK([a authorizationRef] == NULL);
		error = nil;
		CHECK(![a obtainWithRights:NULL flags:kAuthorizationFlagDefaults environment:kAuthorizationEmptyEnvironment authorizedRights:NULL error:&error]);
		CHECK(error != nil && [error code] == errAuthorizationInvalidRef);
		CHECK(AuthorizationCopyRights([b authorizationRef], noRights, kAuthorizationEmptyEnvironment, kAuthorizationFlagDefaults, NULL) == errAuthorizationSuccess);
		CHECK([[SFAuthorization authorization] authorizationRef] != NULL);

		[b release];
		[d release];
	}
	if (failures == 0)
		printf("PASS\n");
	return failures;
}

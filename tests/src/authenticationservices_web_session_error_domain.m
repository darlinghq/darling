// CFLAGS: -framework AuthenticationServices -framework Foundation
// Copyright (C) 2026 Lubos Dolezel
//
// ASWebAuthenticationSessionErrorDomain and its error codes. Observed on macOS 26.7.3; passes
// unchanged there.
#import <AuthenticationServices/AuthenticationServices.h>
#include <stdio.h>

int main(void)
{
	int ok = [ASWebAuthenticationSessionErrorDomain isEqualToString:@"com.apple.AuthenticationServices.WebAuthenticationSession"]
		&& ASWebAuthenticationSessionErrorCodeCanceledLogin == 1
		&& ASWebAuthenticationSessionErrorCodePresentationContextNotProvided == 2
		&& ASWebAuthenticationSessionErrorCodePresentationContextInvalid == 3;
	printf(ok ? "PASS\n" : "FAIL: ASWebAuthenticationSessionErrorDomain\n");
	return !ok;
}

// CFLAGS: -framework AuthenticationServices -framework Foundation
#import <AuthenticationServices/AuthenticationServices.h>
#import <Foundation/Foundation.h>
#include <stdio.h>

#define CHECK(c) do { if (!(c)) { fprintf(stderr, "FAIL: %s (line %d)\n", #c, __LINE__); return 1; } } while (0)

int main(void) {
    @autoreleasepool {
        CHECK([ASAuthorizationScopeEmail isEqualToString:@"email"]);
        CHECK([ASWebAuthenticationSessionCallback class] != Nil);
    }
    puts("PASS");
    return 0;
}

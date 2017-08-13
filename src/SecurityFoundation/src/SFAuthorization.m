#include <SecurityFoundation/SecurityFoundation.h>

@implementation SFAuthorization

+ (id)authorization {
    return [[self alloc] init];
}

- (AuthorizationRef)authorizationRef {
    return NULL;
}

+ (id)authorizationWithFlags:(AuthorizationFlags)flags rights:(const AuthorizationRights *)rights environment:(const AuthorizationEnvironment *)environment {
    return nil;
}

- (id)initWithFlags:(AuthorizationFlags)flags rights:(const AuthorizationRights *)rights environment:(const AuthorizationEnvironment *)environment {
    return nil;
}

-(id)init {
    if (self = [super init]) {
        return self;
    } else {
        return nil;
    }
}

- (void)invalidateCredentials {

}

- (BOOL)obtainWithRight:(AuthorizationString)rightName flags:(AuthorizationFlags)flags error:(NSError**)error {
    return NO;
}

- (BOOL)obtainWithRights:(const AuthorizationRights *)rights flags:(AuthorizationFlags)flags environment:(const AuthorizationEnvironment *)environment authorizedRights:(AuthorizationRights **)authorizedRights error:(NSError**)error {
    return NO;
}

@end

@implementation SFAuthorization (SFAuthorizationDeprecated)

- (OSStatus)permitWithRights:(const AuthorizationRights *)rights flags:(AuthorizationFlags)flags environment:(const AuthorizationEnvironment *)environment authorizedRights:(AuthorizationRights *)authorizedRights {
    return 0;
}
- (OSStatus)permitWithRight:(AuthorizationString)rightName flags:(AuthorizationFlags)flags {
    return 0;
}

@end

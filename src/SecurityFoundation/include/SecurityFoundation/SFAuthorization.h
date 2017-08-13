#ifndef _SFAUTHORIZATION_H_
#define _SFAUTHORIZATION_H_

#import <Foundation/Foundation.h>
#import <Security/Authorization.h>

@interface SFAuthorization : NSObject <NSCoding>
{
@private
	AuthorizationRef _authorization;
    id _reserved;
}

+ (id)authorization;

- (AuthorizationRef)authorizationRef;

+ (id)authorizationWithFlags:(AuthorizationFlags)flags rights:(const AuthorizationRights *)rights environment:(const AuthorizationEnvironment *)environment;

- (id)initWithFlags:(AuthorizationFlags)flags rights:(const AuthorizationRights *)rights environment:(const AuthorizationEnvironment *)environment;

-(id)init;

- (void)invalidateCredentials;

- (BOOL)obtainWithRight:(AuthorizationString)rightName flags:(AuthorizationFlags)flags error:(NSError**)error;

- (BOOL)obtainWithRights:(const AuthorizationRights *)rights flags:(AuthorizationFlags)flags environment:(const AuthorizationEnvironment *)environment authorizedRights:(AuthorizationRights **)authorizedRights error:(NSError**)error;

@end

@interface SFAuthorization (SFAuthorizationDeprecated)

- (OSStatus)permitWithRights:(const AuthorizationRights *)rights flags:(AuthorizationFlags)flags environment:(const AuthorizationEnvironment *)environment authorizedRights:(AuthorizationRights *)authorizedRights DEPRECATED_IN_MAC_OS_X_VERSION_10_5_AND_LATER;

- (OSStatus)permitWithRight:(AuthorizationString)rightName flags:(AuthorizationFlags)flags DEPRECATED_IN_MAC_OS_X_VERSION_10_5_AND_LATER;

@end

#endif

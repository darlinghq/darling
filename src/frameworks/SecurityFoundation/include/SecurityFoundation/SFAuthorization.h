/*
 This file is part of Darling.

 Copyright (C) 2019 Lubos Dolezel

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

#ifndef _SFAUTHORIZATION_H_
#define _SFAUTHORIZATION_H_

#include <Foundation/Foundation.h>
#include <Security/Authorization.h>

// An Objective-C interface to some of the functions of Authorization Services. It owns an
// AuthorizationRef, created when the object is initialised and freed when it is deallocated.
@interface SFAuthorization : NSObject {
@private
	AuthorizationRef _authorization;
}

+ (id)authorization;
+ (id)authorizationWithFlags:(AuthorizationFlags)flags rights:(const AuthorizationRights *)rights environment:(const AuthorizationEnvironment *)environment;

- (id)init;
- (id)initWithFlags:(AuthorizationFlags)flags rights:(const AuthorizationRights *)rights environment:(const AuthorizationEnvironment *)environment;

- (AuthorizationRef)authorizationRef;

- (BOOL)obtainWithRights:(const AuthorizationRights *)rights flags:(AuthorizationFlags)flags environment:(const AuthorizationEnvironment *)environment authorizedRights:(AuthorizationRights **)authorizedRights error:(NSError **)error;
- (BOOL)obtainWithRight:(AuthorizationString)rightName flags:(AuthorizationFlags)flags error:(NSError **)error;

- (void)invalidateCredentials;

@end

#endif

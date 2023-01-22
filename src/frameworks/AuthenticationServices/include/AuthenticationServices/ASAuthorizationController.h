/*
 This file is part of Darling.

 Copyright (C) 2022 Darling Team

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

#include <Foundation/Foundation.h>
#include <AppKit/NSWindow.h>

#include <AuthenticationServices/ASAuthorizationControllerDelegate.h>
#include <AuthenticationServices/ASAuthorizationControllerPresentationContextProviding.h>
#include <AuthenticationServices/ASAuthorizationRequest.h>

#ifdef TARGET_OS_OSX
typedef NSWindow *ASPresentationAnchor;
#else // TARGET_OS_IPHONE
typedef UIWindow *ASPresentationAnchor;
#endif


@interface ASAuthorizationController : NSObject {
    id<ASAuthorizationControllerDelegate> _delegate;
    id<ASAuthorizationControllerPresentationContextProviding> _presentationContextProvider;
}

#ifndef DARLING
@property(nonatomic, weak, nullable) id<ASAuthorizationControllerDelegate> delegate;
@property(nonatomic, weak, nullable) id<ASAuthorizationControllerPresentationContextProviding> presentationContextProvider;
#else
@property(nonatomic, assign, nullable) id<ASAuthorizationControllerDelegate> delegate;
@property(nonatomic, assign, nullable) id<ASAuthorizationControllerPresentationContextProviding> presentationContextProvider;
#endif

- (instancetype)initWithAuthorizationRequests:(NSArray<ASAuthorizationRequest *> *)authorizationRequests;
- (void)performRequests;

@end

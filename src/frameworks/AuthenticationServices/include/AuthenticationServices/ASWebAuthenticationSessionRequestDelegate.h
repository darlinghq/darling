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

NS_ASSUME_NONNULL_BEGIN

@class ASWebAuthenticationSessionRequest;

// https://developer.apple.com/documentation/authenticationservices/aswebauthenticationsessionrequestdelegate
@protocol ASWebAuthenticationSessionRequestDelegate <NSObject>

@optional
- (void)authenticationSessionRequest:(ASWebAuthenticationSessionRequest *)authenticationSessionRequest didCompleteWithCallbackURL:(NSURL *)callbackURL;
- (void)authenticationSessionRequest:(ASWebAuthenticationSessionRequest *)authenticationSessionRequest didCancelWithError:(NSError *)error;

@end

NS_ASSUME_NONNULL_END

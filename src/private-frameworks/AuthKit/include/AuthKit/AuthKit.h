/*
This file is part of Darling.

Copyright (C) 2017 Lubos Dolezel

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

void* AKLoginCodeNotificationCreate(void);
void* AKStringFromServiceType(void);
void* _AKLogSystem(void);
void* _AKTrafficLogSubsystem(void);

#import <Foundation/Foundation.h>
#import <AuthKit/AKAppleIDAuthenticationController.h>
#import <AuthKit/_AKAppleIDAuthenticationContextManager.h>
#import <AuthKit/AKURLDataTask.h>
#import <AuthKit/AKURLSession.h>
#import <AuthKit/AKCertificatePinning.h>
#import <AuthKit/AKAppleIDAuthenticationClientInterface.h>
#import <AuthKit/AKFollowUpFactoryImpl.h>
#import <AuthKit/AKConfiguration.h>
#import <AuthKit/AKAppleIDServerResourceLoadDelegate.h>
#import <AuthKit/AKURLBag.h>
#import <AuthKit/AKCircleRequestContext.h>
#import <AuthKit/AKAbsintheSigner.h>
#import <AuthKit/AKAppleIDSession.h>
#import <AuthKit/AKFollowUpTearDownContext.h>
#import <AuthKit/AKAnisetteData.h>
#import <AuthKit/AKToken.h>
#import <AuthKit/AKMasterToken.h>
#import <AuthKit/AKCDPFactory.h>
#import <AuthKit/AKAnisetteProvisioningDaemonInterface.h>
#import <AuthKit/AKDevice.h>
#import <AuthKit/AKCircleRequestPayload.h>
#import <AuthKit/AKFollowUpProviderImpl.h>
#import <AuthKit/AKAnisetteProvisioningController.h>
#import <AuthKit/AKAppleIDAuthenticationContext.h>
#import <AuthKit/AKAccountManager.h>
#import <AuthKit/AKAppleIDAuthenticationDaemonInterface.h>
#import <AuthKit/AKDeviceListRequestContext.h>
#import <AuthKit/AKRemoteDevice.h>

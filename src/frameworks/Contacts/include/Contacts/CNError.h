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

#import <Foundation/Foundation.h>

NS_ASSUME_NONNULL_BEGIN

// Error domain of the Contacts framework.
extern NSErrorDomain const CNErrorDomain API_AVAILABLE(macos(10.11));

typedef NS_ERROR_ENUM(CNErrorDomain, CNErrorCode) {
    CNErrorCodeCommunicationError = 1,
    CNErrorCodeNoAccessableWritableContainers = 101,
    CNErrorCodeAuthorizationDenied = 100,
    CNErrorCodeRecordDoesNotExist = 200,
    CNErrorCodeInsertedRecordAlreadyExists = 201,
    CNErrorCodeContainmentCycle = 202,
    CNErrorCodeContainmentScope = 203,
    CNErrorCodeParentRecordDoesNotExist = 204,
    CNErrorCodeRecordIdentifierInvalid = 205,
    CNErrorCodeValidationMultipleErrors = 300,
    CNErrorCodeValidationTypeMismatch = 301,
    CNErrorCodeValidationConfigurationError = 302,
    CNErrorCodePredicateInvalid = 400,
    CNErrorCodePolicyViolation = 500,
    CNErrorCodeClientIdentifierInvalid = 600,
    CNErrorCodeClientIdentifierCollision = 602,
    CNErrorCodeChangeHistoryExpired = 603,
    CNErrorCodeChangeHistoryInvalidAnchor = 604,
    CNErrorCodeChangeHistoryInvalidFetchRequest = 605,
    CNErrorCodeVCardMalformed = 700,
    CNErrorCodeVCardSummarizationError = 701,
} API_AVAILABLE(macos(10.11));

// Keys of the userInfo dictionary of a CNErrorDomain error.
extern NSString * const CNErrorUserInfoAffectedRecordsKey API_AVAILABLE(macos(10.11));
extern NSString * const CNErrorUserInfoAffectedRecordIdentifiersKey API_AVAILABLE(macos(10.11));
extern NSString * const CNErrorUserInfoKeyPathsKey API_AVAILABLE(macos(10.11));

NS_ASSUME_NONNULL_END

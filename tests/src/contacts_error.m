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



// Contacts errors: the CNErrorDomain string, the CNErrorCode values and the userInfo keys.
// Prints PASS when everything holds; the exit code is the number of failed checks.

#import <Foundation/Foundation.h>
#import <Contacts/Contacts.h>

static int failures;

#define CHECK(cond) do { if (!(cond)) { printf("FAIL line %d: %s\n", __LINE__, #cond); failures++; } } while (0)

int main(void)
{
    @autoreleasepool {
        CHECK([CNErrorDomain isEqualToString:@"CNErrorDomain"]);
        CHECK([CNErrorUserInfoAffectedRecordsKey isEqualToString:@"CNInvalidRecords"]);
        CHECK([CNErrorUserInfoAffectedRecordIdentifiersKey isEqualToString:@"CNInvalidRecordIdentifiers"]);
        CHECK([CNErrorUserInfoKeyPathsKey isEqualToString:@"CNKeyPaths"]);

        CHECK(sizeof(CNErrorCode) == sizeof(NSInteger));
        CHECK(CNErrorCodeCommunicationError == 1);
        CHECK(CNErrorCodeNoAccessableWritableContainers == 101);
        CHECK(CNErrorCodeAuthorizationDenied == 100);
        CHECK(CNErrorCodeRecordDoesNotExist == 200);
        CHECK(CNErrorCodeInsertedRecordAlreadyExists == 201);
        CHECK(CNErrorCodeContainmentCycle == 202);
        CHECK(CNErrorCodeContainmentScope == 203);
        CHECK(CNErrorCodeParentRecordDoesNotExist == 204);
        CHECK(CNErrorCodeRecordIdentifierInvalid == 205);
        CHECK(CNErrorCodeValidationMultipleErrors == 300);
        CHECK(CNErrorCodeValidationTypeMismatch == 301);
        CHECK(CNErrorCodeValidationConfigurationError == 302);
        CHECK(CNErrorCodePredicateInvalid == 400);
        CHECK(CNErrorCodePolicyViolation == 500);
        CHECK(CNErrorCodeClientIdentifierInvalid == 600);
        CHECK(CNErrorCodeClientIdentifierCollision == 602);
        CHECK(CNErrorCodeChangeHistoryExpired == 603);
        CHECK(CNErrorCodeChangeHistoryInvalidAnchor == 604);
        CHECK(CNErrorCodeChangeHistoryInvalidFetchRequest == 605);
        CHECK(CNErrorCodeVCardMalformed == 700);
        CHECK(CNErrorCodeVCardSummarizationError == 701);

        NSError *e = [NSError errorWithDomain:CNErrorDomain
                                         code:CNErrorCodeRecordDoesNotExist
                                     userInfo:@{CNErrorUserInfoKeyPathsKey: @[@"givenName"]}];
        CHECK(e.code == 200);
        CHECK([e.userInfo[CNErrorUserInfoKeyPathsKey] isEqual:@[@"givenName"]]);
    }
    if (!failures) printf("PASS\n");
    return failures;
}

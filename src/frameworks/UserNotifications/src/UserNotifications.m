/*
 This file is part of Darling.

 Copyright (C) 2021 Lubos Dolezel

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


#include <UserNotifications/UserNotifications.h>
#include <stdlib.h>
#include <stdio.h>

static int verbose = 0;

__attribute__((constructor))
static void initme(void) {
    verbose = getenv("STUB_VERBOSE") != NULL;
}

void* UNDoubleIsZero(void)
{
    if (verbose) puts("STUB: UNDoubleIsZero called");
    return NULL;
}

void* UNEqualBools(void)
{
    if (verbose) puts("STUB: UNEqualBools called");
    return NULL;
}

void* UNEqualDoubles(void)
{
    if (verbose) puts("STUB: UNEqualDoubles called");
    return NULL;
}

void* UNEqualFloats(void)
{
    if (verbose) puts("STUB: UNEqualFloats called");
    return NULL;
}

void* UNEqualIntegers(void)
{
    if (verbose) puts("STUB: UNEqualIntegers called");
    return NULL;
}

void* UNEqualObjects(void)
{
    if (verbose) puts("STUB: UNEqualObjects called");
    return NULL;
}

void* UNEqualStrings(void)
{
    if (verbose) puts("STUB: UNEqualStrings called");
    return NULL;
}

void* UNFloatIsZero(void)
{
    if (verbose) puts("STUB: UNFloatIsZero called");
    return NULL;
}

void* UNFormatLocalizedString(void)
{
    if (verbose) puts("STUB: UNFormatLocalizedString called");
    return NULL;
}

void* UNFormatLocalizedStringInBundleWithDefaultValue(void)
{
    if (verbose) puts("STUB: UNFormatLocalizedStringInBundleWithDefaultValue called");
    return NULL;
}

void* UNIsInternalInstall(void)
{
    if (verbose) puts("STUB: UNIsInternalInstall called");
    return NULL;
}

void* UNNotificationAttachmentFamilyFromTypeIdentifier(void)
{
    if (verbose) puts("STUB: UNNotificationAttachmentFamilyFromTypeIdentifier called");
    return NULL;
}

void* UNNotificationAttachmentFamilyMaximumSize(void)
{
    if (verbose) puts("STUB: UNNotificationAttachmentFamilyMaximumSize called");
    return NULL;
}

void* UNNotificationAttachmentFamilyToString(void)
{
    if (verbose) puts("STUB: UNNotificationAttachmentFamilyToString called");
    return NULL;
}

void* UNNotificationGroupingSettingString(void)
{
    if (verbose) puts("STUB: UNNotificationGroupingSettingString called");
    return NULL;
}

void* UNNotificationSettingString(void)
{
    if (verbose) puts("STUB: UNNotificationSettingString called");
    return NULL;
}

void* UNRegisterUserNotificationsLogging(void)
{
    if (verbose) puts("STUB: UNRegisterUserNotificationsLogging called");
    return NULL;
}

void* UNSafeCast(void)
{
    if (verbose) puts("STUB: UNSafeCast called");
    return NULL;
}

void* UNSafeCastAny(void)
{
    if (verbose) puts("STUB: UNSafeCastAny called");
    return NULL;
}

void* UNSafeConforms(void)
{
    if (verbose) puts("STUB: UNSafeConforms called");
    return NULL;
}

void* UNShowPreviewsSettingString(void)
{
    if (verbose) puts("STUB: UNShowPreviewsSettingString called");
    return NULL;
}

void* UNSimilarSets(void)
{
    if (verbose) puts("STUB: UNSimilarSets called");
    return NULL;
}

void* UNSimilarStrings(void)
{
    if (verbose) puts("STUB: UNSimilarStrings called");
    return NULL;
}

void* un_CGRectContainsRect(void)
{
    if (verbose) puts("STUB: un_CGRectContainsRect called");
    return NULL;
}

void* un_CGRectCreateDictionaryRepresentation(void)
{
    if (verbose) puts("STUB: un_CGRectCreateDictionaryRepresentation called");
    return NULL;
}

void* un_CGRectEqualToRect(void)
{
    if (verbose) puts("STUB: un_CGRectEqualToRect called");
    return NULL;
}

void* un_CGRectIsEmpty(void)
{
    if (verbose) puts("STUB: un_CGRectIsEmpty called");
    return NULL;
}

void* un_CGRectMakeWithDictionaryRepresentation(void)
{
    if (verbose) puts("STUB: un_CGRectMakeWithDictionaryRepresentation called");
    return NULL;
}

void* un_CGRectNull(void)
{
    if (verbose) puts("STUB: un_CGRectNull called");
    return NULL;
}

void* un_CMTimeCompare(void)
{
    if (verbose) puts("STUB: un_CMTimeCompare called");
    return NULL;
}

void* un_CMTimeCopyAsDictionary(void)
{
    if (verbose) puts("STUB: un_CMTimeCopyAsDictionary called");
    return NULL;
}

void* un_CMTimeMakeFromDictionary(void)
{
    if (verbose) puts("STUB: un_CMTimeMakeFromDictionary called");
    return NULL;
}

void* un_CMTimeMakeWithSeconds(void)
{
    if (verbose) puts("STUB: un_CMTimeMakeWithSeconds called");
    return NULL;
}

void* un_kCMTimeInvalid(void)
{
    if (verbose) puts("STUB: un_kCMTimeInvalid called");
    return NULL;
}

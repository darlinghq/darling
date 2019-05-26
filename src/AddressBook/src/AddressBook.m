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


#include <AddressBook/AddressBook.h>
#include <stdlib.h>
#include <stdio.h>

static int verbose = 0;

__attribute__((constructor))
static void initme(void) {
    verbose = getenv("STUB_VERBOSE") != NULL;
}

void* ABAddressBookLocalizedString(void)
{
    if (verbose) puts("STUB: ABAddressBookLocalizedString called");
    return NULL;
}

void* ABDeleteRecordsCommandNameForRecords(void)
{
    if (verbose) puts("STUB: ABDeleteRecordsCommandNameForRecords called");
    return NULL;
}

void* ABEmailCertificateCopyMatchingUnexpired(void)
{
    if (verbose) puts("STUB: ABEmailCertificateCopyMatchingUnexpired called");
    return NULL;
}

void* ABEmailCertificateHasMatchingUnexpired(void)
{
    if (verbose) puts("STUB: ABEmailCertificateHasMatchingUnexpired called");
    return NULL;
}

void* ABIsParagraphSeparator(void)
{
    if (verbose) puts("STUB: ABIsParagraphSeparator called");
    return NULL;
}

void* ABPickerAddProperty(void)
{
    if (verbose) puts("STUB: ABPickerAddProperty called");
    return NULL;
}

void* ABPickerChangeAttributes(void)
{
    if (verbose) puts("STUB: ABPickerChangeAttributes called");
    return NULL;
}

void* ABPickerClearSearchField(void)
{
    if (verbose) puts("STUB: ABPickerClearSearchField called");
    return NULL;
}

void* ABPickerCopyColumnTitle(void)
{
    if (verbose) puts("STUB: ABPickerCopyColumnTitle called");
    return NULL;
}

void* ABPickerCopyDisplayedProperty(void)
{
    if (verbose) puts("STUB: ABPickerCopyDisplayedProperty called");
    return NULL;
}

void* ABPickerCopyProperties(void)
{
    if (verbose) puts("STUB: ABPickerCopyProperties called");
    return NULL;
}

void* ABPickerCopySelectedGroups(void)
{
    if (verbose) puts("STUB: ABPickerCopySelectedGroups called");
    return NULL;
}

void* ABPickerCopySelectedIdentifiers(void)
{
    if (verbose) puts("STUB: ABPickerCopySelectedIdentifiers called");
    return NULL;
}

void* ABPickerCopySelectedRecords(void)
{
    if (verbose) puts("STUB: ABPickerCopySelectedRecords called");
    return NULL;
}

void* ABPickerCopySelectedValues(void)
{
    if (verbose) puts("STUB: ABPickerCopySelectedValues called");
    return NULL;
}

void* ABPickerCreate(void)
{
    if (verbose) puts("STUB: ABPickerCreate called");
    return NULL;
}

void* ABPickerDeselectAll(void)
{
    if (verbose) puts("STUB: ABPickerDeselectAll called");
    return NULL;
}

void* ABPickerDeselectGroup(void)
{
    if (verbose) puts("STUB: ABPickerDeselectGroup called");
    return NULL;
}

void* ABPickerDeselectIdentifier(void)
{
    if (verbose) puts("STUB: ABPickerDeselectIdentifier called");
    return NULL;
}

void* ABPickerDeselectRecord(void)
{
    if (verbose) puts("STUB: ABPickerDeselectRecord called");
    return NULL;
}

void* ABPickerEditInAddressBook(void)
{
    if (verbose) puts("STUB: ABPickerEditInAddressBook called");
    return NULL;
}

void* ABPickerGetAttributes(void)
{
    if (verbose) puts("STUB: ABPickerGetAttributes called");
    return NULL;
}

void* ABPickerGetDelegate(void)
{
    if (verbose) puts("STUB: ABPickerGetDelegate called");
    return NULL;
}

void* ABPickerGetFrame(void)
{
    if (verbose) puts("STUB: ABPickerGetFrame called");
    return NULL;
}

void* ABPickerIsVisible(void)
{
    if (verbose) puts("STUB: ABPickerIsVisible called");
    return NULL;
}

void* ABPickerRemoveProperty(void)
{
    if (verbose) puts("STUB: ABPickerRemoveProperty called");
    return NULL;
}

void* ABPickerSelectGroup(void)
{
    if (verbose) puts("STUB: ABPickerSelectGroup called");
    return NULL;
}

void* ABPickerSelectIdentifier(void)
{
    if (verbose) puts("STUB: ABPickerSelectIdentifier called");
    return NULL;
}

void* ABPickerSelectInAddressBook(void)
{
    if (verbose) puts("STUB: ABPickerSelectInAddressBook called");
    return NULL;
}

void* ABPickerSelectRecord(void)
{
    if (verbose) puts("STUB: ABPickerSelectRecord called");
    return NULL;
}

void* ABPickerSetColumnTitle(void)
{
    if (verbose) puts("STUB: ABPickerSetColumnTitle called");
    return NULL;
}

void* ABPickerSetDelegate(void)
{
    if (verbose) puts("STUB: ABPickerSetDelegate called");
    return NULL;
}

void* ABPickerSetDisplayedProperty(void)
{
    if (verbose) puts("STUB: ABPickerSetDisplayedProperty called");
    return NULL;
}

void* ABPickerSetFrame(void)
{
    if (verbose) puts("STUB: ABPickerSetFrame called");
    return NULL;
}

void* ABPickerSetVisibility(void)
{
    if (verbose) puts("STUB: ABPickerSetVisibility called");
    return NULL;
}

void* _ABClipRangeToLength(void)
{
    if (verbose) puts("STUB: _ABClipRangeToLength called");
    return NULL;
}

void* _ABEmailCertificateCopyAllMatching(void)
{
    if (verbose) puts("STUB: _ABEmailCertificateCopyAllMatching called");
    return NULL;
}

void* _ABEmailCertificateHasExpired(void)
{
    if (verbose) puts("STUB: _ABEmailCertificateHasExpired called");
    return NULL;
}

void* addChild(void)
{
    if (verbose) puts("STUB: addChild called");
    return NULL;
}

void* createStructure(void)
{
    if (verbose) puts("STUB: createStructure called");
    return NULL;
}

void* endStructure(void)
{
    if (verbose) puts("STUB: endStructure called");
    return NULL;
}

void* handleError(void)
{
    if (verbose) puts("STUB: handleError called");
    return NULL;
}

void* resolveExternalEntity(void)
{
    if (verbose) puts("STUB: resolveExternalEntity called");
    return NULL;
}

/*
 This file is part of Darling.

 Copyright (C) 2020 Lubos Dolezel

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


#include <Accounts/Accounts.h>
#include <stdlib.h>
#include <stdio.h>

static int verbose = 0;

__attribute__((constructor))
static void initme(void) {
    verbose = getenv("STUB_VERBOSE") != NULL;
}

void* ACAccountTypeIdentifierForASSAccountType(void)
{
    if (verbose) puts("STUB: ACAccountTypeIdentifierForASSAccountType called");
    return NULL;
}

void* ACDGetAdamOrDisplayIDForPID(void)
{
    if (verbose) puts("STUB: ACDGetAdamOrDisplayIDForPID called");
    return NULL;
}

void* ACDGetStoreOrBundleIDForPID(void)
{
    if (verbose) puts("STUB: ACDGetStoreOrBundleIDForPID called");
    return NULL;
}

void* ACDGetTeamIDForBundleURL(void)
{
    if (verbose) puts("STUB: ACDGetTeamIDForBundleURL called");
    return NULL;
}

void* ACDataclassForASSDataclass(void)
{
    if (verbose) puts("STUB: ACDataclassForASSDataclass called");
    return NULL;
}

void* _ACLogSystem(void)
{
    if (verbose) puts("STUB: _ACLogSystem called");
    return NULL;
}

void* _ACPLogSystem(void)
{
    if (verbose) puts("STUB: _ACPLogSystem called");
    return NULL;
}

void* _ACSignpostCreate(void)
{
    if (verbose) puts("STUB: _ACSignpostCreate called");
    return NULL;
}

void* _ACSignpostGetNanoseconds(void)
{
    if (verbose) puts("STUB: _ACSignpostGetNanoseconds called");
    return NULL;
}

void* _ACSignpostLogSystem(void)
{
    if (verbose) puts("STUB: _ACSignpostLogSystem called");
    return NULL;
}

void* createACAccountWithASAccountProperties(void)
{
    if (verbose) puts("STUB: createACAccountWithASAccountProperties called");
    return NULL;
}

void* createACAccountWithASAccountPropertiesAndACAccountType(void)
{
    if (verbose) puts("STUB: createACAccountWithASAccountPropertiesAndACAccountType called");
    return NULL;
}

void* serializeSecCertificates(void)
{
    if (verbose) puts("STUB: serializeSecCertificates called");
    return NULL;
}

void* serializeSecTrust(void)
{
    if (verbose) puts("STUB: serializeSecTrust called");
    return NULL;
}

void* unserializeSecCertificates(void)
{
    if (verbose) puts("STUB: unserializeSecCertificates called");
    return NULL;
}

void* unserializeSecTrust(void)
{
    if (verbose) puts("STUB: unserializeSecTrust called");
    return NULL;
}

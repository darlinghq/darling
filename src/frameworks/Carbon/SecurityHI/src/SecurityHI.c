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


#include <SecurityHI/SecurityHI.h>
#include <stdlib.h>
#include <stdio.h>

static int verbose = 0;

__attribute__((constructor))
static void initme(void) {
    verbose = getenv("STUB_VERBOSE") != NULL;
}

void* DisposeURLNotifyUPP(void)
{
    if (verbose) puts("STUB: DisposeURLNotifyUPP called");
    return NULL;
}

void* DisposeURLSystemEventUPP(void)
{
    if (verbose) puts("STUB: DisposeURLSystemEventUPP called");
    return NULL;
}

void* InvokeURLNotifyUPP(void)
{
    if (verbose) puts("STUB: InvokeURLNotifyUPP called");
    return NULL;
}

void* InvokeURLSystemEventUPP(void)
{
    if (verbose) puts("STUB: InvokeURLSystemEventUPP called");
    return NULL;
}

void* KCAddAppleSharePassword(void)
{
    if (verbose) puts("STUB: KCAddAppleSharePassword called");
    return NULL;
}

void* KCAddGenericPassword(void)
{
    if (verbose) puts("STUB: KCAddGenericPassword called");
    return NULL;
}

void* KCAddInternetPassword(void)
{
    if (verbose) puts("STUB: KCAddInternetPassword called");
    return NULL;
}

void* KCAddInternetPasswordWithPath(void)
{
    if (verbose) puts("STUB: KCAddInternetPasswordWithPath called");
    return NULL;
}

void* KCAddItem(void)
{
    if (verbose) puts("STUB: KCAddItem called");
    return NULL;
}

void* KCChangeSettings(void)
{
    if (verbose) puts("STUB: KCChangeSettings called");
    return NULL;
}

void* KCCreateKeychain(void)
{
    if (verbose) puts("STUB: KCCreateKeychain called");
    return NULL;
}

void* KCUnlock(void)
{
    if (verbose) puts("STUB: KCUnlock called");
    return NULL;
}

void* NewURLNotifyUPP(void)
{
    if (verbose) puts("STUB: NewURLNotifyUPP called");
    return NULL;
}

void* NewURLSystemEventUPP(void)
{
    if (verbose) puts("STUB: NewURLSystemEventUPP called");
    return NULL;
}

void* SecChooseIdentity(void)
{
    if (verbose) puts("STUB: SecChooseIdentity called");
    return NULL;
}

void* SecChooseIdentityAsSheet(void)
{
    if (verbose) puts("STUB: SecChooseIdentityAsSheet called");
    return NULL;
}

void* SecDisplayCertificate(void)
{
    if (verbose) puts("STUB: SecDisplayCertificate called");
    return NULL;
}

void* SecDisplayCertificateGroup(void)
{
    if (verbose) puts("STUB: SecDisplayCertificateGroup called");
    return NULL;
}

void* SecEditTrust(void)
{
    if (verbose) puts("STUB: SecEditTrust called");
    return NULL;
}

void* SecEditTrustAsSheet(void)
{
    if (verbose) puts("STUB: SecEditTrustAsSheet called");
    return NULL;
}

void* kcaddapplesharepassword(void)
{
    if (verbose) puts("STUB: kcaddapplesharepassword called");
    return NULL;
}

void* kcaddgenericpassword(void)
{
    if (verbose) puts("STUB: kcaddgenericpassword called");
    return NULL;
}

void* kcaddinternetpassword(void)
{
    if (verbose) puts("STUB: kcaddinternetpassword called");
    return NULL;
}

void* kcaddinternetpasswordwithpath(void)
{
    if (verbose) puts("STUB: kcaddinternetpasswordwithpath called");
    return NULL;
}

void* kccreatekeychain(void)
{
    if (verbose) puts("STUB: kccreatekeychain called");
    return NULL;
}

void* kcunlock(void)
{
    if (verbose) puts("STUB: kcunlock called");
    return NULL;
}

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


#include <SecurityInterface/SecurityInterface.h>
#include <stdlib.h>

static int verbose = 0;

__attribute__((constructor))
static void initme(void) {
    verbose = getenv("STUB_VERBOSE") != NULL;
}

void* _CommonCocoaPanels_ChooseIdentity(void) {
    if (verbose) puts("STUB: _CommonCocoaPanels_ChooseIdentity called");
    return NULL;
}

void* _CommonCocoaPanels_ChooseIdentitySheet(void) {
    if (verbose) puts("STUB: _CommonCocoaPanels_ChooseIdentitySheet called");
    return NULL;
}

void* _CommonCocoaPanels_CreateKeychainPanel(void) {
    if (verbose) puts("STUB: _CommonCocoaPanels_CreateKeychainPanel called");
    return NULL;
}

void* _CommonCocoaPanels_DisplayCertificate(void) {
    if (verbose) puts("STUB: _CommonCocoaPanels_DisplayCertificate called");
    return NULL;
}

void* _CommonCocoaPanels_DisplayCertificateGroup(void) {
    if (verbose) puts("STUB: _CommonCocoaPanels_DisplayCertificateGroup called");
    return NULL;
}

void* _CommonCocoaPanels_KeychainSettingsPanel(void) {
    if (verbose) puts("STUB: _CommonCocoaPanels_KeychainSettingsPanel called");
    return NULL;
}

void* _CommonCocoaPanels_SecEditTrust(void) {
    if (verbose) puts("STUB: _CommonCocoaPanels_SecEditTrust called");
    return NULL;
}

void* _CommonCocoaPanels_SecEditTrustSheet(void) {
    if (verbose) puts("STUB: _CommonCocoaPanels_SecEditTrustSheet called");
    return NULL;
}

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


#include <SecurityFoundation/SecurityFoundation.h>
#include <stdlib.h>
#include <stdio.h>

static int verbose = 0;

__attribute__((constructor))
static void initme(void) {
    verbose = getenv("STUB_VERBOSE") != NULL;
}

void* SFAccessibilityMakeWithMode(void) {
    if (verbose) puts("STUB: SFAccessibilityMakeWithMode called");
    return NULL;
}

void* SFKeychainServerXPCInterface(void) {
    if (verbose) puts("STUB: SFKeychainServerXPCInterface called");
    return NULL;
}

void* SFPWACStringInDictionary(void) {
    if (verbose) puts("STUB: SFPWACStringInDictionary called");
    return NULL;
}

void* SFPWAContextCreate(void) {
    if (verbose) puts("STUB: SFPWAContextCreate called");
    return NULL;
}

void* SFPWAContextCreateWithDefaults(void) {
    if (verbose) puts("STUB: SFPWAContextCreateWithDefaults called");
    return NULL;
}

void* SFPWAContextLoadDictionaries(void) {
    if (verbose) puts("STUB: SFPWAContextLoadDictionaries called");
    return NULL;
}

void* SFPWAContextRelease(void) {
    if (verbose) puts("STUB: SFPWAContextRelease called");
    return NULL;
}

void* SFPWAPasswordEvaluator(void) {
    if (verbose) puts("STUB: SFPWAPasswordEvaluator called");
    return NULL;
}

void* SFPWAPasswordSuggest(void) {
    if (verbose) puts("STUB: SFPWAPasswordSuggest called");
    return NULL;
}

void* SFPWAPolicyCopyDefault(void) {
    if (verbose) puts("STUB: SFPWAPolicyCopyDefault called");
    return NULL;
}

void* SFPWAPolicyCopyFromFile(void) {
    if (verbose) puts("STUB: SFPWAPolicyCopyFromFile called");
    return NULL;
}

void* SFPWAPolicyLanguages(void) {
    if (verbose) puts("STUB: SFPWAPolicyLanguages called");
    return NULL;
}

void* SFPWAPolicyParse(void) {
    if (verbose) puts("STUB: SFPWAPolicyParse called");
    return NULL;
}

void* SFPWARandomWord(void) {
    if (verbose) puts("STUB: SFPWARandomWord called");
    return NULL;
}

void* SFPWAStringInDictionary(void) {
    if (verbose) puts("STUB: SFPWAStringInDictionary called");
    return NULL;
}

void* SecFileVaultChangeMasterPasswordPassword(void) {
    if (verbose) puts("STUB: SecFileVaultChangeMasterPasswordPassword called");
    return NULL;
}

void* SecFileVaultCompact(void) {
    if (verbose) puts("STUB: SecFileVaultCompact called");
    return NULL;
}

void* SecFileVaultCreate(void) {
    if (verbose) puts("STUB: SecFileVaultCreate called");
    return NULL;
}

void* SecFileVaultCreateUsingCertificate(void) {
    if (verbose) puts("STUB: SecFileVaultCreateUsingCertificate called");
    return NULL;
}

void* SecFileVaultMakeMasterPassword(void) {
    if (verbose) puts("STUB: SecFileVaultMakeMasterPassword called");
    return NULL;
}

void* SecFileVaultMasterMount(void) {
    if (verbose) puts("STUB: SecFileVaultMasterMount called");
    return NULL;
}

void* SecFileVaultMasterPasswordEnabled(void) {
    if (verbose) puts("STUB: SecFileVaultMasterPasswordEnabled called");
    return NULL;
}

void* SecFileVaultMasterPasswordValidates(void) {
    if (verbose) puts("STUB: SecFileVaultMasterPasswordValidates called");
    return NULL;
}

void* SecFileVaultMount(void) {
    if (verbose) puts("STUB: SecFileVaultMount called");
    return NULL;
}

void* SecFileVaultRecover(void) {
    if (verbose) puts("STUB: SecFileVaultRecover called");
    return NULL;
}

void* SecFileVaultResize(void) {
    if (verbose) puts("STUB: SecFileVaultResize called");
    return NULL;
}

void* SecFileVaultUnmount(void) {
    if (verbose) puts("STUB: SecFileVaultUnmount called");
    return NULL;
}

void* SecFileVaultUserChangePassword(void) {
    if (verbose) puts("STUB: SecFileVaultUserChangePassword called");
    return NULL;
}

void* SecFileVaultUserMount(void) {
    if (verbose) puts("STUB: SecFileVaultUserMount called");
    return NULL;
}

void* _Z13SFPWARandWordlji(void) {
    if (verbose) puts("STUB: _Z13SFPWARandWordlji called");
    return NULL;
}

void* _Z17SecCreateSHA1HMACPK10__CFStringPK8__CFData(void) {
    if (verbose) puts("STUB: _Z17SecCreateSHA1HMACPK10__CFStringPK8__CFData called");
    return NULL;
}

void* _Z17SecVerifySHA1HMACPK10__CFStringPK8__CFDataS4_(void) {
    if (verbose) puts("STUB: _Z17SecVerifySHA1HMACPK10__CFStringPK8__CFDataS4_ called");
    return NULL;
}

void* _Z26SFPWAStdStringInDictionarylNSt3__112basic_stringIcNS_11char_traitsIcEENS_9allocatorIcEEEE(void) {
    if (verbose) puts("STUB: _Z26SFPWAStdStringInDictionarylNSt3__112basic_stringIcNS_11char_traitsIcEENS_9allocatorIcEEEE called");
    return NULL;
}

void* _ZN10CertParser10initFieldsEv(void) {
    if (verbose) puts("STUB: _ZN10CertParser10initFieldsEv called");
    return NULL;
}

void* _ZN10CertParser11fieldForOidERK9cssm_dataj(void) {
    if (verbose) puts("STUB: _ZN10CertParser11fieldForOidERK9cssm_dataj called");
    return NULL;
}

void* _ZN10CertParser12initWithDataERK9cssm_data(void) {
    if (verbose) puts("STUB: _ZN10CertParser12initWithDataERK9cssm_data called");
    return NULL;
}

void* _ZN10CertParser14initWithCFDataEPK8__CFData(void) {
    if (verbose) puts("STUB: _ZN10CertParser14initWithCFDataEPK8__CFData called");
    return NULL;
}

void* _ZN10CertParser15extensionForOidERK9cssm_data(void) {
    if (verbose) puts("STUB: _ZN10CertParser15extensionForOidERK9cssm_data called");
    return NULL;
}

void* _ZN10CertParser15initWithSecCertEP23OpaqueSecCertificateRef(void) {
    if (verbose) puts("STUB: _ZN10CertParser15initWithSecCertEP23OpaqueSecCertificateRef called");
    return NULL;
}

void* _ZN10CertParserC1EP23OpaqueSecCertificateRef(void) {
    if (verbose) puts("STUB: _ZN10CertParserC1EP23OpaqueSecCertificateRef called");
    return NULL;
}

void* _ZN10CertParserC1El(void) {
    if (verbose) puts("STUB: _ZN10CertParserC1El called");
    return NULL;
}

void* _ZN10CertParserC1ElRK9cssm_data(void) {
    if (verbose) puts("STUB: _ZN10CertParserC1ElRK9cssm_data called");
    return NULL;
}

void* _ZN10CertParserC1Ev(void) {
    if (verbose) puts("STUB: _ZN10CertParserC1Ev called");
    return NULL;
}

void* _ZN10CertParserC2EP23OpaqueSecCertificateRef(void) {
    if (verbose) puts("STUB: _ZN10CertParserC2EP23OpaqueSecCertificateRef called");
    return NULL;
}

void* _ZN10CertParserC2El(void) {
    if (verbose) puts("STUB: _ZN10CertParserC2El called");
    return NULL;
}

void* _ZN10CertParserC2ElRK9cssm_data(void) {
    if (verbose) puts("STUB: _ZN10CertParserC2ElRK9cssm_data called");
    return NULL;
}

void* _ZN10CertParserC2Ev(void) {
    if (verbose) puts("STUB: _ZN10CertParserC2Ev called");
    return NULL;
}

void* _ZN10CertParserD1Ev(void) {
    if (verbose) puts("STUB: _ZN10CertParserD1Ev called");
    return NULL;
}

void* _ZN10CertParserD2Ev(void) {
    if (verbose) puts("STUB: _ZN10CertParserD2Ev called");
    return NULL;
}

void* _copyIdentityForCMSDecoder(void) {
    if (verbose) puts("STUB: _copyIdentityForCMSDecoder called");
    return NULL;
}

void* caIPAddressIntValue(void) {
    if (verbose) puts("STUB: caIPAddressIntValue called");
    return NULL;
}

void* certPrintName(void) {
    if (verbose) puts("STUB: certPrintName called");
    return NULL;
}

void* createCertCsr(void) {
    if (verbose) puts("STUB: createCertCsr called");
    return NULL;
}

void* nssNameToCSSM(void) {
    if (verbose) puts("STUB: nssNameToCSSM called");
    return NULL;
}

void* subjPubKeyInfoToCssmKey(void) {
    if (verbose) puts("STUB: subjPubKeyInfoToCssmKey called");
    return NULL;
}

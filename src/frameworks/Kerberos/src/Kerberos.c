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


#include <Kerberos/Kerberos.h>
#include <stdlib.h>
#include <stdio.h>

static int verbose = 0;

__attribute__((constructor))
static void initme(void) {
    verbose = getenv("STUB_VERBOSE") != NULL;
}

void* FSp_krb_get_svc_in_tkt(void)
{
    if (verbose) puts("STUB: FSp_krb_get_svc_in_tkt called");
    return NULL;
}

void* FSp_profile_init(void)
{
    if (verbose) puts("STUB: FSp_profile_init called");
    return NULL;
}

void* FSp_profile_init_path(void)
{
    if (verbose) puts("STUB: FSp_profile_init_path called");
    return NULL;
}

void* FSp_put_svc_key(void)
{
    if (verbose) puts("STUB: FSp_put_svc_key called");
    return NULL;
}

void* FSp_read_service_key(void)
{
    if (verbose) puts("STUB: FSp_read_service_key called");
    return NULL;
}

void* K5ClientGetAuthenticatorForServiceDeprecated(void)
{
    if (verbose) puts("STUB: K5ClientGetAuthenticatorForServiceDeprecated called");
    return NULL;
}

void* K5ClientGetTicketForServiceDeprecated(void)
{
    if (verbose) puts("STUB: K5ClientGetTicketForServiceDeprecated called");
    return NULL;
}

void* KClientAddCredentialsDeprecated(void)
{
    if (verbose) puts("STUB: KClientAddCredentialsDeprecated called");
    return NULL;
}

void* KClientAddRealmMapDeprecated(void)
{
    if (verbose) puts("STUB: KClientAddRealmMapDeprecated called");
    return NULL;
}

void* KClientAddServerMapDeprecated(void)
{
    if (verbose) puts("STUB: KClientAddServerMapDeprecated called");
    return NULL;
}

void* KClientAddServiceKey(void)
{
    if (verbose) puts("STUB: KClientAddServiceKey called");
    return NULL;
}

void* KClientCacheInitialTicketDeprecated(void)
{
    if (verbose) puts("STUB: KClientCacheInitialTicketDeprecated called");
    return NULL;
}

void* KClientDecrypt(void)
{
    if (verbose) puts("STUB: KClientDecrypt called");
    return NULL;
}

void* KClientDecryptCompat(void)
{
    if (verbose) puts("STUB: KClientDecryptCompat called");
    return NULL;
}

void* KClientDeleteCredentialsDeprecated(void)
{
    if (verbose) puts("STUB: KClientDeleteCredentialsDeprecated called");
    return NULL;
}

void* KClientDeleteRealmMapDeprecated(void)
{
    if (verbose) puts("STUB: KClientDeleteRealmMapDeprecated called");
    return NULL;
}

void* KClientDeleteServerMapDeprecated(void)
{
    if (verbose) puts("STUB: KClientDeleteServerMapDeprecated called");
    return NULL;
}

void* KClientDeleteSessionDeprecated(void)
{
    if (verbose) puts("STUB: KClientDeleteSessionDeprecated called");
    return NULL;
}

void* KClientDisposePrincipal(void)
{
    if (verbose) puts("STUB: KClientDisposePrincipal called");
    return NULL;
}

void* KClientDisposeSession(void)
{
    if (verbose) puts("STUB: KClientDisposeSession called");
    return NULL;
}

void* KClientDisposeSessionCompat(void)
{
    if (verbose) puts("STUB: KClientDisposeSessionCompat called");
    return NULL;
}

void* KClientEncrypt(void)
{
    if (verbose) puts("STUB: KClientEncrypt called");
    return NULL;
}

void* KClientEncryptCompat(void)
{
    if (verbose) puts("STUB: KClientEncryptCompat called");
    return NULL;
}

void* KClientGetAuthenticatorForService(void)
{
    if (verbose) puts("STUB: KClientGetAuthenticatorForService called");
    return NULL;
}

void* KClientGetCCacheReference(void)
{
    if (verbose) puts("STUB: KClientGetCCacheReference called");
    return NULL;
}

void* KClientGetClientPrincipal(void)
{
    if (verbose) puts("STUB: KClientGetClientPrincipal called");
    return NULL;
}

void* KClientGetCredentialsDeprecated(void)
{
    if (verbose) puts("STUB: KClientGetCredentialsDeprecated called");
    return NULL;
}

void* KClientGetEncryptedServiceReply(void)
{
    if (verbose) puts("STUB: KClientGetEncryptedServiceReply called");
    return NULL;
}

void* KClientGetErrorTextDeprecated(void)
{
    if (verbose) puts("STUB: KClientGetErrorTextDeprecated called");
    return NULL;
}

void* KClientGetExpirationTime(void)
{
    if (verbose) puts("STUB: KClientGetExpirationTime called");
    return NULL;
}

void* KClientGetLocalAddress(void)
{
    if (verbose) puts("STUB: KClientGetLocalAddress called");
    return NULL;
}

void* KClientGetLocalRealmDeprecated(void)
{
    if (verbose) puts("STUB: KClientGetLocalRealmDeprecated called");
    return NULL;
}

void* KClientGetNthCredentialDeprecated(void)
{
    if (verbose) puts("STUB: KClientGetNthCredentialDeprecated called");
    return NULL;
}

void* KClientGetNthRealmMapDeprecated(void)
{
    if (verbose) puts("STUB: KClientGetNthRealmMapDeprecated called");
    return NULL;
}

void* KClientGetNthServerDeprecated(void)
{
    if (verbose) puts("STUB: KClientGetNthServerDeprecated called");
    return NULL;
}

void* KClientGetNthServerMapDeprecated(void)
{
    if (verbose) puts("STUB: KClientGetNthServerMapDeprecated called");
    return NULL;
}

void* KClientGetNthServerPortDeprecated(void)
{
    if (verbose) puts("STUB: KClientGetNthServerPortDeprecated called");
    return NULL;
}

void* KClientGetNthSessionDeprecated(void)
{
    if (verbose) puts("STUB: KClientGetNthSessionDeprecated called");
    return NULL;
}

void* KClientGetNumCredentialsDeprecated(void)
{
    if (verbose) puts("STUB: KClientGetNumCredentialsDeprecated called");
    return NULL;
}

void* KClientGetNumSessionsDeprecated(void)
{
    if (verbose) puts("STUB: KClientGetNumSessionsDeprecated called");
    return NULL;
}

void* KClientGetProtectedServiceReply(void)
{
    if (verbose) puts("STUB: KClientGetProtectedServiceReply called");
    return NULL;
}

void* KClientGetRealmDeprecated(void)
{
    if (verbose) puts("STUB: KClientGetRealmDeprecated called");
    return NULL;
}

void* KClientGetRemoteAddress(void)
{
    if (verbose) puts("STUB: KClientGetRemoteAddress called");
    return NULL;
}

void* KClientGetServerPrincipal(void)
{
    if (verbose) puts("STUB: KClientGetServerPrincipal called");
    return NULL;
}

void* KClientGetServiceKey(void)
{
    if (verbose) puts("STUB: KClientGetServiceKey called");
    return NULL;
}

void* KClientGetSessionKey(void)
{
    if (verbose) puts("STUB: KClientGetSessionKey called");
    return NULL;
}

void* KClientGetSessionKeyCompat(void)
{
    if (verbose) puts("STUB: KClientGetSessionKeyCompat called");
    return NULL;
}

void* KClientGetSessionUserNameCompat(void)
{
    if (verbose) puts("STUB: KClientGetSessionUserNameCompat called");
    return NULL;
}

void* KClientGetTicketForService(void)
{
    if (verbose) puts("STUB: KClientGetTicketForService called");
    return NULL;
}

void* KClientGetTicketForServiceCompat(void)
{
    if (verbose) puts("STUB: KClientGetTicketForServiceCompat called");
    return NULL;
}

void* KClientGetTicketForServiceWithChecksumCompat(void)
{
    if (verbose) puts("STUB: KClientGetTicketForServiceWithChecksumCompat called");
    return NULL;
}

void* KClientGetUserNameDeprecated(void)
{
    if (verbose) puts("STUB: KClientGetUserNameDeprecated called");
    return NULL;
}

void* KClientGetVersion(void)
{
    if (verbose) puts("STUB: KClientGetVersion called");
    return NULL;
}

void* KClientKeyFileLogin(void)
{
    if (verbose) puts("STUB: KClientKeyFileLogin called");
    return NULL;
}

void* KClientLogin(void)
{
    if (verbose) puts("STUB: KClientLogin called");
    return NULL;
}

void* KClientLoginCompat(void)
{
    if (verbose) puts("STUB: KClientLoginCompat called");
    return NULL;
}

void* KClientLogout(void)
{
    if (verbose) puts("STUB: KClientLogout called");
    return NULL;
}

void* KClientLogoutCompat(void)
{
    if (verbose) puts("STUB: KClientLogoutCompat called");
    return NULL;
}

void* KClientMakeSendAuthCompat(void)
{
    if (verbose) puts("STUB: KClientMakeSendAuthCompat called");
    return NULL;
}

void* KClientNewClientSession(void)
{
    if (verbose) puts("STUB: KClientNewClientSession called");
    return NULL;
}

void* KClientNewServerSession(void)
{
    if (verbose) puts("STUB: KClientNewServerSession called");
    return NULL;
}

void* KClientNewSessionCompat(void)
{
    if (verbose) puts("STUB: KClientNewSessionCompat called");
    return NULL;
}

void* KClientPasswordLogin(void)
{
    if (verbose) puts("STUB: KClientPasswordLogin called");
    return NULL;
}

void* KClientPasswordLoginCompat(void)
{
    if (verbose) puts("STUB: KClientPasswordLoginCompat called");
    return NULL;
}

void* KClientPrincipalToV4String(void)
{
    if (verbose) puts("STUB: KClientPrincipalToV4String called");
    return NULL;
}

void* KClientPrincipalToV4Triplet(void)
{
    if (verbose) puts("STUB: KClientPrincipalToV4Triplet called");
    return NULL;
}

void* KClientProtectIntegrity(void)
{
    if (verbose) puts("STUB: KClientProtectIntegrity called");
    return NULL;
}

void* KClientProtectIntegrityCompat(void)
{
    if (verbose) puts("STUB: KClientProtectIntegrityCompat called");
    return NULL;
}

void* KClientSetClientPrincipal(void)
{
    if (verbose) puts("STUB: KClientSetClientPrincipal called");
    return NULL;
}

void* KClientSetKeyFile(void)
{
    if (verbose) puts("STUB: KClientSetKeyFile called");
    return NULL;
}

void* KClientSetLocalAddress(void)
{
    if (verbose) puts("STUB: KClientSetLocalAddress called");
    return NULL;
}

void* KClientSetLocalRealmDeprecated(void)
{
    if (verbose) puts("STUB: KClientSetLocalRealmDeprecated called");
    return NULL;
}

void* KClientSetNthServerPortDeprecated(void)
{
    if (verbose) puts("STUB: KClientSetNthServerPortDeprecated called");
    return NULL;
}

void* KClientSetRemoteAddress(void)
{
    if (verbose) puts("STUB: KClientSetRemoteAddress called");
    return NULL;
}

void* KClientSetServerPrincipal(void)
{
    if (verbose) puts("STUB: KClientSetServerPrincipal called");
    return NULL;
}

void* KClientStatusCompat(void)
{
    if (verbose) puts("STUB: KClientStatusCompat called");
    return NULL;
}

void* KClientV4StringToPrincipal(void)
{
    if (verbose) puts("STUB: KClientV4StringToPrincipal called");
    return NULL;
}

void* KClientVerifyAuthenticator(void)
{
    if (verbose) puts("STUB: KClientVerifyAuthenticator called");
    return NULL;
}

void* KClientVerifyEncryptedServiceReply(void)
{
    if (verbose) puts("STUB: KClientVerifyEncryptedServiceReply called");
    return NULL;
}

void* KClientVerifyIntegrity(void)
{
    if (verbose) puts("STUB: KClientVerifyIntegrity called");
    return NULL;
}

void* KClientVerifyIntegrityCompat(void)
{
    if (verbose) puts("STUB: KClientVerifyIntegrityCompat called");
    return NULL;
}

void* KClientVerifyProtectedServiceReply(void)
{
    if (verbose) puts("STUB: KClientVerifyProtectedServiceReply called");
    return NULL;
}

void* KClientVerifyReplyTicketCompat(void)
{
    if (verbose) puts("STUB: KClientVerifyReplyTicketCompat called");
    return NULL;
}

void* KClientVerifyUnencryptedReplyTicketCompat(void)
{
    if (verbose) puts("STUB: KClientVerifyUnencryptedReplyTicketCompat called");
    return NULL;
}

void* KClientVersionCompat(void)
{
    if (verbose) puts("STUB: KClientVersionCompat called");
    return NULL;
}

void* KLAcquireInitialTickets(void)
{
    if (verbose) puts("STUB: KLAcquireInitialTickets called");
    return NULL;
}

void* KLAcquireInitialTicketsWithPassword(void)
{
    if (verbose) puts("STUB: KLAcquireInitialTicketsWithPassword called");
    return NULL;
}

void* KLAcquireNewInitialTicketCredentialsWithPassword(void)
{
    if (verbose) puts("STUB: KLAcquireNewInitialTicketCredentialsWithPassword called");
    return NULL;
}

void* KLAcquireNewInitialTickets(void)
{
    if (verbose) puts("STUB: KLAcquireNewInitialTickets called");
    return NULL;
}

void* KLAcquireNewInitialTicketsWithKeytab(void)
{
    if (verbose) puts("STUB: KLAcquireNewInitialTicketsWithKeytab called");
    return NULL;
}

void* KLAcquireNewInitialTicketsWithPassword(void)
{
    if (verbose) puts("STUB: KLAcquireNewInitialTicketsWithPassword called");
    return NULL;
}

void* KLAcquireNewTickets(void)
{
    if (verbose) puts("STUB: KLAcquireNewTickets called");
    return NULL;
}

void* KLAcquireNewTicketsWithPassword(void)
{
    if (verbose) puts("STUB: KLAcquireNewTicketsWithPassword called");
    return NULL;
}

void* KLAcquireTickets(void)
{
    if (verbose) puts("STUB: KLAcquireTickets called");
    return NULL;
}

void* KLAcquireTicketsWithPassword(void)
{
    if (verbose) puts("STUB: KLAcquireTicketsWithPassword called");
    return NULL;
}

void* KLCacheHasValidTickets(void)
{
    if (verbose) puts("STUB: KLCacheHasValidTickets called");
    return NULL;
}

void* KLCancelAllDialogs(void)
{
    if (verbose) puts("STUB: KLCancelAllDialogs called");
    return NULL;
}

void* KLChangePassword(void)
{
    if (verbose) puts("STUB: KLChangePassword called");
    return NULL;
}

void* KLChangePasswordWithPasswords(void)
{
    if (verbose) puts("STUB: KLChangePasswordWithPasswords called");
    return NULL;
}

void* KLComparePrincipal(void)
{
    if (verbose) puts("STUB: KLComparePrincipal called");
    return NULL;
}

void* KLCountKerberosRealms(void)
{
    if (verbose) puts("STUB: KLCountKerberosRealms called");
    return NULL;
}

void* KLCreateLoginOptions(void)
{
    if (verbose) puts("STUB: KLCreateLoginOptions called");
    return NULL;
}

void* KLCreatePrincipalFromKerberos5Principal(void)
{
    if (verbose) puts("STUB: KLCreatePrincipalFromKerberos5Principal called");
    return NULL;
}

void* KLCreatePrincipalFromPrincipal(void)
{
    if (verbose) puts("STUB: KLCreatePrincipalFromPrincipal called");
    return NULL;
}

void* KLCreatePrincipalFromString(void)
{
    if (verbose) puts("STUB: KLCreatePrincipalFromString called");
    return NULL;
}

void* KLCreatePrincipalFromTriplet(void)
{
    if (verbose) puts("STUB: KLCreatePrincipalFromTriplet called");
    return NULL;
}

void* KLDestroyTickets(void)
{
    if (verbose) puts("STUB: KLDestroyTickets called");
    return NULL;
}

void* KLDisposeLoginOptions(void)
{
    if (verbose) puts("STUB: KLDisposeLoginOptions called");
    return NULL;
}

void* KLDisposePrincipal(void)
{
    if (verbose) puts("STUB: KLDisposePrincipal called");
    return NULL;
}

void* KLDisposeString(void)
{
    if (verbose) puts("STUB: KLDisposeString called");
    return NULL;
}

void* KLFindKerberosRealmByName(void)
{
    if (verbose) puts("STUB: KLFindKerberosRealmByName called");
    return NULL;
}

void* KLGetApplicationOptions(void)
{
    if (verbose) puts("STUB: KLGetApplicationOptions called");
    return NULL;
}

void* KLGetDefaultLoginOption(void)
{
    if (verbose) puts("STUB: KLGetDefaultLoginOption called");
    return NULL;
}

void* KLGetDisplayStringFromPrincipal(void)
{
    if (verbose) puts("STUB: KLGetDisplayStringFromPrincipal called");
    return NULL;
}

void* KLGetErrorString(void)
{
    if (verbose) puts("STUB: KLGetErrorString called");
    return NULL;
}

void* KLGetIdleCallback(void)
{
    if (verbose) puts("STUB: KLGetIdleCallback called");
    return NULL;
}

void* KLGetKerberosDefaultRealm(void)
{
    if (verbose) puts("STUB: KLGetKerberosDefaultRealm called");
    return NULL;
}

void* KLGetKerberosDefaultRealmByName(void)
{
    if (verbose) puts("STUB: KLGetKerberosDefaultRealmByName called");
    return NULL;
}

void* KLGetKerberosRealm(void)
{
    if (verbose) puts("STUB: KLGetKerberosRealm called");
    return NULL;
}

void* KLGetStringFromPrincipal(void)
{
    if (verbose) puts("STUB: KLGetStringFromPrincipal called");
    return NULL;
}

void* KLGetTripletFromPrincipal(void)
{
    if (verbose) puts("STUB: KLGetTripletFromPrincipal called");
    return NULL;
}

void* KLHandleError(void)
{
    if (verbose) puts("STUB: KLHandleError called");
    return NULL;
}

void* KLInsertKerberosRealm(void)
{
    if (verbose) puts("STUB: KLInsertKerberosRealm called");
    return NULL;
}

void* KLLastChangedTime(void)
{
    if (verbose) puts("STUB: KLLastChangedTime called");
    return NULL;
}

void* KLLoginOptionsSetAddressless(void)
{
    if (verbose) puts("STUB: KLLoginOptionsSetAddressless called");
    return NULL;
}

void* KLLoginOptionsSetForwardable(void)
{
    if (verbose) puts("STUB: KLLoginOptionsSetForwardable called");
    return NULL;
}

void* KLLoginOptionsSetProxiable(void)
{
    if (verbose) puts("STUB: KLLoginOptionsSetProxiable called");
    return NULL;
}

void* KLLoginOptionsSetRenewableLifetime(void)
{
    if (verbose) puts("STUB: KLLoginOptionsSetRenewableLifetime called");
    return NULL;
}

void* KLLoginOptionsSetServiceName(void)
{
    if (verbose) puts("STUB: KLLoginOptionsSetServiceName called");
    return NULL;
}

void* KLLoginOptionsSetTicketLifetime(void)
{
    if (verbose) puts("STUB: KLLoginOptionsSetTicketLifetime called");
    return NULL;
}

void* KLLoginOptionsSetTicketStartTime(void)
{
    if (verbose) puts("STUB: KLLoginOptionsSetTicketStartTime called");
    return NULL;
}

void* KLRemoveAllKerberosRealms(void)
{
    if (verbose) puts("STUB: KLRemoveAllKerberosRealms called");
    return NULL;
}

void* KLRemoveKerberosRealm(void)
{
    if (verbose) puts("STUB: KLRemoveKerberosRealm called");
    return NULL;
}

void* KLRenewInitialTickets(void)
{
    if (verbose) puts("STUB: KLRenewInitialTickets called");
    return NULL;
}

void* KLSetApplicationOptions(void)
{
    if (verbose) puts("STUB: KLSetApplicationOptions called");
    return NULL;
}

void* KLSetDefaultLoginOption(void)
{
    if (verbose) puts("STUB: KLSetDefaultLoginOption called");
    return NULL;
}

void* KLSetIdleCallback(void)
{
    if (verbose) puts("STUB: KLSetIdleCallback called");
    return NULL;
}

void* KLSetKerberosDefaultRealm(void)
{
    if (verbose) puts("STUB: KLSetKerberosDefaultRealm called");
    return NULL;
}

void* KLSetKerberosDefaultRealmByName(void)
{
    if (verbose) puts("STUB: KLSetKerberosDefaultRealmByName called");
    return NULL;
}

void* KLSetKerberosRealm(void)
{
    if (verbose) puts("STUB: KLSetKerberosRealm called");
    return NULL;
}

void* KLSetSystemDefaultCache(void)
{
    if (verbose) puts("STUB: KLSetSystemDefaultCache called");
    return NULL;
}

void* KLStoreNewInitialTicketCredentials(void)
{
    if (verbose) puts("STUB: KLStoreNewInitialTicketCredentials called");
    return NULL;
}

void* KLTicketExpirationTime(void)
{
    if (verbose) puts("STUB: KLTicketExpirationTime called");
    return NULL;
}

void* KLTicketStartTime(void)
{
    if (verbose) puts("STUB: KLTicketStartTime called");
    return NULL;
}

void* KLValidateInitialTickets(void)
{
    if (verbose) puts("STUB: KLValidateInitialTickets called");
    return NULL;
}

void* KLVerifyInitialTicketCredentials(void)
{
    if (verbose) puts("STUB: KLVerifyInitialTicketCredentials called");
    return NULL;
}

void* KLVerifyInitialTickets(void)
{
    if (verbose) puts("STUB: KLVerifyInitialTickets called");
    return NULL;
}

void* KPCreatePreferencesFile(void)
{
    if (verbose) puts("STUB: KPCreatePreferencesFile called");
    return NULL;
}

void* KPFreeListOfPreferencesFiles(void)
{
    if (verbose) puts("STUB: KPFreeListOfPreferencesFiles called");
    return NULL;
}

void* KPGetListOfPreferencesFiles(void)
{
    if (verbose) puts("STUB: KPGetListOfPreferencesFiles called");
    return NULL;
}

void* KPInitializeWithDefaultKerberosLibraryPreferences(void)
{
    if (verbose) puts("STUB: KPInitializeWithDefaultKerberosLibraryPreferences called");
    return NULL;
}

void* KPPreferencesFileIsReadable(void)
{
    if (verbose) puts("STUB: KPPreferencesFileIsReadable called");
    return NULL;
}

void* KPPreferencesFileIsWritable(void)
{
    if (verbose) puts("STUB: KPPreferencesFileIsWritable called");
    return NULL;
}

void* KServerAddKeyCompat(void)
{
    if (verbose) puts("STUB: KServerAddKeyCompat called");
    return NULL;
}

void* KServerGetKeyCompat(void)
{
    if (verbose) puts("STUB: KServerGetKeyCompat called");
    return NULL;
}

void* KServerGetReplyTicketCompat(void)
{
    if (verbose) puts("STUB: KServerGetReplyTicketCompat called");
    return NULL;
}

void* KServerGetSessionTimeRemainingCompat(void)
{
    if (verbose) puts("STUB: KServerGetSessionTimeRemainingCompat called");
    return NULL;
}

void* KServerNewSessionCompat(void)
{
    if (verbose) puts("STUB: KServerNewSessionCompat called");
    return NULL;
}

void* KServerVerifyTicketCompat(void)
{
    if (verbose) puts("STUB: KServerVerifyTicketCompat called");
    return NULL;
}

void* __KLAllowAutomaticPrompting(void)
{
    if (verbose) puts("STUB: __KLAllowAutomaticPrompting called");
    return NULL;
}

void* __KLAllowHomeDirectoryAccess(void)
{
    if (verbose) puts("STUB: __KLAllowHomeDirectoryAccess called");
    return NULL;
}

void* __KLAllowRememberPassword(void)
{
    if (verbose) puts("STUB: __KLAllowRememberPassword called");
    return NULL;
}

void* __KLCreatePrincipalFromKerberos5Principal(void)
{
    if (verbose) puts("STUB: __KLCreatePrincipalFromKerberos5Principal called");
    return NULL;
}

void* __KLCreatePrincipalFromTriplet(void)
{
    if (verbose) puts("STUB: __KLCreatePrincipalFromTriplet called");
    return NULL;
}

void* __KLGetKerberos5PrincipalFromPrincipal(void)
{
    if (verbose) puts("STUB: __KLGetKerberos5PrincipalFromPrincipal called");
    return NULL;
}

void* __KLGetKeychainPasswordForPrincipal(void)
{
    if (verbose) puts("STUB: __KLGetKeychainPasswordForPrincipal called");
    return NULL;
}

void* __KLGetTripletFromPrincipal(void)
{
    if (verbose) puts("STUB: __KLGetTripletFromPrincipal called");
    return NULL;
}

void* __KLPrincipalIsTicketGrantingService(void)
{
    if (verbose) puts("STUB: __KLPrincipalIsTicketGrantingService called");
    return NULL;
}

void* __KLPrincipalSetKeychainPassword(void)
{
    if (verbose) puts("STUB: __KLPrincipalSetKeychainPassword called");
    return NULL;
}

void* __KLPromptMechanism(void)
{
    if (verbose) puts("STUB: __KLPromptMechanism called");
    return NULL;
}

void* __KLRemoveKeychainPasswordForPrincipal(void)
{
    if (verbose) puts("STUB: __KLRemoveKeychainPasswordForPrincipal called");
    return NULL;
}

void* __KLSetApplicationPrompter(void)
{
    if (verbose) puts("STUB: __KLSetApplicationPrompter called");
    return NULL;
}

void* __KLSetAutomaticPrompting(void)
{
    if (verbose) puts("STUB: __KLSetAutomaticPrompting called");
    return NULL;
}

void* __KLSetHomeDirectoryAccess(void)
{
    if (verbose) puts("STUB: __KLSetHomeDirectoryAccess called");
    return NULL;
}

void* __KLSetPromptMechanism(void)
{
    if (verbose) puts("STUB: __KLSetPromptMechanism called");
    return NULL;
}

void* __KerberosDebugLogLevel(void)
{
    if (verbose) puts("STUB: __KerberosDebugLogLevel called");
    return NULL;
}

void* __KerberosDebugPrint(void)
{
    if (verbose) puts("STUB: __KerberosDebugPrint called");
    return NULL;
}

void* __KerberosDebugPrintMemory(void)
{
    if (verbose) puts("STUB: __KerberosDebugPrintMemory called");
    return NULL;
}

void* __KerberosDebugPrintSession(void)
{
    if (verbose) puts("STUB: __KerberosDebugPrintSession called");
    return NULL;
}

void* __KerberosDebugVAPrint(void)
{
    if (verbose) puts("STUB: __KerberosDebugVAPrint called");
    return NULL;
}

void* __KerberosInternal_krb5int_sendtokdc_debug_handler(void)
{
    if (verbose) puts("STUB: __KerberosInternal_krb5int_sendtokdc_debug_handler called");
    return NULL;
}

void* add_error_table(void)
{
    if (verbose) puts("STUB: add_error_table called");
    return NULL;
}

void* apple_gss_krb5_export_authdata_if_relevant_context(void)
{
    if (verbose) puts("STUB: apple_gss_krb5_export_authdata_if_relevant_context called");
    return NULL;
}

void* apple_gss_krb5_free_authdata_if_relevant(void)
{
    if (verbose) puts("STUB: apple_gss_krb5_free_authdata_if_relevant called");
    return NULL;
}

void* cc_close(void)
{
    if (verbose) puts("STUB: cc_close called");
    return NULL;
}

void* cc_create(void)
{
    if (verbose) puts("STUB: cc_create called");
    return NULL;
}

void* cc_destroy(void)
{
    if (verbose) puts("STUB: cc_destroy called");
    return NULL;
}

void* cc_free_NC_info(void)
{
    if (verbose) puts("STUB: cc_free_NC_info called");
    return NULL;
}

void* cc_free_creds(void)
{
    if (verbose) puts("STUB: cc_free_creds called");
    return NULL;
}

void* cc_free_name(void)
{
    if (verbose) puts("STUB: cc_free_name called");
    return NULL;
}

void* cc_free_principal(void)
{
    if (verbose) puts("STUB: cc_free_principal called");
    return NULL;
}

void* cc_get_NC_info(void)
{
    if (verbose) puts("STUB: cc_get_NC_info called");
    return NULL;
}

void* cc_get_change_time(void)
{
    if (verbose) puts("STUB: cc_get_change_time called");
    return NULL;
}

void* cc_get_cred_version(void)
{
    if (verbose) puts("STUB: cc_get_cred_version called");
    return NULL;
}

void* cc_get_name(void)
{
    if (verbose) puts("STUB: cc_get_name called");
    return NULL;
}

void* cc_get_principal(void)
{
    if (verbose) puts("STUB: cc_get_principal called");
    return NULL;
}

void* cc_initialize(void)
{
    if (verbose) puts("STUB: cc_initialize called");
    return NULL;
}

void* cc_open(void)
{
    if (verbose) puts("STUB: cc_open called");
    return NULL;
}

void* cc_remove_cred(void)
{
    if (verbose) puts("STUB: cc_remove_cred called");
    return NULL;
}

void* cc_seq_fetch_NCs_begin(void)
{
    if (verbose) puts("STUB: cc_seq_fetch_NCs_begin called");
    return NULL;
}

void* cc_seq_fetch_NCs_end(void)
{
    if (verbose) puts("STUB: cc_seq_fetch_NCs_end called");
    return NULL;
}

void* cc_seq_fetch_NCs_next(void)
{
    if (verbose) puts("STUB: cc_seq_fetch_NCs_next called");
    return NULL;
}

void* cc_seq_fetch_creds_begin(void)
{
    if (verbose) puts("STUB: cc_seq_fetch_creds_begin called");
    return NULL;
}

void* cc_seq_fetch_creds_end(void)
{
    if (verbose) puts("STUB: cc_seq_fetch_creds_end called");
    return NULL;
}

void* cc_seq_fetch_creds_next(void)
{
    if (verbose) puts("STUB: cc_seq_fetch_creds_next called");
    return NULL;
}

void* cc_set_principal(void)
{
    if (verbose) puts("STUB: cc_set_principal called");
    return NULL;
}

void* cc_shutdown(void)
{
    if (verbose) puts("STUB: cc_shutdown called");
    return NULL;
}

void* cc_store(void)
{
    if (verbose) puts("STUB: cc_store called");
    return NULL;
}

void* com_err(void)
{
    if (verbose) puts("STUB: com_err called");
    return NULL;
}

void* com_err_va(void)
{
    if (verbose) puts("STUB: com_err_va called");
    return NULL;
}

void* decomp_ticket(void)
{
    if (verbose) puts("STUB: decomp_ticket called");
    return NULL;
}

void* dest_all_tkts(void)
{
    if (verbose) puts("STUB: dest_all_tkts called");
    return NULL;
}

void* dest_tkt(void)
{
    if (verbose) puts("STUB: dest_tkt called");
    return NULL;
}

void* encode_krb5_as_req(void)
{
    if (verbose) puts("STUB: encode_krb5_as_req called");
    return NULL;
}

void* error_message(void)
{
    if (verbose) puts("STUB: error_message called");
    return NULL;
}

void* get_ad_tkt(void)
{
    if (verbose) puts("STUB: get_ad_tkt called");
    return NULL;
}

void* get_pw_tkt(void)
{
    if (verbose) puts("STUB: get_pw_tkt called");
    return NULL;
}

void* gss_accept_sec_context(void)
{
    if (verbose) puts("STUB: gss_accept_sec_context called");
    return NULL;
}

void* gss_acquire_cred(void)
{
    if (verbose) puts("STUB: gss_acquire_cred called");
    return NULL;
}

void* gss_add_cred(void)
{
    if (verbose) puts("STUB: gss_add_cred called");
    return NULL;
}

void* gss_add_oid_set_member(void)
{
    if (verbose) puts("STUB: gss_add_oid_set_member called");
    return NULL;
}

void* gss_canonicalize_name(void)
{
    if (verbose) puts("STUB: gss_canonicalize_name called");
    return NULL;
}

void* gss_compare_name(void)
{
    if (verbose) puts("STUB: gss_compare_name called");
    return NULL;
}

void* gss_context_time(void)
{
    if (verbose) puts("STUB: gss_context_time called");
    return NULL;
}

void* gss_create_empty_oid_set(void)
{
    if (verbose) puts("STUB: gss_create_empty_oid_set called");
    return NULL;
}

void* gss_delete_sec_context(void)
{
    if (verbose) puts("STUB: gss_delete_sec_context called");
    return NULL;
}

void* gss_display_name(void)
{
    if (verbose) puts("STUB: gss_display_name called");
    return NULL;
}

void* gss_display_status(void)
{
    if (verbose) puts("STUB: gss_display_status called");
    return NULL;
}

void* gss_duplicate_name(void)
{
    if (verbose) puts("STUB: gss_duplicate_name called");
    return NULL;
}

void* gss_export_name(void)
{
    if (verbose) puts("STUB: gss_export_name called");
    return NULL;
}

void* gss_export_sec_context(void)
{
    if (verbose) puts("STUB: gss_export_sec_context called");
    return NULL;
}

void* gss_get_mic(void)
{
    if (verbose) puts("STUB: gss_get_mic called");
    return NULL;
}

void* gss_import_name(void)
{
    if (verbose) puts("STUB: gss_import_name called");
    return NULL;
}

void* gss_import_sec_context(void)
{
    if (verbose) puts("STUB: gss_import_sec_context called");
    return NULL;
}

void* gss_indicate_mechs(void)
{
    if (verbose) puts("STUB: gss_indicate_mechs called");
    return NULL;
}

void* gss_init_sec_context(void)
{
    if (verbose) puts("STUB: gss_init_sec_context called");
    return NULL;
}

void* gss_inquire_context(void)
{
    if (verbose) puts("STUB: gss_inquire_context called");
    return NULL;
}

void* gss_inquire_cred(void)
{
    if (verbose) puts("STUB: gss_inquire_cred called");
    return NULL;
}

void* gss_inquire_cred_by_mech(void)
{
    if (verbose) puts("STUB: gss_inquire_cred_by_mech called");
    return NULL;
}

void* gss_inquire_mechs_for_name(void)
{
    if (verbose) puts("STUB: gss_inquire_mechs_for_name called");
    return NULL;
}

void* gss_inquire_names_for_mech(void)
{
    if (verbose) puts("STUB: gss_inquire_names_for_mech called");
    return NULL;
}

void* gss_krb5_ccache_name(void)
{
    if (verbose) puts("STUB: gss_krb5_ccache_name called");
    return NULL;
}

void* gss_krb5_copy_ccache(void)
{
    if (verbose) puts("STUB: gss_krb5_copy_ccache called");
    return NULL;
}

void* gss_krb5_export_lucid_sec_context(void)
{
    if (verbose) puts("STUB: gss_krb5_export_lucid_sec_context called");
    return NULL;
}

void* gss_krb5_free_lucid_sec_context(void)
{
    if (verbose) puts("STUB: gss_krb5_free_lucid_sec_context called");
    return NULL;
}

void* gss_krb5_get_tkt_flags(void)
{
    if (verbose) puts("STUB: gss_krb5_get_tkt_flags called");
    return NULL;
}

void* gss_krb5_set_allowable_enctypes(void)
{
    if (verbose) puts("STUB: gss_krb5_set_allowable_enctypes called");
    return NULL;
}

void* gss_krb5_ui(void)
{
    if (verbose) puts("STUB: gss_krb5_ui called");
    return NULL;
}

void* gss_oid_to_str(void)
{
    if (verbose) puts("STUB: gss_oid_to_str called");
    return NULL;
}

void* gss_process_context_token(void)
{
    if (verbose) puts("STUB: gss_process_context_token called");
    return NULL;
}

void* gss_release_buffer(void)
{
    if (verbose) puts("STUB: gss_release_buffer called");
    return NULL;
}

void* gss_release_cred(void)
{
    if (verbose) puts("STUB: gss_release_cred called");
    return NULL;
}

void* gss_release_name(void)
{
    if (verbose) puts("STUB: gss_release_name called");
    return NULL;
}

void* gss_release_oid(void)
{
    if (verbose) puts("STUB: gss_release_oid called");
    return NULL;
}

void* gss_release_oid_set(void)
{
    if (verbose) puts("STUB: gss_release_oid_set called");
    return NULL;
}

void* gss_seal(void)
{
    if (verbose) puts("STUB: gss_seal called");
    return NULL;
}

void* gss_sign(void)
{
    if (verbose) puts("STUB: gss_sign called");
    return NULL;
}

void* gss_str_to_oid(void)
{
    if (verbose) puts("STUB: gss_str_to_oid called");
    return NULL;
}

void* gss_test_oid_set_member(void)
{
    if (verbose) puts("STUB: gss_test_oid_set_member called");
    return NULL;
}

void* gss_unseal(void)
{
    if (verbose) puts("STUB: gss_unseal called");
    return NULL;
}

void* gss_unwrap(void)
{
    if (verbose) puts("STUB: gss_unwrap called");
    return NULL;
}

void* gss_verify(void)
{
    if (verbose) puts("STUB: gss_verify called");
    return NULL;
}

void* gss_verify_mic(void)
{
    if (verbose) puts("STUB: gss_verify_mic called");
    return NULL;
}

void* gss_wrap(void)
{
    if (verbose) puts("STUB: gss_wrap called");
    return NULL;
}

void* gss_wrap_size_limit(void)
{
    if (verbose) puts("STUB: gss_wrap_size_limit called");
    return NULL;
}

void* initialize_prof_error_table(void)
{
    if (verbose) puts("STUB: initialize_prof_error_table called");
    return NULL;
}

void* k_isinst(void)
{
    if (verbose) puts("STUB: k_isinst called");
    return NULL;
}

void* k_isname(void)
{
    if (verbose) puts("STUB: k_isname called");
    return NULL;
}

void* k_isrealm(void)
{
    if (verbose) puts("STUB: k_isrealm called");
    return NULL;
}

void* kim_ccache_compare(void)
{
    if (verbose) puts("STUB: kim_ccache_compare called");
    return NULL;
}

void* kim_ccache_copy(void)
{
    if (verbose) puts("STUB: kim_ccache_copy called");
    return NULL;
}

void* kim_ccache_create_from_client_identity(void)
{
    if (verbose) puts("STUB: kim_ccache_create_from_client_identity called");
    return NULL;
}

void* kim_ccache_create_from_default(void)
{
    if (verbose) puts("STUB: kim_ccache_create_from_default called");
    return NULL;
}

void* kim_ccache_create_from_display_name(void)
{
    if (verbose) puts("STUB: kim_ccache_create_from_display_name called");
    return NULL;
}

void* kim_ccache_create_from_keytab(void)
{
    if (verbose) puts("STUB: kim_ccache_create_from_keytab called");
    return NULL;
}

void* kim_ccache_create_from_krb5_ccache(void)
{
    if (verbose) puts("STUB: kim_ccache_create_from_krb5_ccache called");
    return NULL;
}

void* kim_ccache_create_from_type_and_name(void)
{
    if (verbose) puts("STUB: kim_ccache_create_from_type_and_name called");
    return NULL;
}

void* kim_ccache_create_new(void)
{
    if (verbose) puts("STUB: kim_ccache_create_new called");
    return NULL;
}

void* kim_ccache_create_new_if_needed(void)
{
    if (verbose) puts("STUB: kim_ccache_create_new_if_needed called");
    return NULL;
}

void* kim_ccache_create_new_if_needed_with_password(void)
{
    if (verbose) puts("STUB: kim_ccache_create_new_if_needed_with_password called");
    return NULL;
}

void* kim_ccache_create_new_with_password(void)
{
    if (verbose) puts("STUB: kim_ccache_create_new_with_password called");
    return NULL;
}

void* kim_ccache_destroy(void)
{
    if (verbose) puts("STUB: kim_ccache_destroy called");
    return NULL;
}

void* kim_ccache_free(void)
{
    if (verbose) puts("STUB: kim_ccache_free called");
    return NULL;
}

void* kim_ccache_get_client_identity(void)
{
    if (verbose) puts("STUB: kim_ccache_get_client_identity called");
    return NULL;
}

void* kim_ccache_get_display_name(void)
{
    if (verbose) puts("STUB: kim_ccache_get_display_name called");
    return NULL;
}

void* kim_ccache_get_expiration_time(void)
{
    if (verbose) puts("STUB: kim_ccache_get_expiration_time called");
    return NULL;
}

void* kim_ccache_get_krb5_ccache(void)
{
    if (verbose) puts("STUB: kim_ccache_get_krb5_ccache called");
    return NULL;
}

void* kim_ccache_get_name(void)
{
    if (verbose) puts("STUB: kim_ccache_get_name called");
    return NULL;
}

void* kim_ccache_get_options(void)
{
    if (verbose) puts("STUB: kim_ccache_get_options called");
    return NULL;
}

void* kim_ccache_get_renewal_expiration_time(void)
{
    if (verbose) puts("STUB: kim_ccache_get_renewal_expiration_time called");
    return NULL;
}

void* kim_ccache_get_start_time(void)
{
    if (verbose) puts("STUB: kim_ccache_get_start_time called");
    return NULL;
}

void* kim_ccache_get_state(void)
{
    if (verbose) puts("STUB: kim_ccache_get_state called");
    return NULL;
}

void* kim_ccache_get_type(void)
{
    if (verbose) puts("STUB: kim_ccache_get_type called");
    return NULL;
}

void* kim_ccache_get_valid_credential(void)
{
    if (verbose) puts("STUB: kim_ccache_get_valid_credential called");
    return NULL;
}

void* kim_ccache_iterator_create(void)
{
    if (verbose) puts("STUB: kim_ccache_iterator_create called");
    return NULL;
}

void* kim_ccache_iterator_free(void)
{
    if (verbose) puts("STUB: kim_ccache_iterator_free called");
    return NULL;
}

void* kim_ccache_iterator_next(void)
{
    if (verbose) puts("STUB: kim_ccache_iterator_next called");
    return NULL;
}

void* kim_ccache_renew(void)
{
    if (verbose) puts("STUB: kim_ccache_renew called");
    return NULL;
}

void* kim_ccache_set_default(void)
{
    if (verbose) puts("STUB: kim_ccache_set_default called");
    return NULL;
}

void* kim_ccache_validate(void)
{
    if (verbose) puts("STUB: kim_ccache_validate called");
    return NULL;
}

void* kim_ccache_verify(void)
{
    if (verbose) puts("STUB: kim_ccache_verify called");
    return NULL;
}

void* kim_credential_copy(void)
{
    if (verbose) puts("STUB: kim_credential_copy called");
    return NULL;
}

void* kim_credential_create_from_keytab(void)
{
    if (verbose) puts("STUB: kim_credential_create_from_keytab called");
    return NULL;
}

void* kim_credential_create_from_krb5_creds(void)
{
    if (verbose) puts("STUB: kim_credential_create_from_krb5_creds called");
    return NULL;
}

void* kim_credential_create_new(void)
{
    if (verbose) puts("STUB: kim_credential_create_new called");
    return NULL;
}

void* kim_credential_create_new_with_password(void)
{
    if (verbose) puts("STUB: kim_credential_create_new_with_password called");
    return NULL;
}

void* kim_credential_free(void)
{
    if (verbose) puts("STUB: kim_credential_free called");
    return NULL;
}

void* kim_credential_get_client_identity(void)
{
    if (verbose) puts("STUB: kim_credential_get_client_identity called");
    return NULL;
}

void* kim_credential_get_expiration_time(void)
{
    if (verbose) puts("STUB: kim_credential_get_expiration_time called");
    return NULL;
}

void* kim_credential_get_krb5_creds(void)
{
    if (verbose) puts("STUB: kim_credential_get_krb5_creds called");
    return NULL;
}

void* kim_credential_get_options(void)
{
    if (verbose) puts("STUB: kim_credential_get_options called");
    return NULL;
}

void* kim_credential_get_renewal_expiration_time(void)
{
    if (verbose) puts("STUB: kim_credential_get_renewal_expiration_time called");
    return NULL;
}

void* kim_credential_get_service_identity(void)
{
    if (verbose) puts("STUB: kim_credential_get_service_identity called");
    return NULL;
}

void* kim_credential_get_start_time(void)
{
    if (verbose) puts("STUB: kim_credential_get_start_time called");
    return NULL;
}

void* kim_credential_get_state(void)
{
    if (verbose) puts("STUB: kim_credential_get_state called");
    return NULL;
}

void* kim_credential_is_tgt(void)
{
    if (verbose) puts("STUB: kim_credential_is_tgt called");
    return NULL;
}

void* kim_credential_iterator_create(void)
{
    if (verbose) puts("STUB: kim_credential_iterator_create called");
    return NULL;
}

void* kim_credential_iterator_free(void)
{
    if (verbose) puts("STUB: kim_credential_iterator_free called");
    return NULL;
}

void* kim_credential_iterator_next(void)
{
    if (verbose) puts("STUB: kim_credential_iterator_next called");
    return NULL;
}

void* kim_credential_renew(void)
{
    if (verbose) puts("STUB: kim_credential_renew called");
    return NULL;
}

void* kim_credential_store(void)
{
    if (verbose) puts("STUB: kim_credential_store called");
    return NULL;
}

void* kim_credential_validate(void)
{
    if (verbose) puts("STUB: kim_credential_validate called");
    return NULL;
}

void* kim_credential_verify(void)
{
    if (verbose) puts("STUB: kim_credential_verify called");
    return NULL;
}

void* kim_identity_change_password(void)
{
    if (verbose) puts("STUB: kim_identity_change_password called");
    return NULL;
}

void* kim_identity_compare(void)
{
    if (verbose) puts("STUB: kim_identity_compare called");
    return NULL;
}

void* kim_identity_copy(void)
{
    if (verbose) puts("STUB: kim_identity_copy called");
    return NULL;
}

void* kim_identity_create_from_components(void)
{
    if (verbose) puts("STUB: kim_identity_create_from_components called");
    return NULL;
}

void* kim_identity_create_from_krb5_principal(void)
{
    if (verbose) puts("STUB: kim_identity_create_from_krb5_principal called");
    return NULL;
}

void* kim_identity_create_from_string(void)
{
    if (verbose) puts("STUB: kim_identity_create_from_string called");
    return NULL;
}

void* kim_identity_free(void)
{
    if (verbose) puts("STUB: kim_identity_free called");
    return NULL;
}

void* kim_identity_get_component_at_index(void)
{
    if (verbose) puts("STUB: kim_identity_get_component_at_index called");
    return NULL;
}

void* kim_identity_get_components_string(void)
{
    if (verbose) puts("STUB: kim_identity_get_components_string called");
    return NULL;
}

void* kim_identity_get_display_string(void)
{
    if (verbose) puts("STUB: kim_identity_get_display_string called");
    return NULL;
}

void* kim_identity_get_krb5_principal(void)
{
    if (verbose) puts("STUB: kim_identity_get_krb5_principal called");
    return NULL;
}

void* kim_identity_get_number_of_components(void)
{
    if (verbose) puts("STUB: kim_identity_get_number_of_components called");
    return NULL;
}

void* kim_identity_get_realm(void)
{
    if (verbose) puts("STUB: kim_identity_get_realm called");
    return NULL;
}

void* kim_identity_get_string(void)
{
    if (verbose) puts("STUB: kim_identity_get_string called");
    return NULL;
}

void* kim_library_set_allow_automatic_prompting(void)
{
    if (verbose) puts("STUB: kim_library_set_allow_automatic_prompting called");
    return NULL;
}

void* kim_library_set_allow_home_directory_access(void)
{
    if (verbose) puts("STUB: kim_library_set_allow_home_directory_access called");
    return NULL;
}

void* kim_library_set_application_name(void)
{
    if (verbose) puts("STUB: kim_library_set_application_name called");
    return NULL;
}

void* kim_library_set_ui_environment(void)
{
    if (verbose) puts("STUB: kim_library_set_ui_environment called");
    return NULL;
}

void* kim_options_copy(void)
{
    if (verbose) puts("STUB: kim_options_copy called");
    return NULL;
}

void* kim_options_create(void)
{
    if (verbose) puts("STUB: kim_options_create called");
    return NULL;
}

void* kim_options_create_from_stream(void)
{
    if (verbose) puts("STUB: kim_options_create_from_stream called");
    return NULL;
}

void* kim_options_free(void)
{
    if (verbose) puts("STUB: kim_options_free called");
    return NULL;
}

void* kim_options_get_addressless(void)
{
    if (verbose) puts("STUB: kim_options_get_addressless called");
    return NULL;
}

void* kim_options_get_forwardable(void)
{
    if (verbose) puts("STUB: kim_options_get_forwardable called");
    return NULL;
}

void* kim_options_get_lifetime(void)
{
    if (verbose) puts("STUB: kim_options_get_lifetime called");
    return NULL;
}

void* kim_options_get_proxiable(void)
{
    if (verbose) puts("STUB: kim_options_get_proxiable called");
    return NULL;
}

void* kim_options_get_renewable(void)
{
    if (verbose) puts("STUB: kim_options_get_renewable called");
    return NULL;
}

void* kim_options_get_renewal_lifetime(void)
{
    if (verbose) puts("STUB: kim_options_get_renewal_lifetime called");
    return NULL;
}

void* kim_options_get_service_name(void)
{
    if (verbose) puts("STUB: kim_options_get_service_name called");
    return NULL;
}

void* kim_options_get_start_time(void)
{
    if (verbose) puts("STUB: kim_options_get_start_time called");
    return NULL;
}

void* kim_options_set_addressless(void)
{
    if (verbose) puts("STUB: kim_options_set_addressless called");
    return NULL;
}

void* kim_options_set_forwardable(void)
{
    if (verbose) puts("STUB: kim_options_set_forwardable called");
    return NULL;
}

void* kim_options_set_lifetime(void)
{
    if (verbose) puts("STUB: kim_options_set_lifetime called");
    return NULL;
}

void* kim_options_set_proxiable(void)
{
    if (verbose) puts("STUB: kim_options_set_proxiable called");
    return NULL;
}

void* kim_options_set_renewable(void)
{
    if (verbose) puts("STUB: kim_options_set_renewable called");
    return NULL;
}

void* kim_options_set_renewal_lifetime(void)
{
    if (verbose) puts("STUB: kim_options_set_renewal_lifetime called");
    return NULL;
}

void* kim_options_set_service_name(void)
{
    if (verbose) puts("STUB: kim_options_set_service_name called");
    return NULL;
}

void* kim_options_set_start_time(void)
{
    if (verbose) puts("STUB: kim_options_set_start_time called");
    return NULL;
}

void* kim_options_write_to_stream(void)
{
    if (verbose) puts("STUB: kim_options_write_to_stream called");
    return NULL;
}

void* kim_preferences_add_favorite_identity(void)
{
    if (verbose) puts("STUB: kim_preferences_add_favorite_identity called");
    return NULL;
}

void* kim_preferences_copy(void)
{
    if (verbose) puts("STUB: kim_preferences_copy called");
    return NULL;
}

void* kim_preferences_create(void)
{
    if (verbose) puts("STUB: kim_preferences_create called");
    return NULL;
}

void* kim_preferences_free(void)
{
    if (verbose) puts("STUB: kim_preferences_free called");
    return NULL;
}

void* kim_preferences_get_client_identity(void)
{
    if (verbose) puts("STUB: kim_preferences_get_client_identity called");
    return NULL;
}

void* kim_preferences_get_favorite_identity_at_index(void)
{
    if (verbose) puts("STUB: kim_preferences_get_favorite_identity_at_index called");
    return NULL;
}

void* kim_preferences_get_maximum_lifetime(void)
{
    if (verbose) puts("STUB: kim_preferences_get_maximum_lifetime called");
    return NULL;
}

void* kim_preferences_get_maximum_renewal_lifetime(void)
{
    if (verbose) puts("STUB: kim_preferences_get_maximum_renewal_lifetime called");
    return NULL;
}

void* kim_preferences_get_minimum_lifetime(void)
{
    if (verbose) puts("STUB: kim_preferences_get_minimum_lifetime called");
    return NULL;
}

void* kim_preferences_get_minimum_renewal_lifetime(void)
{
    if (verbose) puts("STUB: kim_preferences_get_minimum_renewal_lifetime called");
    return NULL;
}

void* kim_preferences_get_number_of_favorite_identities(void)
{
    if (verbose) puts("STUB: kim_preferences_get_number_of_favorite_identities called");
    return NULL;
}

void* kim_preferences_get_options(void)
{
    if (verbose) puts("STUB: kim_preferences_get_options called");
    return NULL;
}

void* kim_preferences_get_remember_client_identity(void)
{
    if (verbose) puts("STUB: kim_preferences_get_remember_client_identity called");
    return NULL;
}

void* kim_preferences_get_remember_options(void)
{
    if (verbose) puts("STUB: kim_preferences_get_remember_options called");
    return NULL;
}

void* kim_preferences_remove_all_favorite_identities(void)
{
    if (verbose) puts("STUB: kim_preferences_remove_all_favorite_identities called");
    return NULL;
}

void* kim_preferences_remove_favorite_identity(void)
{
    if (verbose) puts("STUB: kim_preferences_remove_favorite_identity called");
    return NULL;
}

void* kim_preferences_set_client_identity(void)
{
    if (verbose) puts("STUB: kim_preferences_set_client_identity called");
    return NULL;
}

void* kim_preferences_set_maximum_lifetime(void)
{
    if (verbose) puts("STUB: kim_preferences_set_maximum_lifetime called");
    return NULL;
}

void* kim_preferences_set_maximum_renewal_lifetime(void)
{
    if (verbose) puts("STUB: kim_preferences_set_maximum_renewal_lifetime called");
    return NULL;
}

void* kim_preferences_set_minimum_lifetime(void)
{
    if (verbose) puts("STUB: kim_preferences_set_minimum_lifetime called");
    return NULL;
}

void* kim_preferences_set_minimum_renewal_lifetime(void)
{
    if (verbose) puts("STUB: kim_preferences_set_minimum_renewal_lifetime called");
    return NULL;
}

void* kim_preferences_set_options(void)
{
    if (verbose) puts("STUB: kim_preferences_set_options called");
    return NULL;
}

void* kim_preferences_set_remember_client_identity(void)
{
    if (verbose) puts("STUB: kim_preferences_set_remember_client_identity called");
    return NULL;
}

void* kim_preferences_set_remember_options(void)
{
    if (verbose) puts("STUB: kim_preferences_set_remember_options called");
    return NULL;
}

void* kim_preferences_synchronize(void)
{
    if (verbose) puts("STUB: kim_preferences_synchronize called");
    return NULL;
}

void* kim_selection_hints_copy(void)
{
    if (verbose) puts("STUB: kim_selection_hints_copy called");
    return NULL;
}

void* kim_selection_hints_create(void)
{
    if (verbose) puts("STUB: kim_selection_hints_create called");
    return NULL;
}

void* kim_selection_hints_create_from_stream(void)
{
    if (verbose) puts("STUB: kim_selection_hints_create_from_stream called");
    return NULL;
}

void* kim_selection_hints_forget_identity(void)
{
    if (verbose) puts("STUB: kim_selection_hints_forget_identity called");
    return NULL;
}

void* kim_selection_hints_free(void)
{
    if (verbose) puts("STUB: kim_selection_hints_free called");
    return NULL;
}

void* kim_selection_hints_get_allow_user_interaction(void)
{
    if (verbose) puts("STUB: kim_selection_hints_get_allow_user_interaction called");
    return NULL;
}

void* kim_selection_hints_get_explanation(void)
{
    if (verbose) puts("STUB: kim_selection_hints_get_explanation called");
    return NULL;
}

void* kim_selection_hints_get_hint(void)
{
    if (verbose) puts("STUB: kim_selection_hints_get_hint called");
    return NULL;
}

void* kim_selection_hints_get_identity(void)
{
    if (verbose) puts("STUB: kim_selection_hints_get_identity called");
    return NULL;
}

void* kim_selection_hints_get_options(void)
{
    if (verbose) puts("STUB: kim_selection_hints_get_options called");
    return NULL;
}

void* kim_selection_hints_get_remember_identity(void)
{
    if (verbose) puts("STUB: kim_selection_hints_get_remember_identity called");
    return NULL;
}

void* kim_selection_hints_remember_identity(void)
{
    if (verbose) puts("STUB: kim_selection_hints_remember_identity called");
    return NULL;
}

void* kim_selection_hints_set_allow_user_interaction(void)
{
    if (verbose) puts("STUB: kim_selection_hints_set_allow_user_interaction called");
    return NULL;
}

void* kim_selection_hints_set_explanation(void)
{
    if (verbose) puts("STUB: kim_selection_hints_set_explanation called");
    return NULL;
}

void* kim_selection_hints_set_hint(void)
{
    if (verbose) puts("STUB: kim_selection_hints_set_hint called");
    return NULL;
}

void* kim_selection_hints_set_options(void)
{
    if (verbose) puts("STUB: kim_selection_hints_set_options called");
    return NULL;
}

void* kim_selection_hints_set_remember_identity(void)
{
    if (verbose) puts("STUB: kim_selection_hints_set_remember_identity called");
    return NULL;
}

void* kim_string_compare(void)
{
    if (verbose) puts("STUB: kim_string_compare called");
    return NULL;
}

void* kim_string_copy(void)
{
    if (verbose) puts("STUB: kim_string_copy called");
    return NULL;
}

void* kim_string_create_for_last_error(void)
{
    if (verbose) puts("STUB: kim_string_create_for_last_error called");
    return NULL;
}

void* kim_string_free(void)
{
    if (verbose) puts("STUB: kim_string_free called");
    return NULL;
}

void* kname_parse(void)
{
    if (verbose) puts("STUB: kname_parse called");
    return NULL;
}

void* kname_unparse(void)
{
    if (verbose) puts("STUB: kname_unparse called");
    return NULL;
}

void* krb524_convert_creds_kdc(void)
{
    if (verbose) puts("STUB: krb524_convert_creds_kdc called");
    return NULL;
}

void* krb5_425_conv_principal(void)
{
    if (verbose) puts("STUB: krb5_425_conv_principal called");
    return NULL;
}

void* krb5_524_conv_principal(void)
{
    if (verbose) puts("STUB: krb5_524_conv_principal called");
    return NULL;
}

void* krb5_524_convert_creds(void)
{
    if (verbose) puts("STUB: krb5_524_convert_creds called");
    return NULL;
}

void* krb5_address_compare(void)
{
    if (verbose) puts("STUB: krb5_address_compare called");
    return NULL;
}

void* krb5_address_order(void)
{
    if (verbose) puts("STUB: krb5_address_order called");
    return NULL;
}

void* krb5_address_search(void)
{
    if (verbose) puts("STUB: krb5_address_search called");
    return NULL;
}

void* krb5_aname_to_localname(void)
{
    if (verbose) puts("STUB: krb5_aname_to_localname called");
    return NULL;
}

void* krb5_appdefault_boolean(void)
{
    if (verbose) puts("STUB: krb5_appdefault_boolean called");
    return NULL;
}

void* krb5_appdefault_string(void)
{
    if (verbose) puts("STUB: krb5_appdefault_string called");
    return NULL;
}

void* krb5_auth_con_free(void)
{
    if (verbose) puts("STUB: krb5_auth_con_free called");
    return NULL;
}

void* krb5_auth_con_genaddrs(void)
{
    if (verbose) puts("STUB: krb5_auth_con_genaddrs called");
    return NULL;
}

void* krb5_auth_con_get_checksum_func(void)
{
    if (verbose) puts("STUB: krb5_auth_con_get_checksum_func called");
    return NULL;
}

void* krb5_auth_con_getaddrs(void)
{
    if (verbose) puts("STUB: krb5_auth_con_getaddrs called");
    return NULL;
}

void* krb5_auth_con_getauthenticator(void)
{
    if (verbose) puts("STUB: krb5_auth_con_getauthenticator called");
    return NULL;
}

void* krb5_auth_con_getflags(void)
{
    if (verbose) puts("STUB: krb5_auth_con_getflags called");
    return NULL;
}

void* krb5_auth_con_getkey(void)
{
    if (verbose) puts("STUB: krb5_auth_con_getkey called");
    return NULL;
}

void* krb5_auth_con_getlocalseqnumber(void)
{
    if (verbose) puts("STUB: krb5_auth_con_getlocalseqnumber called");
    return NULL;
}

void* krb5_auth_con_getlocalsubkey(void)
{
    if (verbose) puts("STUB: krb5_auth_con_getlocalsubkey called");
    return NULL;
}

void* krb5_auth_con_getrcache(void)
{
    if (verbose) puts("STUB: krb5_auth_con_getrcache called");
    return NULL;
}

void* krb5_auth_con_getrecvsubkey(void)
{
    if (verbose) puts("STUB: krb5_auth_con_getrecvsubkey called");
    return NULL;
}

void* krb5_auth_con_getremoteseqnumber(void)
{
    if (verbose) puts("STUB: krb5_auth_con_getremoteseqnumber called");
    return NULL;
}

void* krb5_auth_con_getremotesubkey(void)
{
    if (verbose) puts("STUB: krb5_auth_con_getremotesubkey called");
    return NULL;
}

void* krb5_auth_con_getsendsubkey(void)
{
    if (verbose) puts("STUB: krb5_auth_con_getsendsubkey called");
    return NULL;
}

void* krb5_auth_con_init(void)
{
    if (verbose) puts("STUB: krb5_auth_con_init called");
    return NULL;
}

void* krb5_auth_con_initivector(void)
{
    if (verbose) puts("STUB: krb5_auth_con_initivector called");
    return NULL;
}

void* krb5_auth_con_set_checksum_func(void)
{
    if (verbose) puts("STUB: krb5_auth_con_set_checksum_func called");
    return NULL;
}

void* krb5_auth_con_setaddrs(void)
{
    if (verbose) puts("STUB: krb5_auth_con_setaddrs called");
    return NULL;
}

void* krb5_auth_con_setflags(void)
{
    if (verbose) puts("STUB: krb5_auth_con_setflags called");
    return NULL;
}

void* krb5_auth_con_setports(void)
{
    if (verbose) puts("STUB: krb5_auth_con_setports called");
    return NULL;
}

void* krb5_auth_con_setrcache(void)
{
    if (verbose) puts("STUB: krb5_auth_con_setrcache called");
    return NULL;
}

void* krb5_auth_con_setrecvsubkey(void)
{
    if (verbose) puts("STUB: krb5_auth_con_setrecvsubkey called");
    return NULL;
}

void* krb5_auth_con_setsendsubkey(void)
{
    if (verbose) puts("STUB: krb5_auth_con_setsendsubkey called");
    return NULL;
}

void* krb5_auth_con_setuseruserkey(void)
{
    if (verbose) puts("STUB: krb5_auth_con_setuseruserkey called");
    return NULL;
}

void* krb5_build_principal(void)
{
    if (verbose) puts("STUB: krb5_build_principal called");
    return NULL;
}

void* krb5_build_principal_alloc_va(void)
{
    if (verbose) puts("STUB: krb5_build_principal_alloc_va called");
    return NULL;
}

void* krb5_build_principal_ext(void)
{
    if (verbose) puts("STUB: krb5_build_principal_ext called");
    return NULL;
}

void* krb5_build_principal_va(void)
{
    if (verbose) puts("STUB: krb5_build_principal_va called");
    return NULL;
}

void* krb5_c_block_size(void)
{
    if (verbose) puts("STUB: krb5_c_block_size called");
    return NULL;
}

void* krb5_c_checksum_length(void)
{
    if (verbose) puts("STUB: krb5_c_checksum_length called");
    return NULL;
}

void* krb5_c_decrypt(void)
{
    if (verbose) puts("STUB: krb5_c_decrypt called");
    return NULL;
}

void* krb5_c_encrypt(void)
{
    if (verbose) puts("STUB: krb5_c_encrypt called");
    return NULL;
}

void* krb5_c_encrypt_length(void)
{
    if (verbose) puts("STUB: krb5_c_encrypt_length called");
    return NULL;
}

void* krb5_c_enctype_compare(void)
{
    if (verbose) puts("STUB: krb5_c_enctype_compare called");
    return NULL;
}

void* krb5_c_free_state(void)
{
    if (verbose) puts("STUB: krb5_c_free_state called");
    return NULL;
}

void* krb5_c_init_state(void)
{
    if (verbose) puts("STUB: krb5_c_init_state called");
    return NULL;
}

void* krb5_c_is_coll_proof_cksum(void)
{
    if (verbose) puts("STUB: krb5_c_is_coll_proof_cksum called");
    return NULL;
}

void* krb5_c_is_keyed_cksum(void)
{
    if (verbose) puts("STUB: krb5_c_is_keyed_cksum called");
    return NULL;
}

void* krb5_c_keyed_checksum_types(void)
{
    if (verbose) puts("STUB: krb5_c_keyed_checksum_types called");
    return NULL;
}

void* krb5_c_make_checksum(void)
{
    if (verbose) puts("STUB: krb5_c_make_checksum called");
    return NULL;
}

void* krb5_c_make_random_key(void)
{
    if (verbose) puts("STUB: krb5_c_make_random_key called");
    return NULL;
}

void* krb5_c_random_add_entropy(void)
{
    if (verbose) puts("STUB: krb5_c_random_add_entropy called");
    return NULL;
}

void* krb5_c_random_make_octets(void)
{
    if (verbose) puts("STUB: krb5_c_random_make_octets called");
    return NULL;
}

void* krb5_c_random_os_entropy(void)
{
    if (verbose) puts("STUB: krb5_c_random_os_entropy called");
    return NULL;
}

void* krb5_c_random_seed(void)
{
    if (verbose) puts("STUB: krb5_c_random_seed called");
    return NULL;
}

void* krb5_c_string_to_key(void)
{
    if (verbose) puts("STUB: krb5_c_string_to_key called");
    return NULL;
}

void* krb5_c_string_to_key_with_params(void)
{
    if (verbose) puts("STUB: krb5_c_string_to_key_with_params called");
    return NULL;
}

void* krb5_c_valid_cksumtype(void)
{
    if (verbose) puts("STUB: krb5_c_valid_cksumtype called");
    return NULL;
}

void* krb5_c_valid_enctype(void)
{
    if (verbose) puts("STUB: krb5_c_valid_enctype called");
    return NULL;
}

void* krb5_c_verify_checksum(void)
{
    if (verbose) puts("STUB: krb5_c_verify_checksum called");
    return NULL;
}

void* krb5_calculate_checksum(void)
{
    if (verbose) puts("STUB: krb5_calculate_checksum called");
    return NULL;
}

void* krb5_cc_cache_match(void)
{
    if (verbose) puts("STUB: krb5_cc_cache_match called");
    return NULL;
}

void* krb5_cc_close(void)
{
    if (verbose) puts("STUB: krb5_cc_close called");
    return NULL;
}

void* krb5_cc_copy_creds(void)
{
    if (verbose) puts("STUB: krb5_cc_copy_creds called");
    return NULL;
}

void* krb5_cc_default(void)
{
    if (verbose) puts("STUB: krb5_cc_default called");
    return NULL;
}

void* krb5_cc_default_name(void)
{
    if (verbose) puts("STUB: krb5_cc_default_name called");
    return NULL;
}

void* krb5_cc_destroy(void)
{
    if (verbose) puts("STUB: krb5_cc_destroy called");
    return NULL;
}

void* krb5_cc_end_seq_get(void)
{
    if (verbose) puts("STUB: krb5_cc_end_seq_get called");
    return NULL;
}

void* krb5_cc_gen_new(void)
{
    if (verbose) puts("STUB: krb5_cc_gen_new called");
    return NULL;
}

void* krb5_cc_get_config(void)
{
    if (verbose) puts("STUB: krb5_cc_get_config called");
    return NULL;
}

void* krb5_cc_get_name(void)
{
    if (verbose) puts("STUB: krb5_cc_get_name called");
    return NULL;
}

void* krb5_cc_get_principal(void)
{
    if (verbose) puts("STUB: krb5_cc_get_principal called");
    return NULL;
}

void* krb5_cc_get_type(void)
{
    if (verbose) puts("STUB: krb5_cc_get_type called");
    return NULL;
}

void* krb5_cc_initialize(void)
{
    if (verbose) puts("STUB: krb5_cc_initialize called");
    return NULL;
}

void* krb5_cc_last_change_time(void)
{
    if (verbose) puts("STUB: krb5_cc_last_change_time called");
    return NULL;
}

void* krb5_cc_lock(void)
{
    if (verbose) puts("STUB: krb5_cc_lock called");
    return NULL;
}

void* krb5_cc_move(void)
{
    if (verbose) puts("STUB: krb5_cc_move called");
    return NULL;
}

void* krb5_cc_new_unique(void)
{
    if (verbose) puts("STUB: krb5_cc_new_unique called");
    return NULL;
}

void* krb5_cc_next_cred(void)
{
    if (verbose) puts("STUB: krb5_cc_next_cred called");
    return NULL;
}

void* krb5_cc_remove_cred(void)
{
    if (verbose) puts("STUB: krb5_cc_remove_cred called");
    return NULL;
}

void* krb5_cc_resolve(void)
{
    if (verbose) puts("STUB: krb5_cc_resolve called");
    return NULL;
}

void* krb5_cc_retrieve_cred(void)
{
    if (verbose) puts("STUB: krb5_cc_retrieve_cred called");
    return NULL;
}

void* krb5_cc_set_config(void)
{
    if (verbose) puts("STUB: krb5_cc_set_config called");
    return NULL;
}

void* krb5_cc_set_default_name(void)
{
    if (verbose) puts("STUB: krb5_cc_set_default_name called");
    return NULL;
}

void* krb5_cc_set_flags(void)
{
    if (verbose) puts("STUB: krb5_cc_set_flags called");
    return NULL;
}

void* krb5_cc_start_seq_get(void)
{
    if (verbose) puts("STUB: krb5_cc_start_seq_get called");
    return NULL;
}

void* krb5_cc_store_cred(void)
{
    if (verbose) puts("STUB: krb5_cc_store_cred called");
    return NULL;
}

void* krb5_cc_unlock(void)
{
    if (verbose) puts("STUB: krb5_cc_unlock called");
    return NULL;
}

void* krb5_cccol_cursor_free(void)
{
    if (verbose) puts("STUB: krb5_cccol_cursor_free called");
    return NULL;
}

void* krb5_cccol_cursor_new(void)
{
    if (verbose) puts("STUB: krb5_cccol_cursor_new called");
    return NULL;
}

void* krb5_cccol_cursor_next(void)
{
    if (verbose) puts("STUB: krb5_cccol_cursor_next called");
    return NULL;
}

void* krb5_cccol_last_change_time(void)
{
    if (verbose) puts("STUB: krb5_cccol_last_change_time called");
    return NULL;
}

void* krb5_cccol_lock(void)
{
    if (verbose) puts("STUB: krb5_cccol_lock called");
    return NULL;
}

void* krb5_cccol_unlock(void)
{
    if (verbose) puts("STUB: krb5_cccol_unlock called");
    return NULL;
}

void* krb5_change_password(void)
{
    if (verbose) puts("STUB: krb5_change_password called");
    return NULL;
}

void* krb5_checksum_size(void)
{
    if (verbose) puts("STUB: krb5_checksum_size called");
    return NULL;
}

void* krb5_cksumtype_to_string(void)
{
    if (verbose) puts("STUB: krb5_cksumtype_to_string called");
    return NULL;
}

void* krb5_clear_error_message(void)
{
    if (verbose) puts("STUB: krb5_clear_error_message called");
    return NULL;
}

void* krb5_copy_addresses(void)
{
    if (verbose) puts("STUB: krb5_copy_addresses called");
    return NULL;
}

void* krb5_copy_authdata(void)
{
    if (verbose) puts("STUB: krb5_copy_authdata called");
    return NULL;
}

void* krb5_copy_authenticator(void)
{
    if (verbose) puts("STUB: krb5_copy_authenticator called");
    return NULL;
}

void* krb5_copy_checksum(void)
{
    if (verbose) puts("STUB: krb5_copy_checksum called");
    return NULL;
}

void* krb5_copy_context(void)
{
    if (verbose) puts("STUB: krb5_copy_context called");
    return NULL;
}

void* krb5_copy_creds(void)
{
    if (verbose) puts("STUB: krb5_copy_creds called");
    return NULL;
}

void* krb5_copy_data(void)
{
    if (verbose) puts("STUB: krb5_copy_data called");
    return NULL;
}

void* krb5_copy_keyblock(void)
{
    if (verbose) puts("STUB: krb5_copy_keyblock called");
    return NULL;
}

void* krb5_copy_keyblock_contents(void)
{
    if (verbose) puts("STUB: krb5_copy_keyblock_contents called");
    return NULL;
}

void* krb5_copy_principal(void)
{
    if (verbose) puts("STUB: krb5_copy_principal called");
    return NULL;
}

void* krb5_copy_ticket(void)
{
    if (verbose) puts("STUB: krb5_copy_ticket called");
    return NULL;
}

void* krb5_decode_ticket(void)
{
    if (verbose) puts("STUB: krb5_decode_ticket called");
    return NULL;
}

void* krb5_decrypt(void)
{
    if (verbose) puts("STUB: krb5_decrypt called");
    return NULL;
}

void* krb5_deltat_to_string(void)
{
    if (verbose) puts("STUB: krb5_deltat_to_string called");
    return NULL;
}

void* krb5_eblock_enctype(void)
{
    if (verbose) puts("STUB: krb5_eblock_enctype called");
    return NULL;
}

void* krb5_encrypt(void)
{
    if (verbose) puts("STUB: krb5_encrypt called");
    return NULL;
}

void* krb5_encrypt_size(void)
{
    if (verbose) puts("STUB: krb5_encrypt_size called");
    return NULL;
}

void* krb5_enctype_to_string(void)
{
    if (verbose) puts("STUB: krb5_enctype_to_string called");
    return NULL;
}

void* krb5_finish_key(void)
{
    if (verbose) puts("STUB: krb5_finish_key called");
    return NULL;
}

void* krb5_finish_random_key(void)
{
    if (verbose) puts("STUB: krb5_finish_random_key called");
    return NULL;
}

void* krb5_free_addresses(void)
{
    if (verbose) puts("STUB: krb5_free_addresses called");
    return NULL;
}

void* krb5_free_ap_rep_enc_part(void)
{
    if (verbose) puts("STUB: krb5_free_ap_rep_enc_part called");
    return NULL;
}

void* krb5_free_authdata(void)
{
    if (verbose) puts("STUB: krb5_free_authdata called");
    return NULL;
}

void* krb5_free_authenticator(void)
{
    if (verbose) puts("STUB: krb5_free_authenticator called");
    return NULL;
}

void* krb5_free_checksum(void)
{
    if (verbose) puts("STUB: krb5_free_checksum called");
    return NULL;
}

void* krb5_free_checksum_contents(void)
{
    if (verbose) puts("STUB: krb5_free_checksum_contents called");
    return NULL;
}

void* krb5_free_cksumtypes(void)
{
    if (verbose) puts("STUB: krb5_free_cksumtypes called");
    return NULL;
}

void* krb5_free_config_files(void)
{
    if (verbose) puts("STUB: krb5_free_config_files called");
    return NULL;
}

void* krb5_free_context(void)
{
    if (verbose) puts("STUB: krb5_free_context called");
    return NULL;
}

void* krb5_free_cred_contents(void)
{
    if (verbose) puts("STUB: krb5_free_cred_contents called");
    return NULL;
}

void* krb5_free_creds(void)
{
    if (verbose) puts("STUB: krb5_free_creds called");
    return NULL;
}

void* krb5_free_data(void)
{
    if (verbose) puts("STUB: krb5_free_data called");
    return NULL;
}

void* krb5_free_data_contents(void)
{
    if (verbose) puts("STUB: krb5_free_data_contents called");
    return NULL;
}

void* krb5_free_default_realm(void)
{
    if (verbose) puts("STUB: krb5_free_default_realm called");
    return NULL;
}

void* krb5_free_enc_tkt_part(void)
{
    if (verbose) puts("STUB: krb5_free_enc_tkt_part called");
    return NULL;
}

void* krb5_free_error(void)
{
    if (verbose) puts("STUB: krb5_free_error called");
    return NULL;
}

void* krb5_free_error_message(void)
{
    if (verbose) puts("STUB: krb5_free_error_message called");
    return NULL;
}

void* krb5_free_host_realm(void)
{
    if (verbose) puts("STUB: krb5_free_host_realm called");
    return NULL;
}

void* krb5_free_keyblock(void)
{
    if (verbose) puts("STUB: krb5_free_keyblock called");
    return NULL;
}

void* krb5_free_keyblock_contents(void)
{
    if (verbose) puts("STUB: krb5_free_keyblock_contents called");
    return NULL;
}

void* krb5_free_keytab_entry_contents(void)
{
    if (verbose) puts("STUB: krb5_free_keytab_entry_contents called");
    return NULL;
}

void* krb5_free_krbhst(void)
{
    if (verbose) puts("STUB: krb5_free_krbhst called");
    return NULL;
}

void* krb5_free_principal(void)
{
    if (verbose) puts("STUB: krb5_free_principal called");
    return NULL;
}

void* krb5_free_tgt_creds(void)
{
    if (verbose) puts("STUB: krb5_free_tgt_creds called");
    return NULL;
}

void* krb5_free_ticket(void)
{
    if (verbose) puts("STUB: krb5_free_ticket called");
    return NULL;
}

void* krb5_free_unparsed_name(void)
{
    if (verbose) puts("STUB: krb5_free_unparsed_name called");
    return NULL;
}

void* krb5_fwd_tgt_creds(void)
{
    if (verbose) puts("STUB: krb5_fwd_tgt_creds called");
    return NULL;
}

void* krb5_get_credentials(void)
{
    if (verbose) puts("STUB: krb5_get_credentials called");
    return NULL;
}

void* krb5_get_credentials_renew(void)
{
    if (verbose) puts("STUB: krb5_get_credentials_renew called");
    return NULL;
}

void* krb5_get_credentials_validate(void)
{
    if (verbose) puts("STUB: krb5_get_credentials_validate called");
    return NULL;
}

void* krb5_get_default_config_files(void)
{
    if (verbose) puts("STUB: krb5_get_default_config_files called");
    return NULL;
}

void* krb5_get_default_realm(void)
{
    if (verbose) puts("STUB: krb5_get_default_realm called");
    return NULL;
}

void* krb5_get_error_message(void)
{
    if (verbose) puts("STUB: krb5_get_error_message called");
    return NULL;
}

void* krb5_get_host_realm(void)
{
    if (verbose) puts("STUB: krb5_get_host_realm called");
    return NULL;
}

void* krb5_get_in_tkt(void)
{
    if (verbose) puts("STUB: krb5_get_in_tkt called");
    return NULL;
}

void* krb5_get_in_tkt_with_keytab(void)
{
    if (verbose) puts("STUB: krb5_get_in_tkt_with_keytab called");
    return NULL;
}

void* krb5_get_in_tkt_with_password(void)
{
    if (verbose) puts("STUB: krb5_get_in_tkt_with_password called");
    return NULL;
}

void* krb5_get_in_tkt_with_skey(void)
{
    if (verbose) puts("STUB: krb5_get_in_tkt_with_skey called");
    return NULL;
}

void* krb5_get_init_creds_keytab(void)
{
    if (verbose) puts("STUB: krb5_get_init_creds_keytab called");
    return NULL;
}

void* krb5_get_init_creds_opt_alloc(void)
{
    if (verbose) puts("STUB: krb5_get_init_creds_opt_alloc called");
    return NULL;
}

void* krb5_get_init_creds_opt_free(void)
{
    if (verbose) puts("STUB: krb5_get_init_creds_opt_free called");
    return NULL;
}

void* krb5_get_init_creds_opt_init(void)
{
    if (verbose) puts("STUB: krb5_get_init_creds_opt_init called");
    return NULL;
}

void* krb5_get_init_creds_opt_set_address_list(void)
{
    if (verbose) puts("STUB: krb5_get_init_creds_opt_set_address_list called");
    return NULL;
}

void* krb5_get_init_creds_opt_set_canonicalize(void)
{
    if (verbose) puts("STUB: krb5_get_init_creds_opt_set_canonicalize called");
    return NULL;
}

void* krb5_get_init_creds_opt_set_change_password_prompt(void)
{
    if (verbose) puts("STUB: krb5_get_init_creds_opt_set_change_password_prompt called");
    return NULL;
}

void* krb5_get_init_creds_opt_set_etype_list(void)
{
    if (verbose) puts("STUB: krb5_get_init_creds_opt_set_etype_list called");
    return NULL;
}

void* krb5_get_init_creds_opt_set_forwardable(void)
{
    if (verbose) puts("STUB: krb5_get_init_creds_opt_set_forwardable called");
    return NULL;
}

void* krb5_get_init_creds_opt_set_pa(void)
{
    if (verbose) puts("STUB: krb5_get_init_creds_opt_set_pa called");
    return NULL;
}

void* krb5_get_init_creds_opt_set_preauth_list(void)
{
    if (verbose) puts("STUB: krb5_get_init_creds_opt_set_preauth_list called");
    return NULL;
}

void* krb5_get_init_creds_opt_set_process_last_req(void)
{
    if (verbose) puts("STUB: krb5_get_init_creds_opt_set_process_last_req called");
    return NULL;
}

void* krb5_get_init_creds_opt_set_proxiable(void)
{
    if (verbose) puts("STUB: krb5_get_init_creds_opt_set_proxiable called");
    return NULL;
}

void* krb5_get_init_creds_opt_set_renew_life(void)
{
    if (verbose) puts("STUB: krb5_get_init_creds_opt_set_renew_life called");
    return NULL;
}

void* krb5_get_init_creds_opt_set_tkt_life(void)
{
    if (verbose) puts("STUB: krb5_get_init_creds_opt_set_tkt_life called");
    return NULL;
}

void* krb5_get_init_creds_password(void)
{
    if (verbose) puts("STUB: krb5_get_init_creds_password called");
    return NULL;
}

void* krb5_get_krbhst(void)
{
    if (verbose) puts("STUB: krb5_get_krbhst called");
    return NULL;
}

void* krb5_get_permitted_enctypes(void)
{
    if (verbose) puts("STUB: krb5_get_permitted_enctypes called");
    return NULL;
}

void* krb5_get_profile(void)
{
    if (verbose) puts("STUB: krb5_get_profile called");
    return NULL;
}

void* krb5_get_prompt_types(void)
{
    if (verbose) puts("STUB: krb5_get_prompt_types called");
    return NULL;
}

void* krb5_get_realm_domain(void)
{
    if (verbose) puts("STUB: krb5_get_realm_domain called");
    return NULL;
}

void* krb5_get_renewed_creds(void)
{
    if (verbose) puts("STUB: krb5_get_renewed_creds called");
    return NULL;
}

void* krb5_get_server_rcache(void)
{
    if (verbose) puts("STUB: krb5_get_server_rcache called");
    return NULL;
}

void* krb5_get_time_offsets(void)
{
    if (verbose) puts("STUB: krb5_get_time_offsets called");
    return NULL;
}

void* krb5_get_validated_creds(void)
{
    if (verbose) puts("STUB: krb5_get_validated_creds called");
    return NULL;
}

void* krb5_gss_register_acceptor_identity(void)
{
    if (verbose) puts("STUB: krb5_gss_register_acceptor_identity called");
    return NULL;
}

void* krb5_gss_use_kdc_context(void)
{
    if (verbose) puts("STUB: krb5_gss_use_kdc_context called");
    return NULL;
}

void* krb5_init_context(void)
{
    if (verbose) puts("STUB: krb5_init_context called");
    return NULL;
}

void* krb5_init_keyblock(void)
{
    if (verbose) puts("STUB: krb5_init_keyblock called");
    return NULL;
}

void* krb5_init_random_key(void)
{
    if (verbose) puts("STUB: krb5_init_random_key called");
    return NULL;
}

void* krb5_init_secure_context(void)
{
    if (verbose) puts("STUB: krb5_init_secure_context called");
    return NULL;
}

void* krb5_ipc_client_clear_target(void)
{
    if (verbose) puts("STUB: krb5_ipc_client_clear_target called");
    return NULL;
}

void* krb5_ipc_client_set_target_uid(void)
{
    if (verbose) puts("STUB: krb5_ipc_client_set_target_uid called");
    return NULL;
}

void* krb5_is_config_principal(void)
{
    if (verbose) puts("STUB: krb5_is_config_principal called");
    return NULL;
}

void* krb5_is_referral_realm(void)
{
    if (verbose) puts("STUB: krb5_is_referral_realm called");
    return NULL;
}

void* krb5_is_thread_safe(void)
{
    if (verbose) puts("STUB: krb5_is_thread_safe called");
    return NULL;
}

void* krb5_kt_add_entry(void)
{
    if (verbose) puts("STUB: krb5_kt_add_entry called");
    return NULL;
}

void* krb5_kt_close(void)
{
    if (verbose) puts("STUB: krb5_kt_close called");
    return NULL;
}

void* krb5_kt_default(void)
{
    if (verbose) puts("STUB: krb5_kt_default called");
    return NULL;
}

void* krb5_kt_default_name(void)
{
    if (verbose) puts("STUB: krb5_kt_default_name called");
    return NULL;
}

void* krb5_kt_end_seq_get(void)
{
    if (verbose) puts("STUB: krb5_kt_end_seq_get called");
    return NULL;
}

void* krb5_kt_get_entry(void)
{
    if (verbose) puts("STUB: krb5_kt_get_entry called");
    return NULL;
}

void* krb5_kt_get_name(void)
{
    if (verbose) puts("STUB: krb5_kt_get_name called");
    return NULL;
}

void* krb5_kt_get_type(void)
{
    if (verbose) puts("STUB: krb5_kt_get_type called");
    return NULL;
}

void* krb5_kt_next_entry(void)
{
    if (verbose) puts("STUB: krb5_kt_next_entry called");
    return NULL;
}

void* krb5_kt_read_service_key(void)
{
    if (verbose) puts("STUB: krb5_kt_read_service_key called");
    return NULL;
}

void* krb5_kt_remove_entry(void)
{
    if (verbose) puts("STUB: krb5_kt_remove_entry called");
    return NULL;
}

void* krb5_kt_resolve(void)
{
    if (verbose) puts("STUB: krb5_kt_resolve called");
    return NULL;
}

void* krb5_kt_start_seq_get(void)
{
    if (verbose) puts("STUB: krb5_kt_start_seq_get called");
    return NULL;
}

void* krb5_kuserok(void)
{
    if (verbose) puts("STUB: krb5_kuserok called");
    return NULL;
}

void* krb5_mk_1cred(void)
{
    if (verbose) puts("STUB: krb5_mk_1cred called");
    return NULL;
}

void* krb5_mk_error(void)
{
    if (verbose) puts("STUB: krb5_mk_error called");
    return NULL;
}

void* krb5_mk_ncred(void)
{
    if (verbose) puts("STUB: krb5_mk_ncred called");
    return NULL;
}

void* krb5_mk_priv(void)
{
    if (verbose) puts("STUB: krb5_mk_priv called");
    return NULL;
}

void* krb5_mk_rep(void)
{
    if (verbose) puts("STUB: krb5_mk_rep called");
    return NULL;
}

void* krb5_mk_req(void)
{
    if (verbose) puts("STUB: krb5_mk_req called");
    return NULL;
}

void* krb5_mk_req_extended(void)
{
    if (verbose) puts("STUB: krb5_mk_req_extended called");
    return NULL;
}

void* krb5_mk_safe(void)
{
    if (verbose) puts("STUB: krb5_mk_safe called");
    return NULL;
}

void* krb5_os_localaddr(void)
{
    if (verbose) puts("STUB: krb5_os_localaddr called");
    return NULL;
}

void* krb5_parse_name(void)
{
    if (verbose) puts("STUB: krb5_parse_name called");
    return NULL;
}

void* krb5_parse_name_flags(void)
{
    if (verbose) puts("STUB: krb5_parse_name_flags called");
    return NULL;
}

void* krb5_pkinit_cert_hash_str(void)
{
    if (verbose) puts("STUB: krb5_pkinit_cert_hash_str called");
    return NULL;
}

void* krb5_pkinit_get_client_cert(void)
{
    if (verbose) puts("STUB: krb5_pkinit_get_client_cert called");
    return NULL;
}

void* krb5_pkinit_get_client_cert_db(void)
{
    if (verbose) puts("STUB: krb5_pkinit_get_client_cert_db called");
    return NULL;
}

void* krb5_pkinit_get_kdc_cert(void)
{
    if (verbose) puts("STUB: krb5_pkinit_get_kdc_cert called");
    return NULL;
}

void* krb5_pkinit_get_kdc_cert_db(void)
{
    if (verbose) puts("STUB: krb5_pkinit_get_kdc_cert_db called");
    return NULL;
}

void* krb5_pkinit_have_client_cert(void)
{
    if (verbose) puts("STUB: krb5_pkinit_have_client_cert called");
    return NULL;
}

void* krb5_pkinit_release_cert(void)
{
    if (verbose) puts("STUB: krb5_pkinit_release_cert called");
    return NULL;
}

void* krb5_pkinit_release_cert_db(void)
{
    if (verbose) puts("STUB: krb5_pkinit_release_cert_db called");
    return NULL;
}

void* krb5_pkinit_set_client_cert(void)
{
    if (verbose) puts("STUB: krb5_pkinit_set_client_cert called");
    return NULL;
}

void* krb5_principal2salt(void)
{
    if (verbose) puts("STUB: krb5_principal2salt called");
    return NULL;
}

void* krb5_principal_compare(void)
{
    if (verbose) puts("STUB: krb5_principal_compare called");
    return NULL;
}

void* krb5_process_key(void)
{
    if (verbose) puts("STUB: krb5_process_key called");
    return NULL;
}

void* krb5_prompter_posix(void)
{
    if (verbose) puts("STUB: krb5_prompter_posix called");
    return NULL;
}

void* krb5_random_key(void)
{
    if (verbose) puts("STUB: krb5_random_key called");
    return NULL;
}

void* krb5_rd_cred(void)
{
    if (verbose) puts("STUB: krb5_rd_cred called");
    return NULL;
}

void* krb5_rd_error(void)
{
    if (verbose) puts("STUB: krb5_rd_error called");
    return NULL;
}

void* krb5_rd_priv(void)
{
    if (verbose) puts("STUB: krb5_rd_priv called");
    return NULL;
}

void* krb5_rd_rep(void)
{
    if (verbose) puts("STUB: krb5_rd_rep called");
    return NULL;
}

void* krb5_rd_req(void)
{
    if (verbose) puts("STUB: krb5_rd_req called");
    return NULL;
}

void* krb5_rd_safe(void)
{
    if (verbose) puts("STUB: krb5_rd_safe called");
    return NULL;
}

void* krb5_read_password(void)
{
    if (verbose) puts("STUB: krb5_read_password called");
    return NULL;
}

void* krb5_realm_compare(void)
{
    if (verbose) puts("STUB: krb5_realm_compare called");
    return NULL;
}

void* krb5_recvauth(void)
{
    if (verbose) puts("STUB: krb5_recvauth called");
    return NULL;
}

void* krb5_recvauth_version(void)
{
    if (verbose) puts("STUB: krb5_recvauth_version called");
    return NULL;
}

void* krb5_salttype_to_string(void)
{
    if (verbose) puts("STUB: krb5_salttype_to_string called");
    return NULL;
}

void* krb5_sendauth(void)
{
    if (verbose) puts("STUB: krb5_sendauth called");
    return NULL;
}

void* krb5_server_decrypt_ticket_keytab(void)
{
    if (verbose) puts("STUB: krb5_server_decrypt_ticket_keytab called");
    return NULL;
}

void* krb5_set_default_realm(void)
{
    if (verbose) puts("STUB: krb5_set_default_realm called");
    return NULL;
}

void* krb5_set_default_tgs_enctypes(void)
{
    if (verbose) puts("STUB: krb5_set_default_tgs_enctypes called");
    return NULL;
}

void* krb5_set_default_tgs_ktypes(void)
{
    if (verbose) puts("STUB: krb5_set_default_tgs_ktypes called");
    return NULL;
}

void* krb5_set_error_message(void)
{
    if (verbose) puts("STUB: krb5_set_error_message called");
    return NULL;
}

void* krb5_set_password(void)
{
    if (verbose) puts("STUB: krb5_set_password called");
    return NULL;
}

void* krb5_set_password_using_ccache(void)
{
    if (verbose) puts("STUB: krb5_set_password_using_ccache called");
    return NULL;
}

void* krb5_set_principal_realm(void)
{
    if (verbose) puts("STUB: krb5_set_principal_realm called");
    return NULL;
}

void* krb5_set_real_time(void)
{
    if (verbose) puts("STUB: krb5_set_real_time called");
    return NULL;
}

void* krb5_sname_to_principal(void)
{
    if (verbose) puts("STUB: krb5_sname_to_principal called");
    return NULL;
}

void* krb5_string_to_cksumtype(void)
{
    if (verbose) puts("STUB: krb5_string_to_cksumtype called");
    return NULL;
}

void* krb5_string_to_deltat(void)
{
    if (verbose) puts("STUB: krb5_string_to_deltat called");
    return NULL;
}

void* krb5_string_to_enctype(void)
{
    if (verbose) puts("STUB: krb5_string_to_enctype called");
    return NULL;
}

void* krb5_string_to_key(void)
{
    if (verbose) puts("STUB: krb5_string_to_key called");
    return NULL;
}

void* krb5_string_to_salttype(void)
{
    if (verbose) puts("STUB: krb5_string_to_salttype called");
    return NULL;
}

void* krb5_string_to_timestamp(void)
{
    if (verbose) puts("STUB: krb5_string_to_timestamp called");
    return NULL;
}

void* krb5_timeofday(void)
{
    if (verbose) puts("STUB: krb5_timeofday called");
    return NULL;
}

void* krb5_timestamp_to_sfstring(void)
{
    if (verbose) puts("STUB: krb5_timestamp_to_sfstring called");
    return NULL;
}

void* krb5_timestamp_to_string(void)
{
    if (verbose) puts("STUB: krb5_timestamp_to_string called");
    return NULL;
}

void* krb5_unparse_name(void)
{
    if (verbose) puts("STUB: krb5_unparse_name called");
    return NULL;
}

void* krb5_unparse_name_ext(void)
{
    if (verbose) puts("STUB: krb5_unparse_name_ext called");
    return NULL;
}

void* krb5_unparse_name_flags(void)
{
    if (verbose) puts("STUB: krb5_unparse_name_flags called");
    return NULL;
}

void* krb5_unparse_name_flags_ext(void)
{
    if (verbose) puts("STUB: krb5_unparse_name_flags_ext called");
    return NULL;
}

void* krb5_us_timeofday(void)
{
    if (verbose) puts("STUB: krb5_us_timeofday called");
    return NULL;
}

void* krb5_use_enctype(void)
{
    if (verbose) puts("STUB: krb5_use_enctype called");
    return NULL;
}

void* krb5_verify_checksum(void)
{
    if (verbose) puts("STUB: krb5_verify_checksum called");
    return NULL;
}

void* krb5_verify_init_creds(void)
{
    if (verbose) puts("STUB: krb5_verify_init_creds called");
    return NULL;
}

void* krb5_verify_init_creds_opt_init(void)
{
    if (verbose) puts("STUB: krb5_verify_init_creds_opt_init called");
    return NULL;
}

void* krb5_verify_init_creds_opt_set_ap_req_nofail(void)
{
    if (verbose) puts("STUB: krb5_verify_init_creds_opt_set_ap_req_nofail called");
    return NULL;
}

void* krb5_vset_error_message(void)
{
    if (verbose) puts("STUB: krb5_vset_error_message called");
    return NULL;
}

void* krb5int_accessor(void)
{
    if (verbose) puts("STUB: krb5int_accessor called");
    return NULL;
}

void* krb5int_freeaddrinfo(void)
{
    if (verbose) puts("STUB: krb5int_freeaddrinfo called");
    return NULL;
}

void* krb5int_gai_strerror(void)
{
    if (verbose) puts("STUB: krb5int_gai_strerror called");
    return NULL;
}

void* krb5int_getaddrinfo(void)
{
    if (verbose) puts("STUB: krb5int_getaddrinfo called");
    return NULL;
}

void* krb5int_gmt_mktime(void)
{
    if (verbose) puts("STUB: krb5int_gmt_mktime called");
    return NULL;
}

void* krb5int_init_context_kdc(void)
{
    if (verbose) puts("STUB: krb5int_init_context_kdc called");
    return NULL;
}

void* krb5int_pkinit_auth_pack_decode(void)
{
    if (verbose) puts("STUB: krb5int_pkinit_auth_pack_decode called");
    return NULL;
}

void* krb5int_pkinit_create_cms_msg(void)
{
    if (verbose) puts("STUB: krb5int_pkinit_create_cms_msg called");
    return NULL;
}

void* krb5int_pkinit_pa_pk_as_rep_encode(void)
{
    if (verbose) puts("STUB: krb5int_pkinit_pa_pk_as_rep_encode called");
    return NULL;
}

void* krb5int_pkinit_pa_pk_as_req_decode(void)
{
    if (verbose) puts("STUB: krb5int_pkinit_pa_pk_as_req_decode called");
    return NULL;
}

void* krb5int_pkinit_parse_cms_msg(void)
{
    if (verbose) puts("STUB: krb5int_pkinit_parse_cms_msg called");
    return NULL;
}

void* krb5int_pkinit_reply_key_pack_encode(void)
{
    if (verbose) puts("STUB: krb5int_pkinit_reply_key_pack_encode called");
    return NULL;
}

void* krb__get_srvtabname(void)
{
    if (verbose) puts("STUB: krb__get_srvtabname called");
    return NULL;
}

void* krb_change_password(void)
{
    if (verbose) puts("STUB: krb_change_password called");
    return NULL;
}

void* krb_check_auth(void)
{
    if (verbose) puts("STUB: krb_check_auth called");
    return NULL;
}

void* krb_delete_cred(void)
{
    if (verbose) puts("STUB: krb_delete_cred called");
    return NULL;
}

void* krb_get_admhst(void)
{
    if (verbose) puts("STUB: krb_get_admhst called");
    return NULL;
}

void* krb_get_cred(void)
{
    if (verbose) puts("STUB: krb_get_cred called");
    return NULL;
}

void* krb_get_err_text(void)
{
    if (verbose) puts("STUB: krb_get_err_text called");
    return NULL;
}

void* krb_get_in_tkt(void)
{
    if (verbose) puts("STUB: krb_get_in_tkt called");
    return NULL;
}

void* krb_get_in_tkt_creds(void)
{
    if (verbose) puts("STUB: krb_get_in_tkt_creds called");
    return NULL;
}

void* krb_get_krbhst(void)
{
    if (verbose) puts("STUB: krb_get_krbhst called");
    return NULL;
}

void* krb_get_lrealm(void)
{
    if (verbose) puts("STUB: krb_get_lrealm called");
    return NULL;
}

void* krb_get_nth_cred(void)
{
    if (verbose) puts("STUB: krb_get_nth_cred called");
    return NULL;
}

void* krb_get_num_cred(void)
{
    if (verbose) puts("STUB: krb_get_num_cred called");
    return NULL;
}

void* krb_get_phost(void)
{
    if (verbose) puts("STUB: krb_get_phost called");
    return NULL;
}

void* krb_get_profile(void)
{
    if (verbose) puts("STUB: krb_get_profile called");
    return NULL;
}

void* krb_get_pw_in_tkt(void)
{
    if (verbose) puts("STUB: krb_get_pw_in_tkt called");
    return NULL;
}

void* krb_get_pw_in_tkt_creds(void)
{
    if (verbose) puts("STUB: krb_get_pw_in_tkt_creds called");
    return NULL;
}

void* krb_get_svc_in_tkt(void)
{
    if (verbose) puts("STUB: krb_get_svc_in_tkt called");
    return NULL;
}

void* krb_get_tf_fullname(void)
{
    if (verbose) puts("STUB: krb_get_tf_fullname called");
    return NULL;
}

void* krb_get_tf_realm(void)
{
    if (verbose) puts("STUB: krb_get_tf_realm called");
    return NULL;
}

void* krb_get_ticket_for_service(void)
{
    if (verbose) puts("STUB: krb_get_ticket_for_service called");
    return NULL;
}

void* krb_life_to_time(void)
{
    if (verbose) puts("STUB: krb_life_to_time called");
    return NULL;
}

void* krb_mk_auth(void)
{
    if (verbose) puts("STUB: krb_mk_auth called");
    return NULL;
}

void* krb_mk_err(void)
{
    if (verbose) puts("STUB: krb_mk_err called");
    return NULL;
}

void* krb_mk_priv(void)
{
    if (verbose) puts("STUB: krb_mk_priv called");
    return NULL;
}

void* krb_mk_req(void)
{
    if (verbose) puts("STUB: krb_mk_req called");
    return NULL;
}

void* krb_mk_req_creds(void)
{
    if (verbose) puts("STUB: krb_mk_req_creds called");
    return NULL;
}

void* krb_mk_safe(void)
{
    if (verbose) puts("STUB: krb_mk_safe called");
    return NULL;
}

void* krb_rd_err(void)
{
    if (verbose) puts("STUB: krb_rd_err called");
    return NULL;
}

void* krb_rd_priv(void)
{
    if (verbose) puts("STUB: krb_rd_priv called");
    return NULL;
}

void* krb_rd_req(void)
{
    if (verbose) puts("STUB: krb_rd_req called");
    return NULL;
}

void* krb_rd_req_int(void)
{
    if (verbose) puts("STUB: krb_rd_req_int called");
    return NULL;
}

void* krb_rd_safe(void)
{
    if (verbose) puts("STUB: krb_rd_safe called");
    return NULL;
}

void* krb_realmofhost(void)
{
    if (verbose) puts("STUB: krb_realmofhost called");
    return NULL;
}

void* krb_recvauth(void)
{
    if (verbose) puts("STUB: krb_recvauth called");
    return NULL;
}

void* krb_sendauth(void)
{
    if (verbose) puts("STUB: krb_sendauth called");
    return NULL;
}

void* krb_set_lifetime(void)
{
    if (verbose) puts("STUB: krb_set_lifetime called");
    return NULL;
}

void* krb_set_tkt_string(void)
{
    if (verbose) puts("STUB: krb_set_tkt_string called");
    return NULL;
}

void* krb_time_to_life(void)
{
    if (verbose) puts("STUB: krb_time_to_life called");
    return NULL;
}

void* kuserok(void)
{
    if (verbose) puts("STUB: kuserok called");
    return NULL;
}

void* profile_abandon(void)
{
    if (verbose) puts("STUB: profile_abandon called");
    return NULL;
}

void* profile_add_relation(void)
{
    if (verbose) puts("STUB: profile_add_relation called");
    return NULL;
}

void* profile_clear_relation(void)
{
    if (verbose) puts("STUB: profile_clear_relation called");
    return NULL;
}

void* profile_configuration_updated(void)
{
    if (verbose) puts("STUB: profile_configuration_updated called");
    return NULL;
}

void* profile_flush(void)
{
    if (verbose) puts("STUB: profile_flush called");
    return NULL;
}

void* profile_flush_to_buffer(void)
{
    if (verbose) puts("STUB: profile_flush_to_buffer called");
    return NULL;
}

void* profile_flush_to_file(void)
{
    if (verbose) puts("STUB: profile_flush_to_file called");
    return NULL;
}

void* profile_free_buffer(void)
{
    if (verbose) puts("STUB: profile_free_buffer called");
    return NULL;
}

void* profile_free_list(void)
{
    if (verbose) puts("STUB: profile_free_list called");
    return NULL;
}

void* profile_get_boolean(void)
{
    if (verbose) puts("STUB: profile_get_boolean called");
    return NULL;
}

void* profile_get_integer(void)
{
    if (verbose) puts("STUB: profile_get_integer called");
    return NULL;
}

void* profile_get_relation_names(void)
{
    if (verbose) puts("STUB: profile_get_relation_names called");
    return NULL;
}

void* profile_get_string(void)
{
    if (verbose) puts("STUB: profile_get_string called");
    return NULL;
}

void* profile_get_subsection_names(void)
{
    if (verbose) puts("STUB: profile_get_subsection_names called");
    return NULL;
}

void* profile_get_values(void)
{
    if (verbose) puts("STUB: profile_get_values called");
    return NULL;
}

void* profile_init(void)
{
    if (verbose) puts("STUB: profile_init called");
    return NULL;
}

void* profile_init_path(void)
{
    if (verbose) puts("STUB: profile_init_path called");
    return NULL;
}

void* profile_is_modified(void)
{
    if (verbose) puts("STUB: profile_is_modified called");
    return NULL;
}

void* profile_is_writable(void)
{
    if (verbose) puts("STUB: profile_is_writable called");
    return NULL;
}

void* profile_iterator(void)
{
    if (verbose) puts("STUB: profile_iterator called");
    return NULL;
}

void* profile_iterator_create(void)
{
    if (verbose) puts("STUB: profile_iterator_create called");
    return NULL;
}

void* profile_iterator_free(void)
{
    if (verbose) puts("STUB: profile_iterator_free called");
    return NULL;
}

void* profile_release(void)
{
    if (verbose) puts("STUB: profile_release called");
    return NULL;
}

void* profile_release_string(void)
{
    if (verbose) puts("STUB: profile_release_string called");
    return NULL;
}

void* profile_rename_section(void)
{
    if (verbose) puts("STUB: profile_rename_section called");
    return NULL;
}

void* profile_update_relation(void)
{
    if (verbose) puts("STUB: profile_update_relation called");
    return NULL;
}

void* put_svc_key(void)
{
    if (verbose) puts("STUB: put_svc_key called");
    return NULL;
}

void* read_service_key(void)
{
    if (verbose) puts("STUB: read_service_key called");
    return NULL;
}

void* remove_error_table(void)
{
    if (verbose) puts("STUB: remove_error_table called");
    return NULL;
}

void* reset_com_err_hook(void)
{
    if (verbose) puts("STUB: reset_com_err_hook called");
    return NULL;
}

void* set_com_err_hook(void)
{
    if (verbose) puts("STUB: set_com_err_hook called");
    return NULL;
}

void* tkt_string(void)
{
    if (verbose) puts("STUB: tkt_string called");
    return NULL;
}

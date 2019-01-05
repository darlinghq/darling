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


#ifndef _SecurityFoundation_H_
#define _SecurityFoundation_H_

#import <Foundation/Foundation.h>

#import <SecurityFoundation/KeychainProxyProtocol.h>
#import <SecurityFoundation/SFDigestOperation.h>
#import <SecurityFoundation/SFDigestOperationInternal.h>
#import <SecurityFoundation/SFIVGenerator.h>
#import <SecurityFoundation/SFKeyDerivingOperation.h>
#import <SecurityFoundation/SFKeychainItemAttributes.h>
#import <SecurityFoundation/SFKeychainServerProtocol.h>
#import <SecurityFoundation/SFMaskGenerationFunction.h>
#import <SecurityFoundation/SFPseudoRandomFunction.h>
#import <SecurityFoundation/SFSigningOperation.h>
#import <SecurityFoundation/_SFEncryptionOperation.h>
#import <SecurityFoundation/_SFKeyAttributes.h>
#import <SecurityFoundation/SFAuthorization.h>
#import <SecurityFoundation/SFSyncManager_ivars.h>
#import <SecurityFoundation/SFSyncManager.h>
#import <SecurityFoundation/SFCredentialSearchFilter_Ivars.h>
#import <SecurityFoundation/_SFCredential.h>
#import <SecurityFoundation/_SFPasswordCredential.h>
#import <SecurityFoundation/SFCredentialSearchFilter.h>
#import <SecurityFoundation/CAAdditionalCertInfo.h>
#import <SecurityFoundation/SFRevocationPolicy_Ivars.h>
#import <SecurityFoundation/SFTrustEvaluator_Ivars.h>
#import <SecurityFoundation/SFSSLClientTrustPolicy_Ivars.h>
#import <SecurityFoundation/SFSSLServerTrustPolicy_Ivars.h>
#import <SecurityFoundation/SFRevocationPolicy.h>
#import <SecurityFoundation/SFTrustEvaluator.h>
#import <SecurityFoundation/SFTrustPolicy.h>
#import <SecurityFoundation/SFX509TrustPolicy.h>
#import <SecurityFoundation/SFSSLClientTrustPolicy.h>
#import <SecurityFoundation/SFSSLServerTrustPolicy.h>
#import <SecurityFoundation/SFIdentity_Ivars.h>
#import <SecurityFoundation/SFCertificate_Ivars.h>
#import <SecurityFoundation/SFIdentityAttributes_Ivars.h>
#import <SecurityFoundation/SFIdentitySearchFilter_Ivars.h>
#import <SecurityFoundation/_SFIdentity.h>
#import <SecurityFoundation/_SFCertificate.h>
#import <SecurityFoundation/SFIdentityAttributes.h>
#import <SecurityFoundation/SFIdentitySearchFilter.h>
#import <SecurityFoundation/CABasicConstraintsExtension.h>
#import <SecurityFoundation/SFCodingOptions_Ivars.h>
#import <SecurityFoundation/SFCoder_Ivars.h>
#import <SecurityFoundation/SFDecoder_Ivars.h>
#import <SecurityFoundation/SFCodingOptions.h>
#import <SecurityFoundation/SFCoder.h>
#import <SecurityFoundation/SFDecoder.h>
#import <SecurityFoundation/CACertInfo.h>
#import <SecurityFoundation/CAExtendedKeyUsageExtension.h>
#import <SecurityFoundation/CAIdentityName.h>
#import <SecurityFoundation/CAKeyPairAttributes.h>
#import <SecurityFoundation/SFKeychainFetchResult_Ivars.h>
#import <SecurityFoundation/SFKeychainManager_Ivars.h>
#import <SecurityFoundation/SFKeychainDataAttributes_Ivars.h>
#import <SecurityFoundation/SFKeychainData_Ivars.h>
#import <SecurityFoundation/_SFKeychainFetchResult.h>
#import <SecurityFoundation/_SFKeychainIdentityFetchResult.h>
#import <SecurityFoundation/_SFKeychainKeyFetchResult.h>
#import <SecurityFoundation/SFKeychainDataFetchResult.h>
#import <SecurityFoundation/_SFKeychainManager.h>
#import <SecurityFoundation/SFKeychainDataAttributes.h>
#import <SecurityFoundation/SFKeychainData.h>
#import <SecurityFoundation/CAKeyUsageExtension.h>
#import <SecurityFoundation/SFPBKDF2Operation_Ivars.h>
#import <SecurityFoundation/SFECDHKeySource_Ivars.h>
#import <SecurityFoundation/SFECDHOperation_Ivars.h>
#import <SecurityFoundation/SFX963KeyDerivationOperation_Ivars.h>
#import <SecurityFoundation/SFPBKDF2Operation.h>
#import <SecurityFoundation/_SFECDHKeySource.h>
#import <SecurityFoundation/_SFECDHOperation.h>
#import <SecurityFoundation/SFX963KeyDerivationOperation.h>
#import <SecurityFoundation/SFCertAuthorityInvitation.h>
#import <SecurityFoundation/SFCertAuthorityInvitationSigner.h>
#import <SecurityFoundation/SFAccessControlEntry_Ivars.h>
#import <SecurityFoundation/SFAccessControlEntry.h>
#import <SecurityFoundation/_SFAccessPolicy.h>
#import <SecurityFoundation/SFCertificateAuthority_ivars.h>
#import <SecurityFoundation/SFCertificateAuthority.h>
#import <SecurityFoundation/SFCertificateAuthorityClient.h>
#import <SecurityFoundation/SFCSR.h>
#import <SecurityFoundation/SFAESKeySpecifier_Ivars.h>
#import <SecurityFoundation/SFRSAKeySpecifier_Ivars.h>
#import <SecurityFoundation/SFECKeySpecifier_Ivars.h>
#import <SecurityFoundation/SFSymmetricKeyAttributes_Ivars.h>
#import <SecurityFoundation/SFKeySearchFilter_Ivars.h>
#import <SecurityFoundation/SFKey_Ivars.h>
#import <SecurityFoundation/SFSymmetricKey_Ivars.h>
#import <SecurityFoundation/SFKeyPair_Ivars.h>
#import <SecurityFoundation/SFPublicKey_Ivars.h>
#import <SecurityFoundation/_SFKeySpecifier.h>
#import <SecurityFoundation/_SFSymmetricKeySpecifier.h>
#import <SecurityFoundation/_SFTripleDESKeySpecifier.h>
#import <SecurityFoundation/_SFAESKeySpecifier.h>
#import <SecurityFoundation/_SFAsymmetricKeySpecifier.h>
#import <SecurityFoundation/_SFRSAKeySpecifier.h>
#import <SecurityFoundation/_SFECKeySpecifier.h>
#import <SecurityFoundation/SFSymmetricKeyAttributes.h>
#import <SecurityFoundation/_SFAsymmetricKeyAttributes.h>
#import <SecurityFoundation/SFKeySearchFilter.h>
#import <SecurityFoundation/_SFKey.h>
#import <SecurityFoundation/_SFSymmetricKey.h>
#import <SecurityFoundation/SFTripleDESKey.h>
#import <SecurityFoundation/_SFAESKey.h>
#import <SecurityFoundation/_SFPublicKey.h>
#import <SecurityFoundation/_SFKeyPair.h>
#import <SecurityFoundation/SFRSAPublicKey.h>
#import <SecurityFoundation/SFRSAKeyPair.h>
#import <SecurityFoundation/_SFECPublicKey.h>
#import <SecurityFoundation/_SFECKeyPair.h>
#import <SecurityFoundation/SFServiceIdentifier_Ivars.h>
#import <SecurityFoundation/_SFCredentialStore.h>
#import <SecurityFoundation/_SFServiceIdentifier.h>
#import <SecurityFoundation/SFMachPort.h>
#import <SecurityFoundation/CASubjectAltNameExtension.h>
#import <SecurityFoundation/SFSHA1DigestOperation_Ivars.h>
#import <SecurityFoundation/SFSHA224DigestOperation_Ivars.h>
#import <SecurityFoundation/SFSHA256DigestOperation_Ivars.h>
#import <SecurityFoundation/SFSHA384DigestOperation_Ivars.h>
#import <SecurityFoundation/SFSHA512DigestOperation_Ivars.h>
#import <SecurityFoundation/SFMD5DigestOperation_Ivars.h>
#import <SecurityFoundation/SFMaskGenerationFunction1_Ivars.h>
#import <SecurityFoundation/SFHMACPseudoRandomFunction_Ivars.h>
#import <SecurityFoundation/_SFSHA1DigestOperation.h>
#import <SecurityFoundation/_SFSHA224DigestOperation.h>
#import <SecurityFoundation/_SFSHA256DigestOperation.h>
#import <SecurityFoundation/_SFSHA384DigestOperation.h>
#import <SecurityFoundation/_SFSHA512DigestOperation.h>
#import <SecurityFoundation/_SFMD5DigestOperation.h>
#import <SecurityFoundation/SFMaskGenerationFunction1.h>
#import <SecurityFoundation/SFHMACPseudoRandomFunction.h>
#import <SecurityFoundation/SFCiphertext_Ivars.h>
#import <SecurityFoundation/SFAuthenticatedCiphertext_Ivars.h>
#import <SecurityFoundation/SFWrappedKeyCiphertext_Ivars.h>
#import <SecurityFoundation/SFIESCiphertext_Ivars.h>
#import <SecurityFoundation/SFSymmetricEncryptionOperation_Ivars.h>
#import <SecurityFoundation/SFAuthenticatedEncryptionOperation_Ivars.h>
#import <SecurityFoundation/SFRSAEncryptionOperation_Ivars.h>
#import <SecurityFoundation/SFRSA_OAEPEncryptionOperation_Ivars.h>
#import <SecurityFoundation/SFRSA_WrappedKeyEncryptionOperation_Ivars.h>
#import <SecurityFoundation/SFIESOperation_Ivars.h>
#import <SecurityFoundation/_SFCiphertext.h>
#import <SecurityFoundation/_SFAuthenticatedCiphertext.h>
#import <SecurityFoundation/SFWrappedKeyCiphertext.h>
#import <SecurityFoundation/_SFIESCiphertext.h>
#import <SecurityFoundation/SFSymmetricEncryptionOperation.h>
#import <SecurityFoundation/SFTripleDESEncryptionOperation.h>
#import <SecurityFoundation/SFAESEncryptionOperation.h>
#import <SecurityFoundation/_SFAuthenticatedEncryptionOperation.h>
#import <SecurityFoundation/SFRSAEncryptionOperation.h>
#import <SecurityFoundation/SFRSA_PKCS1EncryptionOperation.h>
#import <SecurityFoundation/SFRSA_OAEPEncryptionOperation.h>
#import <SecurityFoundation/SFRSA_WrappedKeyEncryptionOperation.h>
#import <SecurityFoundation/_SFIESOperation.h>
#import <SecurityFoundation/SFIncrementingIVGenerator_Ivars.h>
#import <SecurityFoundation/SFIncrementingIVGenerator.h>
#import <SecurityFoundation/OIDStringCache.h>
#import <SecurityFoundation/SFOidParser.h>
#import <SecurityFoundation/SFSignedData_Ivars.h>
#import <SecurityFoundation/SFRSASigningOperation_Ivars.h>
#import <SecurityFoundation/SFRSA_PSSSigningOperation_Ivars.h>
#import <SecurityFoundation/SFEC_X962SigningOperation_Ivars.h>
#import <SecurityFoundation/SFCMACOperation_Ivars.h>
#import <SecurityFoundation/SFHMACOperation_Ivars.h>
#import <SecurityFoundation/_SFSignedData.h>
#import <SecurityFoundation/SFRSASigningOperation.h>
#import <SecurityFoundation/SFRSA_PSSSigningOperation.h>
#import <SecurityFoundation/_SFEC_X962SigningOperation.h>
#import <SecurityFoundation/SFMessageAuthenticationCodeOperation.h>
#import <SecurityFoundation/SFCMACOperation.h>
#import <SecurityFoundation/SFHMACOperation.h>
#import <SecurityFoundation/SFCertificateData_ivars.h>
#import <SecurityFoundation/SFCertificateData.h>
#import <SecurityFoundation/SecFoundationModVector.h>

void* SFAccessibilityMakeWithMode(void);
void* SFKeychainServerXPCInterface(void);
void* SFPWACStringInDictionary(void);
void* SFPWAContextCreate(void);
void* SFPWAContextCreateWithDefaults(void);
void* SFPWAContextLoadDictionaries(void);
void* SFPWAContextRelease(void);
void* SFPWAPasswordEvaluator(void);
void* SFPWAPasswordSuggest(void);
void* SFPWAPolicyCopyDefault(void);
void* SFPWAPolicyCopyFromFile(void);
void* SFPWAPolicyLanguages(void);
void* SFPWAPolicyParse(void);
void* SFPWARandomWord(void);
void* SFPWAStringInDictionary(void);
void* SecFileVaultChangeMasterPasswordPassword(void);
void* SecFileVaultCompact(void);
void* SecFileVaultCreate(void);
void* SecFileVaultCreateUsingCertificate(void);
void* SecFileVaultMakeMasterPassword(void);
void* SecFileVaultMasterMount(void);
void* SecFileVaultMasterPasswordEnabled(void);
void* SecFileVaultMasterPasswordValidates(void);
void* SecFileVaultMount(void);
void* SecFileVaultRecover(void);
void* SecFileVaultResize(void);
void* SecFileVaultUnmount(void);
void* SecFileVaultUserChangePassword(void);
void* SecFileVaultUserMount(void);
void* _Z13SFPWARandWordlji(void);
void* _Z17SecCreateSHA1HMACPK10__CFStringPK8__CFData(void);
void* _Z17SecVerifySHA1HMACPK10__CFStringPK8__CFDataS4_(void);
void* _Z26SFPWAStdStringInDictionarylNSt3__112basic_stringIcNS_11char_traitsIcEENS_9allocatorIcEEEE(void);
void* _ZN10CertParser10initFieldsEv(void);
void* _ZN10CertParser11fieldForOidERK9cssm_dataj(void);
void* _ZN10CertParser12initWithDataERK9cssm_data(void);
void* _ZN10CertParser14initWithCFDataEPK8__CFData(void);
void* _ZN10CertParser15extensionForOidERK9cssm_data(void);
void* _ZN10CertParser15initWithSecCertEP23OpaqueSecCertificateRef(void);
void* _ZN10CertParserC1EP23OpaqueSecCertificateRef(void);
void* _ZN10CertParserC1El(void);
void* _ZN10CertParserC1ElRK9cssm_data(void);
void* _ZN10CertParserC1Ev(void);
void* _ZN10CertParserC2EP23OpaqueSecCertificateRef(void);
void* _ZN10CertParserC2El(void);
void* _ZN10CertParserC2ElRK9cssm_data(void);
void* _ZN10CertParserC2Ev(void);
void* _ZN10CertParserD1Ev(void);
void* _ZN10CertParserD2Ev(void);
void* _copyIdentityForCMSDecoder(void);
void* caIPAddressIntValue(void);
void* certPrintName(void);
void* createCertCsr(void);
void* nssNameToCSSM(void);
void* subjPubKeyInfoToCssmKey(void);

#endif

#ifndef _CTK_CLIENT_H_
#define _CTK_CLIENT_H_

#include <CoreFoundation/CoreFoundation.h>

#include <ACMAclDefs.h>
#include <ACMDefs.h>

CF_IMPLICIT_BRIDGING_ENABLED
CF_EXTERN_C_BEGIN

#define kTKErrorDomain "TKErrorDomain"
#define kTKErrorCodeNotImplemented -1
#define kTKErrorCodeCommunicationError -2
#define kTKErrorCodeCorruptedData -3
#define kTKErrorCodeCanceledByUser -4
#define kTKErrorCodeAuthenticationFailed -5
#define kTKErrorCodeObjectNotFound -6
#define kTKErrorCodeTokenNotFound -7
#define kTKErrorCodeBadParameter -8
#define kTKErrorCodeAuthenticationNeeded -9
#define kTKErrorAuthenticationFailed TKErrorCodeAuthenticationFailed
#define kTKErrorObjectNotFound TKErrorCodeObjectNotFound
#define kTKErrorTokenNotFound TKErrorCodeTokenNotFound

#define kTKTokenControlAttribAttestingKey "TKTokenControlAttribAttesting"
#define kTKTokenControlAttribKeyToAttest "TKTokenControlAttribKeyToAttest"
#define kTKTokenControlAttribAttestationData "TKTokenControlAttribAttestationData"
#define kTKTokenControlAttribLifetimeControlKey "TKTokenControlAttribLifetimeControlKey"
#define kTKTokenControlAttribLifetimeType "TKTokenControlAttribLifetimeType"

#define kTKTokenCreateAttributeAuxParams "TKTokenCreateAttributeAuxParams"
#define kTKTokenCreateAttributeTestMode "kTKTokenCreateAttributeTestMode"
#define TKTokenKeyUsageAny 1

typedef const struct CF_BRIDGED_TYPE(id) __TKToken * TKTokenRef;

CF_EXPORT
CFDataRef TKTokenCopyObjectAccessControl(TKTokenRef token, CFDataRef object_id, CFErrorRef *error);

CF_EXPORT
CFDataRef TKTokenCopyPublicKeyData(TKTokenRef token_id, CFDataRef objectID, CFErrorRef *error);

CF_EXPORT
int TKTokenDeleteObject(TKTokenRef token, CFDataRef object_id, CFErrorRef *error);

CF_EXPORT
CFDataRef TKTokenCopySignature(TKTokenRef token, CFDataRef objectID, uint32_t padding, CFDataRef digest, CFErrorRef *error);

CF_EXPORT
TKTokenRef TKTokenCreate(CFDictionaryRef token_attrs, CFErrorRef *error);

CF_EXPORT
CFDataRef TKTokenCreateOrUpdateObject(TKTokenRef token, CFDataRef objectID, CFDictionaryRef token_attrs, CFErrorRef *error);

CF_EXPORT
CFDataRef TKTokenCopyObjectData(TKTokenRef token, CFDataRef object_id, CFErrorRef *error);

CF_EXPORT
CFDataRef TKTokenCopyObjectCreationAccessControl(TKTokenRef token, CFTypeRef object_or_attrs, CFErrorRef *error);

CF_EXPORT
CFDataRef TKTokenCopyOperationResult(TKTokenRef token, CFDataRef objectID, int operation, CFArrayRef algorithms, int other, CFTypeRef in1, CFTypeRef in2, CFErrorRef *error);

CF_EXPORT
CFDictionaryRef TKTokenControl(TKTokenRef token, CFDictionaryRef attributes, CFErrorRef *error);

CF_EXPORT
CFArrayRef TKTokenCopyIdentities(TKTokenRef token, int usage, CFErrorRef *error);

CF_EXPORT
void TKBindUserAm(CFStringRef cf_user, CFDataRef pub_key_hash, void *unknown);

CF_EXTERN_C_END
CF_IMPLICIT_BRIDGING_DISABLED

#endif

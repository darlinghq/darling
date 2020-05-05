#include <ctkclient.h>

CFDataRef TKTokenCopyObjectAccessControl(TKTokenRef token, CFDataRef object_id, CFErrorRef *error) {
	return NULL;
}

CFDataRef TKTokenCopyPublicKeyData(TKTokenRef token_id, CFDataRef objectID, CFErrorRef *error) {
	return NULL;
}

int TKTokenDeleteObject(TKTokenRef token, CFDataRef object_id, CFErrorRef *error) {
	return 0;
}

CFDataRef TKTokenCopySignature(TKTokenRef token, CFDataRef objectID, uint32_t padding, CFDataRef digest, CFErrorRef *error) {
	return NULL;
}

TKTokenRef TKTokenCreate(CFDictionaryRef token_attrs, CFErrorRef *error) {
	return NULL;
}

CFDataRef TKTokenCreateOrUpdateObject(TKTokenRef token, CFDataRef objectID, CFDictionaryRef token_attrs, CFErrorRef *error) {
	return NULL;
}

CFDataRef TKTokenCopyObjectData(TKTokenRef token, CFDataRef object_id, CFErrorRef *error) {
	return NULL;
}

CFDataRef TKTokenCopyObjectCreationAccessControl(TKTokenRef token, CFTypeRef object_or_attrs, CFErrorRef *error) {
	return NULL;
}

CFDictionaryRef TKTokenControl(TKTokenRef token, CFDictionaryRef attributes, CFErrorRef *error)
{
	return NULL;
}

CFArrayRef TKTokenCopyIdentities(TKTokenRef token, int usage, CFErrorRef *error)
{
	return NULL;
}

CFDataRef TKTokenCopyOperationResult(TKTokenRef token, CFDataRef objectID, int operation, CFArrayRef algorithms, int other, CFTypeRef in1, CFTypeRef in2, CFErrorRef *error)
{
	return NULL;
}

#include <ACMLib.h>

ACMContextRef ACMContextCreateWithExternalForm(const void *externalForm, size_t dataLength) {
    printf("STUB %s\n", __PRETTY_FUNCTION__);
    return NULL;
}

ACMStatus ACMContextDelete(ACMContextRef context, bool destroyContext) {
    printf("STUB %s\n", __PRETTY_FUNCTION__);
    return -1;
}

ACMStatus ACMContextRemovePassphraseCredentialsByPurposeAndScope(const ACMContextRef context, ACMPassphrasePurpose purpose, ACMScope scope) {
    printf("STUB %s\n", __PRETTY_FUNCTION__);
    return -1;
}

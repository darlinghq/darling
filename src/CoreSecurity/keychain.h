#ifndef CORESECURITY_KEYCHAIN_H
#define CORESECURITY_KEYCHAIN_H
#include "dummy.h"

DUMMY(SecCertificateGetData);
DUMMY(SecKeychainItemCopyContent);
DUMMY(SecKeychainItemFreeContent);
DUMMY(SecKeychainOpen);
DUMMY(SecKeychainSearchCopyNext);
DUMMY(SecKeychainSearchCreateFromAttributes);

#endif

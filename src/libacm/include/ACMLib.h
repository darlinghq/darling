#ifndef _ACMLIB_H_
#define _ACMLIB_H_

#include <ACMAclDefs.h>
#include <ACMDefs.h>

// CF type
typedef void* ACMContextRef;

typedef int ACMStatus;
typedef char* ACMPassphrasePurpose;
typedef char* ACMScope;

ACMContextRef ACMContextCreateWithExternalForm(const void *externalForm, size_t dataLength);
ACMStatus ACMContextDelete(ACMContextRef context, bool destroyContext);
ACMStatus ACMContextRemovePassphraseCredentialsByPurposeAndScope(const ACMContextRef context, ACMPassphrasePurpose purpose, ACMScope scope);

#endif // _ACMLIB_H_

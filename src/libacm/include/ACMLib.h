#ifndef _ACMLIB_H_
#define _ACMLIB_H_

#include <ACMAclDefs.h>
#include <ACMDefs.h>

// CF type
typedef void* ACMContextRef;

ACMContextRef ACMContextCreateWithExternalForm(const void *externalForm, size_t dataLength);

#endif // _ACMLIB_H_

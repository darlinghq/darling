#ifndef _LIBAKS_SMARTCARD_H_
#define _LIBAKS_SMARTCARD_H_

#include <LocalAuthentication/LACFSupport.h>

typedef int aks_smartcard_mode_t;

#define AKS_SMARTCARD_MODE_RSA 1
#define AKS_SMARTCARD_MODE_ECDH 2

CFRef<CFArrayRef> LACreateNewContextWithACMContext(int a, int b);
void aks_smartcard_unregister(int a);
int aks_smartcard_request_unlock(int a, unsigned char *b, int c, void **d, size_t *e);
int aks_smartcard_unlock(int a, unsigned char *b, int c, unsigned char *d, int e, void **f, size_t *g);
int aks_smartcard_register(int a, unsigned char *b, int c, int d, unsigned char *e, int f, void **g, size_t *h);
int aks_smartcard_get_sc_usk(void *a, int b, const void **c, size_t *d);
int aks_smartcard_get_ec_pub(void *a, int b, const void **c, size_t *d);

#endif

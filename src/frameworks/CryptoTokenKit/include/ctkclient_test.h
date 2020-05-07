#ifndef _CTK_CTKCLIENT_TEST_H_
#define _CTK_CTKCLIENT_TEST_H_

#include <ctkclient.h>

struct TKTokenTestBlocks {
	void *createOrUpdateObject;
	void *copyObjectAccessControl;
	void *copyObjectData;
	void *copyPublicKeyData;
	void *deleteObject;
	void *copySignature;
};

typedef struct TKTokenTestBlocks TKTokenTestBlocks;

void TKTokenTestSetHook(void (^block)(CFDictionaryRef attributes, TKTokenTestBlocks *blocks));

#endif

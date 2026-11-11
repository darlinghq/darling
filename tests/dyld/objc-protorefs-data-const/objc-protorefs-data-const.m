// Copyright (C) 2026 Lubos Dolezel
//
// A program and a dylib whose __objc_protorefs sections are in a read-only
// __DATA_CONST segment (SG_READ_ONLY), both defining the protocol ProtorefProto.
// The objc runtime rewrites the duplicate's protocol references to the one
// canonical protocol while dyld reports the images as mapped, so it must be
// able to write them there. On macOS 27 this program reaches main and all
// references are equal (black-box check MC9).
//
// Without the fix it dies with SIGSEGV before main.

#include <objc/runtime.h>
#include <stdio.h>

@protocol ProtorefProto
- (int)protorefValue;
@end

Protocol* protoref_lib_proto(void);

int main(void)
{
	Protocol* mine = @protocol(ProtorefProto);
	Protocol* lib = protoref_lib_proto();
	Protocol* runtime = objc_getProtocol("ProtorefProto");

	if (mine == NULL || mine != lib || mine != runtime) {
		printf("FAIL: protocol references differ: %p %p %p\n", (void*) mine, (void*) lib, (void*) runtime);
		return 1;
	}
	printf("PASS\n");
	return 0;
}

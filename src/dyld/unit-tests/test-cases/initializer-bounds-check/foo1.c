#include <stdio.h>

extern void* initializers[] __asm__("section$start$__DATA$__mod_init_func");

extern void altSecondInit();


__attribute__((constructor))
void firstInit()
{
	// slam second initializer to be pointer into another dylib
	initializers[1] = &altSecondInit;
}


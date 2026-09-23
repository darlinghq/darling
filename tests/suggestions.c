// Guest test: Suggestions.framework loads from its install name. Activity Monitor links it without
// binding any symbol, so loading is the whole contract. Exits non-zero on failure.
#include <dlfcn.h>
#include <stdio.h>

int main(void)
{
	const char *path = "/System/Library/PrivateFrameworks/Suggestions.framework/Versions/A/Suggestions";
	void *handle = dlopen(path, RTLD_NOW);
	if (!handle) {
		fprintf(stderr, "FAIL: %s\n", dlerror());
		return 1;
	}
	printf("PASS\n");
	return 0;
}

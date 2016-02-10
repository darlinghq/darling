#include <stdio.h>
#include <libunwind.h>
#include <dlfcn.h>

extern void unw_iterate_dwarf_unwind_cache(void (*func)(unw_word_t ip_start, unw_word_t ip_end, unw_word_t fde, unw_word_t mh));



typedef void (*FooProc)();
static FooProc foo;

static bool fooInCache = false;
static void callback(unw_word_t ip_start, unw_word_t ip_end, unw_word_t fde, unw_word_t mh)
{
	//fprintf(stderr, "ip_start = 0x%llX, ip_end = 0x%llX\n", ip_start, ip_end);
	if ( (unw_word_t)(long)foo == ip_start )
		fooInCache = true;
}


int main()
{
	void* handle = dlopen("libfoo.dylib", RTLD_FIRST);
	if ( handle == NULL ) {
		printf("FAIL: dlopen could not load libfoo.dylib\n");
		return 1;
	}

	foo = (FooProc)dlsym(handle, "foo");
	if ( foo == NULL ) {
		printf("FAIL: dlsym could not find foo\n");
		return 1;
	}
	
	//fprintf(stderr, "foo=%p\n", foo);
	(*foo)();
	
	// verify foo is in cache
	fooInCache = false;
	unw_iterate_dwarf_unwind_cache(&callback);
	if ( !fooInCache ) {
		printf("FAIL: foo is not in cache\n");
		return 1;
	}
		
	dlclose(handle);
	//fprintf(stderr, "dlclose\n");
	
	// verify foo is no longer in cache
	fooInCache = false;
	unw_iterate_dwarf_unwind_cache(&callback);
	if ( fooInCache ){
		printf("FAIL: foo is still in cache\n");
		return 1;
	}
	
	return 0;
}

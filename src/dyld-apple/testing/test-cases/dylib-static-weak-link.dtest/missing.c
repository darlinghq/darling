#include <stddef.h>
#include <stdio.h>

extern int foo __attribute__((weak_import));


int main()
{
    printf("[BEGIN] dylib-static-weak-link missing\n");
    // dylib won't be found at runtime, so &foo should be NULL
    if ( &foo == NULL )
        printf("[PASS] dylib-static-weak-link missing\n");
    else
        printf("[FAIL] dylib-static-weak-link missing, &foo != NULL\n");

	return 0;
}



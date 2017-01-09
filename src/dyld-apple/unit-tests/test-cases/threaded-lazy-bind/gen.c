
#include <stdio.h>

int main()
{
	int i;
	for (i = 0; i < 1000; ++i) {
		printf("int do_%03d() { return %d; }\n", i, i);
	}

	for (i = 0; i < 1000; ++i) {
		printf("extern int do_%03d();\n", i);
	}
	for (i = 0; i < 1000; ++i) {
		printf("if ( do_%03d() != %d ) { FAIL(\"iteration %d\"); exit(0); }\n", i, i, i);
	}

	return 0;
}
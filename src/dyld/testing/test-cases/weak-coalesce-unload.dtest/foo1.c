
__attribute__((weak))
int foo = 1;

void* fooPtr() {
	return &foo;
}
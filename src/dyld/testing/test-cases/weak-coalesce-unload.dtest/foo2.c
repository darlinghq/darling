
__attribute__((weak))
int foo() {
	return 2;
}

void* fooPtr() {
	return &foo;
}



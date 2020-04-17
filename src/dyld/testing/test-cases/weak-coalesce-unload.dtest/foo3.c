
__attribute__((weak))
int foo() {
	return 3;
}

void* fooPtr() {
	return &foo;
}



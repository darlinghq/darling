

int foo_value = 1;

void __attribute__((constructor)) setup_foo()
{
	foo_value = 2;
}




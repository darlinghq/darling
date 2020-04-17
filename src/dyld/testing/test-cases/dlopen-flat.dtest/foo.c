

extern int gInitialisersCalled;

__attribute__((constructor))
static void onLoad() {
	++gInitialisersCalled;
}


int foo() {
	return 0;
}
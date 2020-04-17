
extern int foo();

extern int gInitialisersCalled;

__attribute__((constructor))
static void onLoad() {
	++gInitialisersCalled;
}

typedef int(*retTy)();

retTy bar() {
	return &foo;
}
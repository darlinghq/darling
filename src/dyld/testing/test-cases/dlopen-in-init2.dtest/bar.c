
static int inited = 0;

__attribute__((constructor))
static void myinit()
{
	inited = 1;
}

int barIsInited() {
	return inited;
}

int bar() {
	return inited ? 0 : 1;
}
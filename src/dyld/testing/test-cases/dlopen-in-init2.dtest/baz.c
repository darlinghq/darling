
static int inited = 0;

__attribute__((constructor))
static void myinit()
{
	inited = 1;
}

int bazInited() {
	return inited;
}
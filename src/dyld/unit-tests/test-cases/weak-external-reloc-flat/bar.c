

int bar[] = { 20, 21, 22, 23 };


int __attribute__((weak)) frob[] = { 30, 31, 32, 33 };

int bar_getfrob(int x) { return frob[x]; }

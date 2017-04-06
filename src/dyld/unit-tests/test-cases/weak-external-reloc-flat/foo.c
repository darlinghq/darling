

extern int bar_getfrob(int x);


int __attribute__((weak)) foo[] = { 1, 2, 3, 4 };
int __attribute__((weak)) bar[] = { 10, 11, 12, 13 };
int __attribute__((weak)) frob[] = { 20, 21, 22, 23 };

int* pfoo = &foo[2];

int getfrob() {
	return bar_getfrob(2);
}
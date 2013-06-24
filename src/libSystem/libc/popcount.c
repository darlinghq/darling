#include <stdint.h>

int __darwin___popcountdi2(int64_t a)
{
	return __builtin_popcountll(a);
}

int __darwin___popcountsi2(int a)
{
	return __builtin_popcount(a);
}




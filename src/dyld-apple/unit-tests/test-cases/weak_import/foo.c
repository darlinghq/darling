

#include "foo.h"

void func1() {}
void func3() {}
int data1 = 0;
int data3;
int data5 = 0;

#if ALL_SYMBOLS
void func2() {}
void func4() {}

int data2 = 0;	// weak_import initialized
int data4;		// weak_import uninitialized
int data6 = 0;	// weak_import 
#endif


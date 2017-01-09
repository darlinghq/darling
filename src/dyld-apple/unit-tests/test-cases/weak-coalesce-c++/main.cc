#include <stdio.h>
#include <stdlib.h> // exit(), EXIT_SUCCESS

#include "test.h" // PASS(), FAIL(), XPASS(), XFAIL()

extern void set_x_1(int);
extern void set_x_2(int);
extern int  get_x_1(void);
extern int  get_x_2(void);

int main()
{
	set_x_1 (17);
	set_x_2 (76);

	if (get_x_1() == 76 && get_x_2() == 76)
		PASS("weak-coalesce=c++");
	else
		FAIL("weak-coalesce=c++");
}

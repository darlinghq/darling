// CFLAGS: -lncurses
#include <curses.h>

int main()
{
	int v;
	initscr();
	v = acs_map['x'];
	endwin();
	printf("%d\n", v);
	return 0;
}


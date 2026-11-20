// CFLAGS: -lncurses
// Copyright (C) 2026 Lubos Dolezel
//
// The terminfo database has an entry for tmux-256color (darlinghq/darling#2335:
// without it, line editors fall back to wrong cursor motions inside tmux), and its
// cursor and erase capabilities load through setupterm(3)/tigetstr(3).
//
// Expected values were observed with infocmp on macOS 26.7.3 (ncurses 6.0.20150808).
// macOS 10.14.4 (ncurses 5.7) has no tmux entries, so the test fails there.
#include <curses.h>
#include <fcntl.h>
#include <stdio.h>
#include <string.h>
#include <term.h>
#include <unistd.h>

static int failures;

static void check(const char *name, int ok)
{
	printf("%s: %s\n", ok ? "PASS" : "FAIL", name);
	failures += !ok;
}

static void check_str(const char *cap, const char *expected)
{
	char *value = tigetstr((char *) cap);
	char name[64];

	snprintf(name, sizeof(name), "tmux-256color %s", cap);
	check(name, value != NULL && value != (char *) -1 && strcmp(value, expected) == 0);
}

int main(void)
{
	int err = 0;
	int fd = open("/dev/null", O_WRONLY);

	if (setupterm("tmux-256color", fd, &err) != OK) {
		check("setupterm(tmux-256color)", 0);
		printf("(setupterm error %d)\n", err);
		return 1;
	}
	check("setupterm(tmux-256color)", err == 1);

	check_str("cub1", "\b");
	check_str("kbs", "\b");
	check_str("el", "\033[K");
	check_str("cup", "\033[%i%p1%d;%p2%dH");

	char *cup = tigetstr("cup");
	if (cup != NULL && cup != (char *) -1) {
		const char *moved = tparm(cup, 4, 9, 0, 0, 0, 0, 0, 0, 0);
		check("tmux-256color cup(4,9)", moved != NULL && strcmp(moved, "\033[5;10H") == 0);
	}

	check("tmux-256color colors", tigetnum("colors") == 256);

	close(fd);
	return failures ? 1 : 0;
}

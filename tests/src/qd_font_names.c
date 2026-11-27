// Copyright (C) 2026 Lubos Dolezel
//
// QuickDraw font family names (32-bit only, as in macOS): GetFontName gives the name of a font family number as a
// Pascal string, the inverse of GetFNum for the families of the system; a number that is no family has the empty
// name. (Inside Macintosh: Text, "Font Manager".)
#include <Carbon/Carbon.h>
#include <stdio.h>
#include <string.h>

#ifdef __LP64__
int main(void)
{
	printf("PASS qd_font_names (no 64-bit API)\n");
	return 0;
}
#else

static int g_failures;
#define CHECK(cond) do { if (!(cond)) { printf("FAIL line %d: %s\n", __LINE__, #cond); g_failures++; } } while (0)

static int is(ConstStr255Param string, const char *text)
{
	return string[0] == strlen(text) && memcmp(string + 1, text, string[0]) == 0;
}

int main(void)
{
	Str255 name;
	short id = -1;
	GetFNum("\pHelvetica", &id);
	CHECK(id != 0);
	GetFontName(id, name);
	CHECK(is(name, "Helvetica"));
	GetFNum("\pMonaco", &id);
	GetFontName(id, name);
	CHECK(is(name, "Monaco"));
	GetFNum("\pGeneva", &id);
	GetFontName(id, name);
	CHECK(is(name, "Geneva"));
	// the system font family is number 0, Chicago
	GetFontName(0, name);
	CHECK(is(name, "Chicago"));
	// a family that does not exist has no name; the buffer is not left as it was
	memset(name, 0x55, sizeof(name));
	GetFontName(9999, name);
	CHECK(name[0] == 0);
	GetFontName(21, NULL);
	printf(g_failures ? "FAIL qd_font_names\n" : "PASS qd_font_names\n");
	return g_failures != 0;
}
#endif

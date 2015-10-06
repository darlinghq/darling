
extern int bar(const char** list);

int foo(const char** list)
{
	return bar(list);
}

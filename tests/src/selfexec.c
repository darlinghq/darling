#include <unistd.h>
#include <stdio.h>

int main(int argc, char** argv)
{
	if (argc == 1)
	{
		execl(argv[0], argv[0], "Hello world", 0);
		return 1;
	}
	else
	{
		puts(argv[1]);
		return 0;
	}
}



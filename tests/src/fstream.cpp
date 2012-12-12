// CFLAGS: -O3
#include <fstream>
#include <iostream>
#include <unistd.h>
#include <cstring>
#include <cstdio>
#include <ext/stdio_filebuf.h>

void printFile(const char* path);
void test1();
void test2();
void test3();

char text[8001] = "";
const char* file = "/tmp/__test123";

int main()
{
	//__gnu_cxx::stdio_filebuf<char> out;
	const char* file = "/tmp/__test123";

	for (int i = 0; i < 800; i++)
		memcpy(text+i*10, "1234567890", 11);
	
	test1();
	test2();
	test3();

	return 0;
}

void test1()
{
	__gnu_cxx::stdio_filebuf<char> out(fopen(file, "w+b"), std::ios_base::out|std::ios_base::binary);
	out.sputn(text, strlen(text));
	out.close();

	printFile(file);
}

void test2()
{
	std::filebuf out2;
	size_t len;
	out2.open(file, std::ios_base::binary|std::ios_base::in|std::ios_base::out|std::ios_base::trunc);
	len = strlen(text);
	printf("Len %d\n", len);
	out2.sputn(text, len);
	out2.close();

	printFile(file);
}

void test3()
{
	std::ofstream of(file, std::ios_base::binary|std::ios_base::in|std::ios_base::out|std::ios_base::trunc);
	of.write(text, strlen(text));
	of.close();

	printFile(file);
}

void printFile(const char* path)
{
	std::ifstream in;
	char buf[8001];

	in.open(path);
	in.getline(buf, sizeof(buf));
	in.close();

	std::cout << buf << std::endl;
	unlink(path);
}


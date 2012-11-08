#include <fstream>
#include <iostream>
#include <unistd.h>
#include <cstring>
#include <cstdio>
#include <ext/stdio_filebuf.h>

int main()
{
	//__gnu_cxx::stdio_filebuf<char> out;
	std::ifstream in;
	const char* text = "Hello world";
	const char* file = "/tmp/__test123";
	char buf[100];
	
	//out.open(file, std::ios_base::out | std::ios_base::binary);
	__gnu_cxx::stdio_filebuf<char> out(fopen(file, "w+b"), std::ios_base::out|std::ios_base::binary);
	out.sputn(text, strlen(text));
	out.close();

	in.open(file);
	in.getline(buf, 100);
	in.close();

	std::cout << "sizeof: " << sizeof(out) << std::endl;
	std::cout << buf << std::endl;
	unlink(file);

	return 0;
}


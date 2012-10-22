#ifndef ISTORAGE_H
#define ISTORAGE_H
#include <vector>
#include <string>
#include <stdint.h>

class IStream;

struct FileEntry
{
	std::string name;
	int64_t size;
	unsigned char d_type; // see readdir()
};

class IStorage
{
public:
	virtual ~IStorage() = 0;
	virtual IStream* open(const char* path) = 0;
	virtual std::vector<FileEntry> list() = 0;
};

#endif


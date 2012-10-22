#ifndef ISTREAM_H
#define ISTREAM_H
#include <stdint.h>
#include <sys/types.h>
#include <unistd.h>

class IStream
{
public:
	virtual ~IStream() = 0;
	virtual ssize_t read(void* buffer, ssize_t bytes) = 0;
	virtual int64_t seek(int64_t offset, int whence) = 0;
	virtual int64_t length() const = 0;
};

#endif


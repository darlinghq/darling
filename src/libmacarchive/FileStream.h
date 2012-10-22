#ifndef FILESTREAM_H
#define FILESTREAM_H
#include "IStream.h"

class FileStream : public IStream
{
private:
	FileStream(int fd);
public:
	static IStream* open(const char* path);
	virtual ~FileStream();
	virtual ssize_t read(void* buffer, ssize_t bytes) override;
    virtual int64_t seek(int64_t offset, int whence) override;
    virtual int64_t length() const override;
private:
	int m_fd;
	int64_t m_length;
};

#endif


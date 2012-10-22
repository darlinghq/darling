#include "FileStream.h"
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>

FileStream::FileStream(int fd)
	: m_fd(fd)
{
	m_length = lseek(fd, 0, SEEK_END);
	lseek(fd, 0, SEEK_SET);
}

IStream* FileStream::open(const char* path)
{
	int fd = ::open(path, O_RDONLY);
	if (fd < 0)
		return nullptr;
	return new FileStream(fd);
}

FileStream::~FileStream()
{
	close(m_fd);
}

ssize_t FileStream::read(void* buffer, ssize_t bytes)
{
	return ::read(m_fd, buffer, bytes);
}

int64_t FileStream::seek(int64_t offset, int whence)
{
	return lseek(m_fd, offset, whence);
}

int64_t FileStream::length() const
{
	return m_length;
}


#include "DMGStream.h"

IStream* DMGStream::open(IStream* stream)
{
	DMGStream* o = new DMGStream(stream);
	stream->seek(-512, SEEK_END);
	stream->read(&o->m_koly, 512);
	
	if (o->m_koly.Signature.msb() != 'koly')
	{
		delete o;
		return nullptr;
	}
	else
		return o;
}

DMGStream::DMGStream(IStream* stream)
	: m_stream(stream)
{
	static_assert(sizeof(m_koly) == 512, "Invalid kolyblk declaration");
	
	// TODO: process the headers
}

DMGStream::~DMGStream()
{
}

ssize_t DMGStream::read(void* buffer, ssize_t bytes)
{
}

int64_t DMGStream::seek(int64_t offset, int whence)
{
}

int64_t DMGStream::length() const
{
}


#ifndef MACBINARYSTREAM_H
#define MACBINARYSTREAM_H
#include "../macbinary/macbinary.h"
#include "IStream.h"

class MacBinaryStream : public IStream
{
private:
	MacBinaryStream(IStream* stream);
public:
	static IStream* open(IStream* stream);

	~MacBinaryStream();
	virtual ssize_t read(void* buffer, ssize_t bytes) override;
	virtual int64_t seek(int64_t offset, int whence) override;
	virtual int64_t length() const override;
private:
	IStream* m_stream;
	MacBinaryHeader m_header;
	int64_t m_pos, m_dataOffset;
};

#endif


#include "MacBinaryStream.h"

MacBinaryStream::MacBinaryStream(IStream* stream)
	: m_stream(stream), m_pos(0)
{
	m_stream->read(&m_header, sizeof(m_header));

	uint32_t extraLen = 0;

	if (m_header.signature == 'mBIN')
		extraLen = m_header.sec_header_len;

	m_dataOffset = 128 + extraLen;
	m_stream->seek(m_dataOffset, SEEK_SET);
}

IStream* MacBinaryStream::open(IStream* stream)
{
	return new MacBinaryStream(stream);
}

MacBinaryStream::~MacBinaryStream()
{
}

ssize_t MacBinaryStream::read(void* buffer, ssize_t bytes)
{
	if (m_pos + bytes > m_header.data_len)
		bytes = m_header.data_len - m_pos;

	bytes = m_stream->read(buffer, bytes);

	if (bytes > 0)
		m_pos += bytes;

	return bytes;
}

int64_t MacBinaryStream::seek(int64_t offset, int whence)
{
	if (whence == SEEK_SET)
		m_pos = offset;
	else if (whence == SEEK_CUR)
		m_pos += offset;
	else if (whence == SEEK_END)
		m_pos = m_header.data_len - offset;

	if (m_pos > m_header.data_len)
		m_pos = m_header.data_len;
	if (m_pos < 0)
		m_pos = 0;

	m_stream->seek(m_pos, SEEK_SET);
	return m_pos;
}

int64_t MacBinaryStream::length() const
{
	return m_header.data_len;
}


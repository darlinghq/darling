#ifndef MSB_H
#define MSB_H
#ifndef _BSD_SOURCE
#	define _BSD_SOURCE
#endif
#include <endian.h>
#include <stdint.h>

template<typename T>
class MSB
{
protected:
	static uint16_t convert(uint16_t v) { return be16toh(v); }
	static uint32_t convert(uint32_t v) { return be32toh(v); }
	static uint64_t convert(uint64_t v) { return be64toh(v); }
	
public:
	operator T() const { return convert(m_data); }
	T msb() const { return m_data; }
private:
	T m_data;
};

#endif

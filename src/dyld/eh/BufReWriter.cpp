#include "BufReWriter.h"
#include <cassert>
#include <stdexcept>

BufReWriter::BufReWriter(void* mem, uintptr_t length)
: BufReader(mem, length), m_pLastULEB128(nullptr), m_pLast8(nullptr)
{
}
	
uint64_t BufReWriter::readULEB128()
{
	m_pLastULEB128 = (uint8_t*) m_pPos;
	return BufReader::readULEB128();
}

uint8_t BufReWriter::read()
{
	m_pLast8 = (uint8_t*) m_pPos;
	return BufReader::read();
}

void BufReWriter::updateLastULEB128(uint64_t v)
{
	assert(m_pLastULEB128 != nullptr);
	
	uint8_t* pos = m_pLastULEB128;
	
	do
	{
		uint8_t byte = v & 0x7f;
		v >>= 7;
		if (v != 0)
			byte |= 0x80;
		
		if (!(*pos & 0x80) && v != 0)
			std::runtime_error("New ULEB128 value didn't fit into the space of the previous value");
		
		*pos = byte;
		pos++;
	}
	while (v != 0);
}

void BufReWriter::updateLast(uint8_t value)
{
	assert(m_pLast8 != nullptr);
	*m_pLast8 = value;
}

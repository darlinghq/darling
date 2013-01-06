#ifndef BUFREWRITER_H
#define BUFREWRITER_H
#include "BufReader.h"

// only designed to rewrite ULEB128 values!
class BufReWriter : public BufReader
{
public:
	BufReWriter(void* mem, uintptr_t length);
	
	uint8_t read();
	uint64_t readULEB128();
	void updateLastULEB128(uint64_t value);
	void updateLast(uint8_t value);
private:
	uint8_t *m_pLastULEB128, *m_pLast8;
};

#endif

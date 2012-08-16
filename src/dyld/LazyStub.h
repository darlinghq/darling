#ifndef LAZUSTUB_H
#define LAZUSTUB_H
#include <stdint.h>
#include <map>
#include "Mutex.h"

#pragma pack(1)
struct LazyStub
{
	char mov; // b9 (ecx)
	uint32_t index;
	char jmp; // e9
	uint32_t loader;

	void init(uint32_t index, uint32_t loader)
	{
		mov = 0xb9;
		this->index = index;
		jmp = 0xe9;
		this->loader = loader;
	}
};
#pragma pack()

class LazyStubMgr
{
public:
	LazyStubMgr(void* (*lazyResolver)(void* /*opaque*/), int minStubs = 3000);
	~LazyStubMgr();
	
	void* createStub(void* opaque);
	static void* lazystub_resolve(uint32_t index) asm("lazystub_resolve");
private:
	Darling::Mutex m_mutex;
	LazyStub* m_pMem;
	int m_nMax, m_nNext, m_nBytes;
	void** m_pOpaques;
};

#endif

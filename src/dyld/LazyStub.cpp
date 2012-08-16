#include "LazyStub.h"
#include <unistd.h>
#include <sys/mman.h>
#include <stdexcept>
#include <cassert>

extern "C" void lazystub_loader();

LazyStubMgr::LazyStubMgr(void* (*lazyResolver)(void* /*opaque*/), int minStubs)
{
	int ps = getpagesize();
    void* mem;
    
    int bytes = entries * sizeof(LazyStub);
    bytes = (bytes + ps - 1) / ps * ps;
    
    mem = ::mmap(0, bytes, PROT_EXEC | PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, 0, 0);
    if (mem == MAP_FAILED)
        throw std::runtime_error("Failed to map pages for LazyStub");
    
    m_pMem = static_cast<LazyStub*>(mem);
    m_nMax = bytes / sizeof(LazyStub);
    m_nBytes = bytes;

	m_pOpaques = new void*[m_nMax];
}

LazyStubMgr::~LazyStubMgr()
{
	::munmap(m_pMem, m_nBytes);
	delete [] m_pOpaques;
}

void* LazyStubMgr::createStub(void* opaque)
{
	MutexLock l(m_mutex);
	
	if (m_nNext >= m_nMax)
        throw std::runtime_error("LazyStub buffer full");
    
    m_pMem[m_nNext].init(m_nNext, (uint32_t)(long)lazystub_loader);
	m_pOpaques[m_nNext] = opaque;
    return &m_pMem[m_nNext++];
}

void* LazyStubMgr::lazystub_resolve(uint32_t index)
{
}


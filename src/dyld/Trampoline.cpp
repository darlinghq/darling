#include "Trampoline.h"
#include <unistd.h>
#include <sys/mman.h>
#include <stdexcept>
#include <cassert>
#include <cstring>
#include <iostream>
#include <fstream>

TrampolineMgr* TrampolineMgr::m_pInstance = 0;

TrampolineMgr::TrampolineMgr(int entries)
	: m_nNext(0)
{
	assert(m_pInstance == 0);
	m_pInstance = this;

	int ps = getpagesize();
	void* mem;

	int bytes = entries * sizeof(Trampoline);
	bytes = (bytes + ps - 1) / ps * ps;

	mem = ::mmap(0, bytes, PROT_EXEC | PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, 0, 0);
	if (mem == MAP_FAILED)
	throw std::runtime_error("Failed to map pages for TrampolineMgr");

	m_pMem = static_cast<Trampoline*>(mem);
	m_nMax = bytes / sizeof(Trampoline);
	m_nBytes = bytes;

}

TrampolineMgr::~TrampolineMgr()
{
	::munmap(m_pMem, m_nBytes);
}

void* TrampolineMgr::generate(void* targetAddr, const char* name)
{
	if ((targetAddr > m_pMem && targetAddr < m_pMem+m_nMax) || !isExecutable(targetAddr))
		return targetAddr; // will not create a trampoline for a trampoline

	AddrEntry e = { name, targetAddr };
	if (m_nNext >= m_nMax)
		throw std::runtime_error("TrampolineMgr buffer full");

	m_entries.push_back(e);
	m_pMem[m_nNext].init(m_nNext, TrampolineMgr::printInfo);
	
	void* addr = &m_pMem[m_nNext++];
	// std::cout << "Trampoline for " << name << " is at " << addr << std::endl;
	return addr;
}

bool TrampolineMgr::isExecutable(void* addr)
{
	if (m_memoryMap.empty())
		loadMemoryMap();
	
	for (auto it = m_memoryMap.begin(); it != m_memoryMap.end(); it++)
	{
		if (addr >= it->start && addr < it->end)
			return it->executable;
	}
	
	return false;
}

void TrampolineMgr::invalidateMemoryMap()
{
	m_memoryMap.clear();
}

void TrampolineMgr::loadMemoryMap()
{
	std::ifstream file("/proc/self/maps");
	std::string line;
	
	while (std::getline(file, line))
	{
		if (line.empty()) continue;
		const char* s = line.c_str();
		MemoryPages pages;
		
		pages.start = (void*) strtol(s, (char**) &s, 16);
		s++;
		pages.end = (void*) strtol(s, (char**) &s, 16);
		pages.executable = *(s+3) == 'x';
		
		// std::cout << line << " -> " << pages.start << " - " << pages.end << " " << pages.executable << std::endl;
		m_memoryMap.push_back(pages);
	}
}

void* TrampolineMgr::printInfo(uint32_t index)
{
	std::cerr << m_pInstance->m_entries[index].name << "() called\n" << std::flush;
	return m_pInstance->m_entries[index].addr;
}

void Trampoline::init(uint32_t i, void* (*pDebug)(uint32_t))
{
	// See /tools/trampoline.asm for source
	
	memcpy(code1, "\x50\x53\x57\x56\x52\x51\x41\x50\x41\x51\x48\x81\xec\x80\x00\x00\x00\xf3\x0f\x7f"
			"\x04\x24\xf3\x0f\x7f\x4c\x24\x10\xf3\x0f\x7f\x54\x24\x20\xf3\x0f"
			"\x7f\x5c\x24\x30\xf3\x0f\x7f\x64\x24\x40\xf3\x0f\x7f\x6c\x24\x50"
			"\xf3\x0f\x7f\x74\x24\x60\xf3\x0f\x7f\x7c\x24\x70\xbf", sizeof(code1));
	index = i;
	code2[0] = 0x48;
	code2[1] = 0xba;
	debugFcn = uint64_t(pDebug);
	
	memcpy(code3, "\xff\xd2\x49\x89\xc3"
			"\xf3\x0f\x6f\x7c\x24\x70\xf3\x0f\x6f\x74\x24\x60\xf3\x0f\x6f\x6c"
			"\x24\x50\xf3\x0f\x6f\x64\x24\x40\xf3\x0f\x6f\x5c\x24\x30\xf3\x0f"
			"\x6f\x54\x24\x20\xf3\x0f\x6f\x4c\x24\x10\xf3\x0f\x6f\x04\x24\x48"
			"\x81\xc4\x80\x00\x00\x00\x41\x59\x41\x58\x59\x5a\x5e\x5f\x5b\x58\x41\xff\xe3", sizeof(code3));
}


#ifdef TEST

double mytestfunc(int a, int b, double c)
{
	return a*b+c;
}

int main()
{
	TrampolineMgr* mgr = new TrampolineMgr;
	
	double (*pFunc)(int,int,double) = (double (*)(int,int,double)) mgr->generate((void*) &mytestfunc, "mytestfunc");
	std::cout << pFunc(2,3,0.5) << std::endl;
	
	delete mgr;
	return 0;
}

#endif

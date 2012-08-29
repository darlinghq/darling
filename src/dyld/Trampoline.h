#ifndef TRAMPOLINE_H
#define TRAMPOLINE_H
#include <stdint.h>
#include <vector>
#include <string>
#include <list>

struct Trampoline;

class TrampolineMgr
{
public:
	TrampolineMgr(int minTrampolines = 4096);
	~TrampolineMgr();

	void* generate(void* targetAddr, const char* name);
	bool isExecutable(void* targetAddr);
	void invalidateMemoryMap();
private:
	void loadMemoryMap();
	static void* printInfo(uint32_t index);
private:
	static TrampolineMgr* m_pInstance;

	Trampoline* m_pMem;
    int m_nMax, m_nNext, m_nBytes;

	struct AddrEntry
	{
		std::string name;
		void* addr;
	};
	struct MemoryPages
	{
		void* start;
		void* end;
		bool executable;
		
		bool operator<(const MemoryPages& o) { return start < o.start; }
		bool operator<(void* p) { return start < p; }
	};

	std::vector<AddrEntry> m_entries;
	std::list<MemoryPages> m_memoryMap;
};

#pragma pack(1)
struct Trampoline
{
	void init(uint32_t index, void* (*pDebug)(uint32_t));

	char code1[65];
	uint32_t index;
	char code2[2];
	uint64_t debugFcn;
	char code3[72];
	char padding[1];
};
#pragma pack()

#endif


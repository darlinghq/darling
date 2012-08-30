#ifndef TRAMPOLINE_H
#define TRAMPOLINE_H
#include <stdint.h>
#include <vector>
#include <string>
#include <list>
#include <map>

struct Trampoline;

class TrampolineMgr
{
public:
	TrampolineMgr(int minTrampolines = 4096);
	~TrampolineMgr();

	void* generate(void* targetAddr, const char* name);
	bool isExecutable(void* targetAddr);
	void invalidateMemoryMap();
	
	static void loadFunctionInfo(const char* path);
	
	#pragma pack(1)
	struct CallStack
	{
		long double xmm[8]; // xmm7-xmm0
		uint64_t r9, r8, rcx, rdx, rsi, rdi, rbx, rax;
		void* retAddr;
	};
	#pragma pack()
private:
	void loadMemoryMap();
	
	static void* printInfo(uint32_t index, CallStack* stack);
	static void* printInfoR(uint32_t index, CallStack* stack);
private:
	struct AddrEntry
	{
		std::string name;
		void* addr;
		void* retAddr; // not reentrant
	};
	struct MemoryPages
	{
		void* start;
		void* end;
		bool executable;
		
		bool operator<(const MemoryPages& o) { return start < o.start; }
		bool operator<(void* p) { return start < p; }
	};
	struct FunctionInfo
	{
		char retType;
		std::string arguments;
	};
	class ArgumentWalker
	{
	public:
		ArgumentWalker(CallStack* stack);
		uint64_t next64bit();
		long double nextDouble();
		std::string next(char type);
		std::string ret(char type);
	private:
		CallStack* m_stack;
		int m_indexInt, m_indexXmm;
	};
	
	static TrampolineMgr* m_pInstance;

	Trampoline* m_pMem;
    int m_nMax, m_nNext;

	std::vector<AddrEntry> m_entries;
	std::list<MemoryPages> m_memoryMap;
	static int m_nDepth;
	static std::map<std::string, FunctionInfo> m_functionInfo;
};

#pragma pack(1)
struct Trampoline // 119 bytes // 63 code // 56 pointers
{
	void init(uint32_t index, void* (*pDebug)(uint32_t,TrampolineMgr::CallStack*), void* (*pDebugR)(uint32_t,TrampolineMgr::CallStack*));

	char code1[2];
	uint64_t reg_saveall;
	char code2[4];
	uint32_t index;
	char code3[5];
	uint64_t debugFcn;
	char code4[7];
	uint64_t reg_restoreall;
	char code5[20];
	uint64_t reg_saveall2;
	char code6[4];
	uint32_t index2;
	char code7[5];
	uint64_t debugFcnR;
	char code8[7];
	uint64_t reg_restoreall2;
	char code9[9];
	char padding[1];
};
#pragma pack()

#endif


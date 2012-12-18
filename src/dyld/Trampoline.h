/*
This file is part of Darling.

Copyright (C) 2012 Lubos Dolezel

Darling is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

Darling is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with Darling.  If not, see <http://www.gnu.org/licenses/>.
*/

#ifndef TRAMPOLINE_H
#define TRAMPOLINE_H
#include <stdint.h>
#include <vector>
#include <string>
#include <list>
#include <map>
#include <fstream>
#include <sys/time.h>

struct Trampoline;

class TrampolineMgr
{
public:
	TrampolineMgr(int minTrampolines = 4096);
	~TrampolineMgr();

	void* generate(void* targetAddr, const char* name);
	bool isExecutable(void* targetAddr);
	std::string inFile(void* addr);
	void invalidateMemoryMap();
	
	static void loadFunctionInfo(const char* path);
	
#pragma pack(1)
#ifdef __x86_64__
	struct CallStack
	{
		long double xmm[8]; // xmm7-xmm0
		uint64_t r15, r14, r13, r12, r9, r8, rcx, rdx, rsi, rdi, rbx, rax;
		void* retAddr;
		uint64_t moreArguments[];
	};
#else
	struct CallStack
	{
		double st0;
		uint32_t edi, esi, edx, ecx, ebx, eax;
		void* retAddr;
		uint32_t arguments[];
	};
#endif
#pragma pack()
private:
	void loadMemoryMap();
	
	std::string logPath();
	std::ostream* getLogger();
	static std::string timeStamp();
	static std::string callTime();
	static std::string formatTime(double ms);
	static void* printInfo(uint32_t index, CallStack* stack);
	static void* printInfoR(uint32_t index, CallStack* stack);
	static bool loadObjCHelper();
public:
	typedef std::vector<std::pair<char,void*> > OutputArguments;
	
private:
	struct AddrEntry
	{
		std::string name, printName;
		void* addr;
	};
	struct MemoryPages
	{
		void* start;
		void* end;
		bool executable;
		std::string file;
		
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
		ArgumentWalker(CallStack* stack, OutputArguments args /* for printInfoR only */);
#ifdef __x86_64__
		uint64_t next64bit();
		long double nextLongDouble();
#endif
		int nextInt();
		long long nextLL();
		float nextFloat();
		double nextDouble();
		void* nextPointer();
		std::string next(char type);
		std::string ret(char type);
		OutputArguments getOutputArguments() { return m_pointers; }
	private:
		static std::string safeString(const char* in);
	private:
		CallStack* m_stack;
#ifdef __x86_64__
		int m_indexInt, m_indexXmm;
#else
		int m_indexArg;
#endif
		OutputArguments m_pointers;
	};
public:
	struct ReturnInfo
	{
		void* retAddr;
		OutputArguments pointers;
		struct timeval callTime;
		std::map<std::string, FunctionInfo>::iterator it;
	};
private:
	
	static TrampolineMgr* m_pInstance;

	Trampoline* m_pMem;
    int m_nMax, m_nNext;

	std::vector<AddrEntry> m_entries;
	std::list<MemoryPages> m_memoryMap;
	std::string m_wd;
	static struct timeval m_startup;
	static std::map<std::string, FunctionInfo> m_functionInfo;
};

#pragma pack(1)
struct Trampoline
{
	void init(uint32_t index, void* (*pDebug)(uint32_t,TrampolineMgr::CallStack*), void* (*pDebugR)(uint32_t,TrampolineMgr::CallStack*));

#ifdef __x86_64__

	// 119 bytes // 63 code // 56 pointers
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
#else
	// 62+4*8=94 bytes
	char code1[2];
	uint32_t reg_saveall;
	char code2[4];
	uint32_t index;
	char code3;
	uint32_t debugFcn;
	char code4[10];
	uint32_t reg_restoreall;
	char code5[22];
	uint32_t reg_saveall2;
	char code6[4];
	uint32_t index2;
	char code7;
	uint32_t debugFcnR;
	char code8[10];
	uint32_t reg_restoreall2;
	char code9[8];
	//char padding[1];

#endif
};
#pragma pack()

#endif


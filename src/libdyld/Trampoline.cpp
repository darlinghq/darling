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

#include "Trampoline.h"
#include <unistd.h>
#include <sys/mman.h>
#include <stdexcept>
#include <cassert>
#include <cstring>
#include <iostream>
#include <fstream>
#include <sstream>
#include <stack>
#include <cxxabi.h>
#include <dlfcn.h>
#include <sys/syscall.h>
#include "../util/log.h"
#include "../util/stlutils.h"

TrampolineMgr* TrampolineMgr::m_pInstance = 0;
std::map<std::string, TrampolineMgr::FunctionInfo> TrampolineMgr::m_functionInfo;
struct timeval TrampolineMgr::m_startup;

static __thread std::stack<TrampolineMgr::ReturnInfo>* g_returnInfo = 0;
static __thread std::ofstream* g_logger = 0;
static __thread pid_t g_loggerForPid = 0;
static const char* LIB_OBJCDARWIN = "libobjc.A.dylib.so";
static std::string (*objc_helper)(const std::string& /*invoker*/, void* /*arg1*/, void* /*arg2*/, std::string& /*searchable*/) = 0;

extern "C" void reg_saveall();
extern "C" void reg_restoreall();
extern "C" void* trampoline_start;
extern "C" void* trampoline_end;

#ifndef TEST
#	include "MachOMgr.h"
#	include "MachOObject.h"
#else
const char* g_argv[] = { "Trampoline", 0 };
#endif

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
	
	if (char* p = get_current_dir_name())
	{
		m_wd = p;
		free(p);
	}

	::gettimeofday(&m_startup, nullptr);
}

void TrampolineMgr::printPath()
{
	std::cout << logPath() << std::endl;
}

TrampolineMgr::~TrampolineMgr()
{
	::munmap(m_pMem, m_nMax * sizeof(Trampoline));
}

void* TrampolineMgr::generate(void* targetAddr, const char* name)
{
	if ((targetAddr > m_pMem && targetAddr < m_pMem+m_nMax) || !isExecutable(targetAddr))
		return targetAddr; // will not create a trampoline for a trampoline

	AddrEntry e = { name, name, targetAddr };
	if (m_nNext >= m_nMax)
		throw std::runtime_error("TrampolineMgr buffer full");
	
	//std::cout << e.name << std::endl;
	if (e.name.compare(0, 2, "_Z") == 0)
	{
		int status;
		char* n = abi::__cxa_demangle(e.name.c_str(), 0, 0, &status);
		
		//std::cout << n << std::endl;
		
		if (n)
		{
			e.printName = n;
			e.printName += " - ";
			e.printName += e.name;
			free(n);
		}
	}

	m_entries.push_back(e);
	m_pMem[m_nNext].init(m_nNext, TrampolineMgr::printInfo, TrampolineMgr::printInfoR);
	
	void* addr = &m_pMem[m_nNext++];
	// std::cout << "Trampoline for " << name << " is at " << addr << std::endl;
	return addr;
}

bool TrampolineMgr::isExecutable(void* addr)
{
	if (m_memoryMap.empty())
		loadMemoryMap();
		
	// std::cout << "isExecutable(): " << addr << std::endl;
	
	for (auto it = m_memoryMap.begin(); it != m_memoryMap.end(); it++)
	{
		if (addr >= it->start && addr < it->end)
			return it->executable;
	}
	
	return false;
}

std::string TrampolineMgr::inFile(void* addr)
{
	for (auto it = m_memoryMap.begin(); it != m_memoryMap.end(); it++)
	{
		if (addr >= it->start && addr < it->end)
			return it->file;
	}
	return "?";
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
		pages.executable = (*(s+3) == 'x') && (*(s+2) == '-'); // not writable
		
		if (line.size() > 74)
			pages.file = line.substr(73);
		
		//std::cout << line << " -> " << pages.start << " - " << pages.end << " " << pages.executable << std::endl;
		m_memoryMap.push_back(pages);
	}
}

void TrampolineMgr::loadFunctionInfo(const char* path)
{
	std::ifstream file(path);
	std::string line;
	
	while (std::getline(file, line))
	{
		size_t p = line.rfind(':');
		if (p == std::string::npos)
			continue;
		
		FunctionInfo info;
		info.retType = line.at(p+1);
		info.arguments = line.substr(p+2);
		
		m_functionInfo[line.substr(0, p)] = info;
	}
}

std::string TrampolineMgr::formatTime(double ms)
{
	const double HRS = 1000.0*60*60;
	const double MINS = 1000.0*60;
	const double SECS = 1000.0;
	std::stringstream ss;

	if (ms >= HRS)
	{
		int hrs = int(ms / HRS);
		ss << hrs << 'h';
		ms -= double(hrs) * HRS;
	}
	if (ms >= MINS)
	{
		int mins = int(ms / MINS);
		ss << mins << 'm';
		ms -= double(mins) * MINS;
	}
	if (ms >= SECS)
	{
		int secs = int(ms / SECS);
		ss << secs << 's';
		ms -= double(secs) * SECS;
	}
	ss << int(ms) /*<< "ms"*/; // double value here sometime causes crashes, why?
	ss << '.' << int(ms*10)%10 << "ms"; // workaround
	return ss.str();
}

std::string TrampolineMgr::callTime()
{
	double ms;
	struct timeval now;

	::gettimeofday(&now, nullptr);
	ms = (now.tv_sec - g_returnInfo->top().callTime.tv_sec) * 1000;
	ms += (now.tv_usec - g_returnInfo->top().callTime.tv_usec) / 1000.0;

	return formatTime(ms);
}

std::string TrampolineMgr::timeStamp()
{
	double ms;
	struct timeval now;

	::gettimeofday(&now, nullptr);
	ms = (now.tv_sec - m_pInstance->m_startup.tv_sec) * 1000;
	ms += (now.tv_usec - m_pInstance->m_startup.tv_usec) / 1000.0;

	return formatTime(ms);
}

std::string TrampolineMgr::logPath()
{
	pid_t pid = syscall(SYS_gettid);
	std::stringstream ss;
	const char* progname = Darling::MachOMgr::instance()->mainModule()->name();
	//size_t pos;

	//pos = progname.rfind('/');
	//if (pos != std::string::npos)
	//	progname = progname.substr(pos+1);

	ss << m_wd << '/' << progname << '.' << pid << ".log";
	return ss.str();
}

std::ostream* TrampolineMgr::getLogger()
{
	if (g_loggerForPid == syscall(SYS_gettid))
		return g_logger ? g_logger : &std::cerr;
	else
	{
		delete g_logger;
		g_logger = new std::ofstream(logPath(), std::ios_base::out | std::ios_base::app);

		if (!g_logger->is_open())
		{
			delete g_logger;
			g_logger = 0;
		}
		g_loggerForPid = getpid();
		return g_logger;
	}
}

bool TrampolineMgr::loadObjCHelper()
{
	*((void**)&objc_helper) = ::dlsym(RTLD_DEFAULT, "trampoline_objcMsgInfo");
	return objc_helper != 0;
}

void* TrampolineMgr::printInfo(uint32_t index, CallStack* stack)
{
	const AddrEntry& e = m_pInstance->m_entries[index];
	FunctionInfo* info = 0;
	std::map<std::string, FunctionInfo>::iterator it;
	std::ostream* out;
	ReturnInfo retInfo;
	ArgumentWalker w(stack);
	std::string searchable = e.name;

	out = m_pInstance->getLogger();
	
	if (!g_returnInfo)
		g_returnInfo = new std::stack<TrampolineMgr::ReturnInfo>;
		
	std::string stamp = timeStamp();
	
	(*out) << std::string(20 - stamp.size(), ' ');
	(*out) << '[' << timeStamp() << "] ";
	(*out) << std::string(g_returnInfo->size(), ' ');
	
	// Special handling for Objective-C
	if (string_startsWith(e.name, "objc_msgSend"))
	{
		if (!objc_helper)
		{
			if (!loadObjCHelper())
				goto no_objc;
		}
		
		if (string_endsWith(e.name, "ret"))
			w.nextPointer(); // skip the fpret/stret argument
		
		(*out) << objc_helper(e.name, w.nextPointer(), w.nextPointer(), searchable);
	}
	else no_objc:
		(*out) << e.printName;
	(*out) << '(';
	
	// std::cout << "Looking for " << searchable << std::endl;
	it = m_functionInfo.find(searchable);
	
	if (it != m_functionInfo.end())
	{
		bool first = true;
		
		for (char c : it->second.arguments)
		{
			if (!first)
				(*out) << ", ";
			else
				first = false;
			
			(*out) << w.next(c);
		}
		(*out) << ") ";
	}
	else
		(*out) << "?) ";
	(*out) << "ret_ip=" << stack->retAddr << std::endl << std::flush;

	retInfo.retAddr = stack->retAddr;
	retInfo.it = it;
	gettimeofday(&retInfo.callTime, 0);

	g_returnInfo->push(retInfo);
	
	return m_pInstance->m_entries[index].addr;
}

void* TrampolineMgr::printInfoR(uint32_t index, CallStack* stack)
{
	void* rv = g_returnInfo->top().retAddr;
	std::ostream* out;

	out = m_pInstance->getLogger();

	auto it = g_returnInfo->top().it;
	
	std::string stamp = timeStamp();
	(*out) << std::string(20 - stamp.size(), ' ');
	(*out) << '[' << stamp << "] ";
	(*out) << std::string(g_returnInfo->size(), ' ');

	ArgumentWalker w(stack);
	if (it != m_functionInfo.end())
		(*out) << "-> " << w.ret(it->second.retType);
	else
		(*out) << "-> ? (" << w.ret('p') << ')';

	(*out) << " {" << callTime() << "} errno=" << errno << std::endl << std::flush; 
	
	g_returnInfo->pop();

	// standard retval in rax, double in xmm0
	return rv;
}

void Trampoline::init(uint32_t i, void* (*pDebug)(uint32_t,TrampolineMgr::CallStack*), void* (*pDebugR)(uint32_t,TrampolineMgr::CallStack*))
{
	uintptr_t funcLen = uintptr_t(&trampoline_end) - uintptr_t(&trampoline_start);
	memcpy(this, &trampoline_start, funcLen);
	
	this->reg_saveall = reinterpret_cast<uint64_t>(::reg_saveall);
	this->reg_saveall2 = reinterpret_cast<uint64_t>(::reg_saveall);
	this->reg_restoreall = reinterpret_cast<uint64_t>(::reg_restoreall);
	this->reg_restoreall2 = reinterpret_cast<uint64_t>(::reg_restoreall);
	this->index = i;
	this->index2 = i;
	this->debugFcn = reinterpret_cast<uint64_t>(pDebug);
	this->debugFcnR = reinterpret_cast<uint64_t>(pDebugR);
}

TrampolineMgr::ArgumentWalker::ArgumentWalker(CallStack* stack)
: m_stack(stack)
{
#ifdef __x86_64__
	m_indexInt = m_indexXmm = 0;
#else
	m_indexArg = 0;
#endif
}

TrampolineMgr::ArgumentWalker::ArgumentWalker(CallStack* stack, OutputArguments args)
: m_stack(stack), m_pointers(args)
{
#ifdef __x86_64__
	m_indexInt = m_indexXmm = 0;
#else
	m_indexArg = 0;
#endif
}

#ifdef __x86_64__
uint64_t TrampolineMgr::ArgumentWalker::next64bit()
{
	uint64_t rv;
	if (m_indexInt == 0)
		rv = m_stack->rdi;
	else if (m_indexInt == 1)
		rv = m_stack->rsi;
	else if (m_indexInt == 2)
		rv = m_stack->rdx;
	else if (m_indexInt == 3)
		rv = m_stack->rcx;
	else if (m_indexInt == 4)
		rv = m_stack->r8;
	else if (m_indexInt == 5)
		rv = m_stack->r9;
	else
		rv = m_stack->moreArguments[m_indexInt-6];
	
	m_indexInt++;
	return rv;
}

long long TrampolineMgr::ArgumentWalker::nextLL()
{
	uint64_t v = next64bit();
	return *((long*) &v);
}

int TrampolineMgr::ArgumentWalker::nextInt()
{
	uint64_t v = next64bit();
	return *((int*) &v);
}

long double TrampolineMgr::ArgumentWalker::nextLongDouble()
{
	long double rv;
	if (m_indexXmm >= 0 && m_indexXmm <= 7)
		return m_stack->xmm[m_indexXmm];
	else
		throw std::out_of_range("8th double argument not supported");
	
	m_indexXmm++;
	return rv;
}

double TrampolineMgr::ArgumentWalker::nextDouble()
{
	long double v = nextLongDouble();
	return *((double*)&v);
}

float TrampolineMgr::ArgumentWalker::nextFloat()
{
	long double v = nextLongDouble();
	return *((float*)&v);
}

void* TrampolineMgr::ArgumentWalker::nextPointer()
{
	uint64_t v = next64bit();
	return (void*) v;
}

#else

void* TrampolineMgr::ArgumentWalker::nextPointer()
{
	uint32_t v = m_stack->arguments[m_indexArg++];
	return (void*) v;
}

int TrampolineMgr::ArgumentWalker::nextInt()
{
	uint32_t v = m_stack->arguments[m_indexArg++];
	return *((int*) &v);
}

long long TrampolineMgr::ArgumentWalker::nextLL()
{
	union
	{
		uint32_t b32[2];
		long long b64;
	} ll;

	ll.b32[0] = m_stack->arguments[m_indexArg+1];
	ll.b32[1] = m_stack->arguments[m_indexArg];
	m_indexArg += 2;

	return ll.b64;
}

float TrampolineMgr::ArgumentWalker::nextFloat()
{
	uint32_t v = m_stack->arguments[m_indexArg++];
	return *((float*) &v);
}

double TrampolineMgr::ArgumentWalker::nextDouble()
{
	double* d = (double*) &m_stack->arguments[m_indexArg];
	m_indexArg += 2;
	return *d;
}

#endif

std::string TrampolineMgr::ArgumentWalker::next(char type)
{
	std::stringstream ss;
	void* ptr;
	
	if (type == 'u')
		ss << unsigned(nextInt());
	else if (type == 'i')
		ss << nextInt();
	else if (type == 'q')
		ss << nextLL();
	else if (type == 'f')
		ss << nextFloat();
	else if (type == 'd')
		ss << nextDouble();
	else if (type == 'p' || isupper(type))
		ss << nextPointer();
	else if (type == 'c')
		ss << char(nextInt());
	else if (type == 's')
	{
		const char* s = (const char*) nextPointer();
		ss << (void*)s;
		if (s)
			ss << " \"" << safeString(s) << '"';
	}
	else if (type == 'v')
		ss << "(void)";
	else
		ss << '?';
		
	if (isupper(type))
		m_pointers.push_back(std::make_pair(tolower(type), ptr));
	
	return ss.str();
}

#ifdef __x86_64__
std::string TrampolineMgr::ArgumentWalker::ret(char type)
{
	std::stringstream ss;
	
	if (type == 'u')
		ss << m_stack->rax;
	else if (type == 'c')
		ss << char(m_stack->rax);
	else if (type == 'i' || type == 'q')
	{
		uint64_t u = m_stack->rax;
		ss << *((int64_t*) &u);
	}
	else if (type == 'f')
	{
		long double d = m_stack->xmm[0];
		ss << *((float*)&d);
	}
	else if (type == 'd')
	{
		long double d = m_stack->xmm[0];
		ss << *((double*)&d);
	}
	else if (type == 'p' || isupper(type))
		ss << (void*)m_stack->rax << std::dec;
	else if (type == 's')
	{
		const char* s = (const char*) m_stack->rax;
		ss << (void*)s << " \"" <<  safeString(s) << '"';
	}
	else
		ss << '?';
	
	return ss.str();
}
#else
std::string TrampolineMgr::ArgumentWalker::ret(char type)
{
	std::stringstream ss;
	
	if (type == 'u')
		ss << m_stack->eax;
	else if (type == 'c')
		ss << char(m_stack->eax);
	else if (type == 'i')
	{
		uint32_t u = m_stack->eax;
		ss << *((int32_t*) &u);
	}
	else if (type == 'q')
	{
		union
		{
			int64_t v64;
			uint32_t v32[2];
		} q;
		
		q.v32[0] = m_stack->eax;
		q.v32[1] = m_stack->edx;
		
		ss << q.v64;
	}
	//else if (type == 'f')
	//{
	//	double d = m_stack->st0;
	//	ss << * (((float*)&d)+1);
	//}
	else if (type == 'd' || type == 'f')
	{
		double d = m_stack->st0;
		ss << d;
	}
	else if (type == 'p' || isupper(type))
		ss << (void*)m_stack->eax << std::dec;
	else if (type == 's')
	{
		const char* s = (const char*) m_stack->eax;
		ss << (void*)s << " \"" <<  safeString(s) << '"';
	}
	else
		ss << '?';
	
	return ss.str();
}
#endif

std::string TrampolineMgr::ArgumentWalker::safeString(const char* in)
{
	if (!in)
		return std::string();
	
	std::stringstream rv;
	while (*in)
	{
		if (*in >= 32)
			rv << *in;
		else
		{
			if (*in == '\n')
				rv << "\\n";
			else if (*in == '\r')
				rv << "\\r";
			else if (*in == '\t')
				rv << "\\t";
			else
				rv << "\\x" << std::hex << int(*in);
		}
		in++;
	}
	return rv.str();
}

#ifdef TEST

float mytestfunc(int a, int b, double c)
{
	return a*b+c;
}

long long qfunc()
{
	return 1;
}

int main()
{
	TrampolineMgr* mgr = new TrampolineMgr;
	TrampolineMgr::loadFunctionInfo("/tmp/fi");
	
	float (*pFunc)(int,int,double) = (float (*)(int,int,double)) mgr->generate((void*) &mytestfunc, "mytestfunc");
	int (*pPrintf)(FILE* f, const char*,...) = (int (*)(FILE* f, const char*,...)) mgr->generate((void*) &fprintf, "printf");
	std::cout << pFunc(2,3,0.5) << std::endl;
	//pPrintf(stdout, "Hello world: %s\n", "Test");
	
	delete mgr;
	return 0;
}

#endif

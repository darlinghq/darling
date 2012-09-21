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

TrampolineMgr* TrampolineMgr::m_pInstance = 0;
std::map<std::string, TrampolineMgr::FunctionInfo> TrampolineMgr::m_functionInfo;
struct timeval TrampolineMgr::m_startup;

static __thread std::stack<TrampolineMgr::ReturnInfo>* g_returnInfo = 0;
static std::ofstream* g_logger = 0;
static pid_t g_loggerForPid = 0;

extern "C" void reg_saveall();
extern "C" void reg_restoreall();
extern char** g_argv;

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

	AddrEntry e = { name, targetAddr };
	if (m_nNext >= m_nMax)
		throw std::runtime_error("TrampolineMgr buffer full");

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
		size_t p = line.find(':');
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
	ss << ms << "ms";
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
	pid_t pid = getpid();
	std::stringstream ss;
	std::string progname = g_argv[0];
	size_t pos;

	pos = progname.rfind('/');
	if (pos != std::string::npos)
		progname = progname.substr(pos+1);

	ss << m_wd << '/' << progname << '.' << pid << ".log";
	return ss.str();
}

std::ostream* TrampolineMgr::getLogger()
{
	if (g_loggerForPid == getpid())
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

void* TrampolineMgr::printInfo(uint32_t index, CallStack* stack)
{	
	FunctionInfo* info = 0;
	const std::string& name = m_pInstance->m_entries[index].name;
	auto it = m_functionInfo.find(name);
	std::ostream* out;
	ReturnInfo retInfo;

	out = m_pInstance->getLogger();
	
	if (!g_returnInfo)
		g_returnInfo = new std::stack<TrampolineMgr::ReturnInfo>;
	
	std::string stamp = timeStamp();
	(*out) << std::string(20 - stamp.size(), ' ');
	(*out) << '[' << stamp << "] ";
	(*out) << std::string(g_returnInfo->size(), ' ');
	
	if (it != m_functionInfo.end())
	{
		ArgumentWalker w(stack);
		bool first = true;
		
		(*out) << name << '(';
		
		for (char c : it->second.arguments)
		{
			if (!first)
				(*out) << ", ";
			else
				first = false;
			
			(*out) << w.next(c);
		}
		(*out) << ")\n" << std::flush;
	}
	else
		(*out) << m_pInstance->m_entries[index].name << "(?)\n" << std::flush;

	retInfo.retAddr = stack->retAddr;
	gettimeofday(&retInfo.callTime, 0);

	g_returnInfo->push(retInfo);
	
	return m_pInstance->m_entries[index].addr;
}

void* TrampolineMgr::printInfoR(uint32_t index, CallStack* stack)
{
	void* rv = g_returnInfo->top().retAddr;
	std::ostream* out;

	out = m_pInstance->getLogger();

	const std::string& name = m_pInstance->m_entries[index].name;
	auto it = m_functionInfo.find(name);
	
	std::string stamp = timeStamp();
	(*out) << std::string(20 - stamp.size(), ' ');
	(*out) << '[' << stamp << "] ";
	(*out) << std::string(g_returnInfo->size(), ' ');

	if (it != m_functionInfo.end())
	{
		ArgumentWalker w(stack);
		(*out) << "-> " << w.ret(it->second.retType);
	}
	else
		(*out) << "-> ?";

	(*out) << " {" << callTime() << "}\n" << std::flush; 
	
	g_returnInfo->pop();

	// standard retval in rax, double in xmm0
	return rv;
}

void Trampoline::init(uint32_t i, void* (*pDebug)(uint32_t,TrampolineMgr::CallStack*), void* (*pDebugR)(uint32_t,TrampolineMgr::CallStack*))
{
	// See trampoline in trampoline_helper.asm for source
#ifdef __x86_64__
	memcpy(this, "\x49\xba\xb6\xb5\xb4\xb3\xb2\xb1\xb0\x00\x41\xff\xd2\xbf"
		"\x56\x34\x12\x00\x48\x89\xe6\x48\xb9\xff\xee\xdd\xcc\xbb\xaa\x00\x00"
		"\xff\xd1\x49\x89\xc3\x49\xba\xc6\xc5\xc4\xc3\xc2\xc1\xc0\x00\x41\xff"
		"\xd2\x4c\x8d\x15\x08\x00\x00\x00\x4c\x89\x14\x24\x41\xff\xe3\x90\x49"
		"\xba\xb6\xb5\xb4\xb3\xb2\xb1\xb0\x00\x41\xff\xd2\xbf\x56\x34\x12\x00"
		"\x48\x89\xe6\x48\xb9\xa6\xa5\xa4\xa3\xa2\xa1\xa0\x00\xff\xd1\x49\x89"
		"\xc3\x49\xba\xc6\xc5\xc4\xc3\xc2\xc1\xc0\x00\x41\xff\xd2\x41\xff\xe3",
		sizeof(*this)
	);
#else
	memcpy(this, "\x68\xa3\xa2\xa1\xa0\xc3\x54\x68\x78\x56\x34\x12\x68\xb3"
		"\xb2\xb1\xb0\xc3\x89\x44\x24\xf8\x68\xc3\xc2\xc1\xc0\xc3\xe8\x00"
		"\x00\x00\x00\x8b\x04\x24\x83\xc0\x10\x89\x04\x24\x8b\x44\x24\xe0"
		"\xff\xe0\x90\x68\xa3\xa2\xa1\xa0\xc3\x54\x68\x78\x56\x34\x12\x68"
		"\xb9\xb9\xb9\xb9\xc3\x89\x44\x24\xf8\x68\xc3\xc2\xc1\xc0\xc3\x8b"
		"\x4c\x24\xe0\xff\xe1",
		sizeof(*this)
	);

#endif
	
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
		throw std::out_of_range("7th int argument not supported");
	
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
	else if (type == 'i')
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
	// TODO
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

double mytestfunc(int a, int b, double c)
{
	return a*b+c;
}

int main()
{
	TrampolineMgr* mgr = new TrampolineMgr;
	TrampolineMgr::loadFunctionInfo("/tmp/fi");
	
	double (*pFunc)(int,int,double) = (double (*)(int,int,double)) mgr->generate((void*) &mytestfunc, "mytestfunc");
	int (*pPrintf)(FILE* f, const char*,...) = (int (*)(FILE* f, const char*,...)) mgr->generate((void*) &fprintf, "printf");
	//std::cout << pFunc(2,3,0.5) << std::endl;
	pPrintf(stdout, "Hello world: %s\n", "Test");
	
	delete mgr;
	return 0;
}

#endif

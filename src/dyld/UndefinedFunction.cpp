/*
This file is part of Darling.

Copyright (C) 2012-2013 Lubos Dolezel

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

#include "UndefinedFunction.h"
#include <unistd.h>
#include <sys/mman.h>
#include <stdexcept>
#include <dlfcn.h>

UndefMgr::UndefMgr(int entries)
: m_nNext(0)
{
	int ps = getpagesize();
	void* mem;
	
	int bytes = entries * sizeof(UndefinedFunction);
	bytes = (bytes + ps - 1) / ps * ps;
	
	mem = ::mmap(0, bytes, PROT_EXEC | PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, 0, 0);
	if (mem == MAP_FAILED)
		throw std::runtime_error("Failed to map pages for UndefMgr");
	
	m_pMem = static_cast<UndefinedFunction*>(mem);
	m_nMax = bytes / sizeof(UndefinedFunction);
	m_nBytes = bytes;
}

UndefMgr::~UndefMgr()
{
	::munmap(m_pMem, m_nBytes);
}

void* UndefMgr::generateNew(const char* name)
{
	if (m_nNext >= m_nMax)
		throw std::runtime_error("UndefMgr buffer full");
	
	m_pMem[m_nNext].init(name);
	return &m_pMem[m_nNext++];
}

void UndefinedFunction::init(const char* name)
{
#if defined(__x86_64__)
	_asm1[0] = 0x48;
	_asm1[1] = 0xbf;
	_asm2[0] = 0x48;
	_asm2[1] = 0xbe;
	_asm3[0] = 0x48;
	_asm3[1] = 0xba;
	_asm4[0] = 0x48;
	_asm4[1] = 0xbb;
	_asm5[0] = 0x48;
	_asm5[1] = 0x31;
	_asm5[2] = 0xc0;
	_asm5[3] = 0xff;
	_asm5[4] = 0xd3;
	_asm5[5] = 0x48;
	_asm5[6] = 0x31;
	_asm5[7] = 0xc0;
	_asm5[8] = 0xc3;
#elif defined(__i386__)
	_asm1[0] = 0xb8;
	_asm2[0] = 0x50;
	_asm2[1] = 0xb8;
	_asm3[0] = 0x50;
	_asm3[1] = 0xb8;
	_asm4[0] = 0x50;
	_asm4[1] = 0xb8;
	_asm5[0] = 0xff;
	_asm5[1] = 0xd0;
	_asm5[2] = 0x83;
	_asm5[3] = 0xc4;
	_asm5[4] = 0x0c;
	_asm5[5] = 0x31;
	_asm5[6] = 0xc0;
	_asm5[7] = 0xc3;
#else
#   error Unsupported platform!
#endif

	pStderr = stderr;
	pErrMsg = "Undefined function called: %s\n";
	pFuncName = name;
	pFprintf = dlsym(RTLD_DEFAULT, "fprintf");
}

#ifdef TEST
int main()
{
	UndefMgr* mgr = new UndefMgr;
	int (*func)() = (int(*)()) mgr->generateNew("TestFunction");
	int (*func2)() = (int(*)()) mgr->generateNew("TestFunction2");
	
	func();
	func2();
	
	delete mgr;
	return 0;
}
#endif

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

#ifndef UNDEFINEDFUNCTION_H
#define UNDEFINEDFUNCTION_H
#include <cstdio>

#pragma pack(1)
struct UndefinedFunction
{
	void init(const char* name);

#if defined(__x86_64__)
	char _asm1[2];
	void* pStderr;
	char _asm2[2];
	const void* pErrMsg;
	char _asm3[2];
	const void* pFuncName;
	char _asm4[2];
	const void* pFprintf;
	char _asm5[9];
	char padding[7]; // to 48 bytes
#elif defined(__i386__)
	char _asm1[1];
	const void* pFuncName;
	char _asm2[2];
	const void* pErrMsg;
	char _asm3[2];
	const void* pStderr;
	char _asm4[2];
	const void* pFprintf;
	char _asm5[8];
	char padding[1]; // to 32 bytes
#else
#	error Unsupported platform!
#endif

};
#pragma pack()

class UndefMgr
{
public:
	UndefMgr(int entries = 1000);
	~UndefMgr();
	
	// the name must be persistent!
	void* generateNew(const char* name);
private:
	UndefinedFunction* m_pMem;
	int m_nMax, m_nNext, m_nBytes;
};

#endif

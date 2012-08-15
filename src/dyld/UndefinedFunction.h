#ifndef UNDEFINEDFUNCTION_H
#define UNDEFINEDFUNCTION_H
#include <cstdio>

#pragma pack(1)
struct UndefinedFunction
{
	void init(const char* name);

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
};
#pragma pack()

class UndefMgr
{
public:
	UndefMgr(int entries = 300);
	~UndefMgr();
	
	// the name must be persistent!
	void* generateNew(const char* name);
private:
	UndefinedFunction* m_pMem;
	int m_nMax, m_nNext, m_nBytes;
};

#endif

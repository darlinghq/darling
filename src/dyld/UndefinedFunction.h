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

#ifndef UNDEFINEDFUNCTION_H
#define UNDEFINEDFUNCTION_H

#pragma pack(1)
namespace llvm
{
  class Module;
  class Function;
  class FunctionType;
  
class Value;
class ExecutionEngine;
}
class UndefinedFunction
{
public:
	void init(const char* name);

	void* getPointer();
private:
  llvm::Module* mod;
  llvm::Function* llvm_fprintf;
  llvm::Value* undefined_func_format;
  llvm::ExecutionEngine* ee;
  void * ptr;
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
	
	
};

#endif

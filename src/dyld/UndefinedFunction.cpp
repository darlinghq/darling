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

#include "UndefinedFunction.h"
#include <unistd.h>
#include <sys/mman.h>
#include <stdexcept>
#include <sstream>
#include <dlfcn.h>
#include <llvm/Module.h>
#include <llvm/Function.h>
#include <llvm/PassManager.h>
#include <llvm/Analysis/Verifier.h>
#include <llvm/Assembly/PrintModulePass.h>
#include <llvm/Support/IRBuilder.h>
#include <llvm/Constant.h>
#include "llvm/Type.h"
#include <llvm/ExecutionEngine/JIT.h>
#include <llvm/Support/TargetSelect.h>
#include <stdio.h>
static char undef_fmt[] = "Undefined function: %s\n";
UndefMgr::UndefMgr(int entries)
{
  
}

UndefMgr::~UndefMgr()
{

}

void* UndefMgr::generateNew(const char* name)
{
  UndefinedFunction * uf = new UndefinedFunction();
  uf->init(name);
  return uf->getPointer();
}

void UndefinedFunction::init(const char* name)
{
  std::stringstream ss;
  ss << "undefined_function_" << name;
  mod = new llvm::Module(ss.str().c_str(),llvm::getGlobalContext());
  llvm::Type * params1_p[2];
  params1_p[0] = llvm::PointerType::getInt32PtrTy(llvm::getGlobalContext());
  params1_p[1] = llvm::PointerType::getInt8PtrTy(llvm::getGlobalContext());
  llvm::ArrayRef<llvm::Type*> params1(params1_p,2);
  llvm::FunctionType* FuncTy_7 = llvm::FunctionType::get(
  /*Result=*/llvm::IntegerType::get(mod->getContext(), 32),
  /*Params=*/params1,
  /*isVarArg=*/true);
  llvm_fprintf = llvm::cast<llvm::Function>(mod->getOrInsertFunction("fprintf",FuncTy_7));
  llvm::Function* uf = llvm::cast<llvm::Function>( mod->getOrInsertFunction("undefinedfunction",llvm::Type::getVoidTy(llvm::getGlobalContext()), NULL));
  llvm::BasicBlock* entry = llvm::BasicBlock::Create(llvm::getGlobalContext(),"entry",uf);
  llvm::IRBuilder<> b(entry);
  llvm::Value* f = llvm::cast<llvm::Value>(llvm::ConstantExpr::getIntToPtr(llvm::ConstantInt::get(llvm::PointerType::getInt32PtrTy(llvm::getGlobalContext()),(uint64_t)stderr),llvm::Type::getInt32PtrTy(llvm::getGlobalContext())));
   
  llvm::Value* pp0 = b.CreateGlobalStringPtr(undef_fmt);
  llvm::Value* pp1 = b.CreateGlobalStringPtr(name);
  b.CreateCall3(llvm_fprintf,f,pp0,pp1,"fprintf");
  b.CreateRet(NULL);
  llvm::verifyModule(*mod);
  std::string es;
  llvm::InitializeNativeTarget();
  llvm::EngineBuilder eb = llvm::EngineBuilder(mod);
  eb.setEngineKind(llvm::EngineKind::JIT);
  eb.setErrorStr(&es);
  llvm::ExecutionEngine * ee = eb.create();
  ptr = ee->getPointerToFunction(uf);
  
  
}
void* UndefinedFunction::getPointer()
{
  return ptr;
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

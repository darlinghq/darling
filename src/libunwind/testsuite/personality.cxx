
// TEST-OPTIONS: -arch i386 
// TEST-OPTIONS: -arch i386 -Wl,-no_compact_unwind
// TEST-OPTIONS: -arch x86_64 
// TEST-OPTIONS: -arch x86_64  -Wl,-no_compact_unwind
// TEST-OPTIONS: -arch ppc 

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dlfcn.h>

#include "unwind.h"

static __personality_routine realPersonality = (__personality_routine)dlsym(RTLD_DEFAULT, "__gxx_personality_v0");

int count = 0;

const char* actionString(_Unwind_Action act)
{
	static char buffer[100];
	buffer[0] = '\0';
	
	if  ( act & _UA_SEARCH_PHASE )
		strcat(buffer, "_UA_SEARCH_PHASE ");
	if  ( act & _UA_CLEANUP_PHASE )
		strcat(buffer, "_UA_CLEANUP_PHASE ");
	if  ( act & _UA_HANDLER_FRAME )
		strcat(buffer, "_UA_HANDLER_FRAME ");
	if  ( act & _UA_FORCE_UNWIND )
		strcat(buffer, "_UA_FORCE_UNWIND ");
	if  ( act & _UA_END_OF_STACK )
		strcat(buffer, "_UA_END_OF_STACK ");
	if  ( act & 0xFFFFFFE0 )
		strcat(buffer, "unknown bits ");
	return buffer;
}

const char* reasonString(_Unwind_Reason_Code reason)
{
	switch (reason) {
		case _URC_NO_REASON:
			return "_URC_NO_REASON";
		case _URC_FOREIGN_EXCEPTION_CAUGHT:
			return "_URC_FOREIGN_EXCEPTION_CAUGHT";
		case _URC_FATAL_PHASE2_ERROR:
			return "_URC_FATAL_PHASE2_ERROR";
		case _URC_FATAL_PHASE1_ERROR:
			return "_URC_FATAL_PHASE1_ERROR";
		case _URC_NORMAL_STOP:
			return "_URC_NORMAL_STOP";
		case _URC_END_OF_STACK:
			return "_URC_END_OF_STACK";
		case _URC_HANDLER_FOUND:
			return "_URC_HANDLER_FOUND";
		case _URC_INSTALL_CONTEXT:
			return "_URC_INSTALL_CONTEXT";
		case _URC_CONTINUE_UNWIND:
			return "_URC_CONTINUE_UNWIND";
	}
	return "unknown reason code";
}

bool fooCaught = false;
bool mainInnerCaught = false;
bool mainOuterCaught = false;

struct Expected {
	bool				fooCaught;
	bool				mainInnerCaught;
	bool				mainOuterCaught;
	_Unwind_Action		action;
	_Unwind_Reason_Code	result;
};

#define _UA_CLEANUP_PHASE_HANDLER_FRAME (_Unwind_Action)(_UA_CLEANUP_PHASE|_UA_HANDLER_FRAME)

const Expected shouldBe[] = {
	{ false, false, false, _UA_SEARCH_PHASE,					_URC_HANDLER_FOUND   },
	{ false, false, false, _UA_CLEANUP_PHASE_HANDLER_FRAME,		_URC_INSTALL_CONTEXT },
	{ true,  false, false, _UA_SEARCH_PHASE,					_URC_CONTINUE_UNWIND },
	{ true,  false, false, _UA_SEARCH_PHASE,					_URC_HANDLER_FOUND   },
	{ true,  false, false, _UA_CLEANUP_PHASE,					_URC_INSTALL_CONTEXT },
	{ true,  false, false, _UA_CLEANUP_PHASE,					_URC_CONTINUE_UNWIND },
	{ true,  false, false, _UA_CLEANUP_PHASE_HANDLER_FRAME,		_URC_INSTALL_CONTEXT },
	{ true,  true,  false, _UA_SEARCH_PHASE,					_URC_HANDLER_FOUND   },
	{ true,  true,  false, _UA_CLEANUP_PHASE_HANDLER_FRAME,		_URC_INSTALL_CONTEXT }
};	


extern "C" {
	__attribute__((visibility("hidden"))) _Unwind_Reason_Code __gxx_personality_v0(
		int version, _Unwind_Action actions, uint64_t exceptionClass,
		struct _Unwind_Exception* exceptionObject, _Unwind_Context* context) 
	{

		_Unwind_Reason_Code result = (*realPersonality)(version, actions, exceptionClass,
														exceptionObject, context);
		//fprintf(stderr, "fooCaught=%d, mainInnerCaught=%d, mainOuterCaught=%d, ",
		//		fooCaught, 	mainInnerCaught, mainOuterCaught);
		//fprintf(stderr, "personality(%d, %s, 0x%llX, %p, %p) => %s\n", version, actionString(actions), 
		//			exceptionClass, exceptionObject, context, reasonString(result));
		if ( shouldBe[count].fooCaught != fooCaught ) {
			fprintf(stderr, "count=%d, fooCaught=%d\n", count, fooCaught);
			abort();
		}
		if ( shouldBe[count].mainInnerCaught != mainInnerCaught ) {
			fprintf(stderr, "count=%d, mainInnerCaught=%d\n", count, mainInnerCaught);
			abort();
		}
		if ( shouldBe[count].mainOuterCaught != mainOuterCaught )  {
			fprintf(stderr, "count=%d, mainOuterCaught=%d\n", count, mainOuterCaught);
			abort();
		}
		if ( shouldBe[count].action != actions )  {
			fprintf(stderr, "count=%d, actions=%s\n", count, actionString(actions));
			abort();
		}
		if ( shouldBe[count].result != result )  {
			fprintf(stderr, "count=%d, result=%s\n", count, reasonString(result));
			abort();
		}
		if ( count++ > 12 )
			abort();
		return result;

	}
}


int __attribute__((noinline)) bar() 
{ 
	throw 10; 
}


void __attribute__((noinline)) foo() 
{ 
	try {
		bar();
	}
	catch(int x) {
		fooCaught = true;
		throw;
	}
}



int main()
{
	try {
		try {
			foo();
		}
		catch (int x) {
			mainInnerCaught = true;
			throw;
		}
	}
	catch (...) {
		mainOuterCaught = true;
	}
	if ( count != 9 ) {
		fprintf(stderr, "count=%d\n", count);
		abort();
	}
	return 0;
}


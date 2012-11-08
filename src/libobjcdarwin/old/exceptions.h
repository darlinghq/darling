#ifndef OBJC_EXCEPTIONS_H
#define OBJC_EXCEPTIONS_H
#include <objc/runtime.h>
#include <setjmp.h>
#include "../visibility.h"

// Apple 32-bit ObjC ABI doesn't use Itanium ABI zero-cost exceptions.
// Instead, every try block is registered and deregistered, every catch
// block is tested for usability with a runtime function call and if no
// match is found, the exception is thrown again to traverse the try
// block chain.

struct TryBlock
{
	jmp_buf buffer;

	union
	{
		void* fourPointers[4];

		struct
		{
			objc_object* exceptionObject;
			TryBlock* previousBlock;
		};
	};
};

extern "C" {

// This function is called repeatedly until a handler is found
DARLING_VISIBLE void __darwin_objc_exception_throw(objc_object* object);

// Called on every @try {
DARLING_VISIBLE void objc_exception_try_enter(TryBlock* state);

// Called on every } of a @try block
DARLING_VISIBLE void objc_exception_try_exit(TryBlock* state);

// Called to get the exception object
DARLING_VISIBLE void* objc_exception_extract(TryBlock* state);

// Called to check if the handler's type is appropriate for the exception object
DARLING_VISIBLE int objc_exception_match(objc_class* cls, objc_object* object);

}

#endif


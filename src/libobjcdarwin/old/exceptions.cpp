#include "exceptions.h"
#include <cstdlib>
#include <iostream>
#include "../util/trace.h"

static __thread TryBlock* m_lastBlock = nullptr;

void __darwin_objc_exception_throw(objc_object* object)
{
	TRACE1(object);

	if (!object)
	{
		std::cerr << "NULL Objective-C exception thrown!\n";
		abort();
	}

	if (!m_lastBlock)
	{
		std::cerr << "Unhandled Objective-C exception: " << class_getName(object_getClass(object)) << '(' << object << ")\n";
		abort();
	}
	else
	{
		TryBlock* currentBlock = m_lastBlock;
		
		currentBlock->exceptionObject = object;
		m_lastBlock = m_lastBlock->previousBlock;

		longjmp(currentBlock->buffer, true);
	}
}

void objc_exception_try_enter(TryBlock* state)
{
	TRACE1(state);
	state->previousBlock = m_lastBlock;
	m_lastBlock = state;
}

void objc_exception_try_exit(TryBlock* state)
{
	TRACE1(state);
	if (state != m_lastBlock)
	{
		std::cerr << "Incorrect try-block state passed to objc_exception_try_exit\n";
		abort();
	}
	m_lastBlock = state->previousBlock;
}

void* objc_exception_extract(TryBlock* state)
{
	TRACE1(state);
	return state->exceptionObject;
}

int objc_exception_match(objc_class* cls, objc_object* object)
{
	TRACE2(cls, object);
	objc_class* objClass = object_getClass(object);

	while (objClass)
	{
		if (objClass == cls)
			return true;
		else
			objClass = class_getSuperclass(objClass);
	}

	return false;
}


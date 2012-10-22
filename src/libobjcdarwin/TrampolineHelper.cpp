#include <string>
#include <objc/runtime.h>
#include <cstring>
#include <sstream>
#include "visibility.h"
#include "../util/stlutils.h"

DARLING_VISIBLE std::string trampoline_objcMsgInfo(const std::string& invoker, void* arg1, SEL sel, std::string& searchable) asm("trampoline_objcMsgInfo");

std::string trampoline_objcMsgInfo(const std::string& invoker, void* arg1, SEL sel, std::string& searchable)
{
	id obj;
	Class type;
	std::stringstream ret;
	
	if (string_startsWith(invoker, "objc_msgSendSuper2"))
	{
		const objc_super* s = static_cast<objc_super*>(arg1);
		obj = (id) class_getSuperclass(Class(s->super_class));
	}
	else if (string_startsWith(invoker, "objc_msgSendSuper"))
	{
		const objc_super* s = static_cast<objc_super*>(arg1);
		obj = (id) s->super_class;
	}
	else
		obj = id(arg1);
	
	if (obj)
	{
		type = object_getClass(obj);
		
		if (class_isMetaClass(type))
			ret << "+[";
		else
			ret << "-[";
		
		ret << class_getName(type);
		searchable = ret.str();
		
		ret << '(';
		ret << obj << ") ";
		
		searchable += ' ';
		searchable += sel_getName(sel);
		searchable += ']';
	}
	else
		ret << "?[nil(0x0) ";
	
	ret << sel_getName(sel);
	ret << ']';
	
	return ret.str();
}


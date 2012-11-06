#include <string>
#include <objc/runtime.h>
#include <cstring>
#include <sstream>
#include <iostream>
#include <Foundation/NSObject.h>
#include "visibility.h"
#include "../util/stlutils.h"

DARLING_VISIBLE std::string trampoline_objcMsgInfo(const std::string& invoker, void* arg1, SEL sel, std::string& searchable) asm("trampoline_objcMsgInfo");

std::string trampoline_objcMsgInfo(const std::string& invoker, void* arg1, SEL sel, std::string& searchable)
{
	id obj;
	Class type;
	std::stringstream ret;
	bool superCall = false;
	
	if (string_startsWith(invoker, "objc_msgSendSuper2"))
	{
		const objc_super* s = static_cast<objc_super*>(arg1);
		obj = (id) class_getSuperclass(Class(s->super_class));
		superCall = true;
	}
	else if (string_startsWith(invoker, "objc_msgSendSuper"))
	{
		const objc_super* s = static_cast<objc_super*>(arg1);
		obj = (id) s->super_class;
		superCall = true;
	}
	else
		obj = id(arg1);
	
	if (string_endsWith(invoker, "_fixup"))
	{
		struct fixable
		{
			void* pfn;
			SEL sel;
		};
		const fixable* f = (fixable*) sel;
		sel = f->sel;
	}
	
	const char* selname = sel_getName(sel);
	if (obj)
	{
		const char* clsname;
		bool isMeta;
		
		type = object_getClass(obj);
		clsname = class_getName(type);
		
		isMeta = class_isMetaClass(type) == YES;
		if (isMeta && !superCall)
			ret << "+[";
		else
			ret << "-[";
			
		// std::cout << "Resp: " << bool([[NSObject class] instancesRespondToSelector:@selector(description)]) << std::endl;
		
		searchable = ret.str();
		if ( (!isMeta && [NSObject instancesRespondToSelector:sel]) ||
			(isMeta && ([[NSObject class] instancesRespondToSelector:sel] || !strcmp(selname, "alloc")))
		)
		{
			searchable += "NSObject";
		}
		else
			searchable += clsname;
		
		ret << clsname;
		
		ret << '(';
		ret << obj << ") ";
		
		searchable += ' ';
		searchable += selname;
		searchable += ']';
	}
	else
		ret << "?[nil(0x0) ";
	
	ret << selname;
	ret << ']';
	
	return ret.str();
}


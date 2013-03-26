#include "./ivar.h"
#include <Foundation/NSObjCRuntime.h>
#include "../../util/log.h"

void ConvertIvarList(Class c, const old_ivar_list* list)
{
	LOG << list->count << " ivars within\n";

	for (size_t i = 0; i < list->count; i++)
	{
		auto* v = &list->ivar_list[i];
		NSUInteger size, alignment;

		NSGetSizeAndAlignment(v->type, &size, &alignment);
		alignment = __builtin_ffs(alignment) - 1; // TODO: why do we need alignment when we have an offset?
		
		LOG << "Ivar: name: " << v->name << "; type: " << v->type << "; offset: " << v->offset << "; size: " << size << "; alignment: " << alignment << std::endl;
		class_addIvar(c, v->name, size, alignment, v->type);
	}
}

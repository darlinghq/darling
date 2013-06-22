#include "./ivar.h"
#include "../../util/log.h"

void ConvertIvarList(Class c, const ivar_list_t* list)
{
	LOG << list->count << " ivars within\n";

	for (size_t i = 0; i < list->count; i++)
	{
		auto* v = &list->ivar_list[i];
		int align = __builtin_ffs(v->alignment) - 1;

		LOG << "Ivar: name: " << v->name << "; type: " << v->type << "; offset: " << *v->offset << "; size: " << v->size << "; alignment: " << v->alignment << std::endl;
		class_addIvar(c, v->name, v->size, v->alignment, v->type);
		
		Ivar ivar = class_getInstanceVariable(c, v->name);
		
		if (ivar)
		{
			*v->offset = ivar_getOffset(ivar);
			LOG << "Ivar registered, ivar_getOffset() = " << ivar_getOffset(ivar) << " in " << class_getInstanceSize(c) << std::endl;
		}

		assert(ivar != nullptr);
		// assert(ivar_getOffset(ivar) == *v->offset);
	}
}


#ifndef COMMON_METHOD_H
#define COMMON_METHOD_H
#include <queue>
#include <cstring>

extern std::queue<std::pair<Class, IMP>> g_pendingInitClasses;

template<typename ListType> void ConvertMethodListGen(Class c, const ListType* list)
{
	LOG << list->count << " methods within\n";

	const bool isMeta = class_isMetaClass(c) == YES;
	
	for (size_t i = 0; i < list->count; i++)
	{
		auto* m = &list->method_list[i];
		IMP imp = reinterpret_cast<IMP>(m->impl);

		LOG << "Method: selName: " << m->selName << "; types: " << m->types << "; impl: " << m->impl << std::endl;

		SEL sel = sel_registerTypedName_np(m->selName, m->types);
		class_addMethod(c, sel, imp, m->types);

		if (isMeta && strcmp(m->selName, "load") == 0)
			g_pendingInitClasses.push(std::make_pair<objc_class*,IMP>((Class)c, *imp));
	}
}

#endif

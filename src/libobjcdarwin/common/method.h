#ifndef COMMON_METHOD_H
#define COMMON_METHOD_H

template<typename ListType> void ConvertMethodListGen(Class c, const ListType* list)
{
	LOG << list->count << " methods within\n";

	const bool isMeta = class_isMetaClass(c) == YES;
	
	for (size_t i = 0; i < list->count; i++)
	{
		auto* m = &list->method_list[i];

		LOG << "Method: selName: " << m->selName << "; types: " << m->types << "; impl: " << m->impl << std::endl;

		SEL sel = sel_registerName(m->selName);
		class_addMethod(c, sel, reinterpret_cast<IMP>(m->impl), m->types);
	}
}

#endif

#include "./protocol.h"
#include <cstring>
#include "../../util/log.h"
#include "../../dyld/public.h"
#include "../common/ref.h"
#include "../common/property.h"
#include "../TopologySort.h"

void AddClassProtocols(Class c, const protocol_list_t* list, intptr_t slide)
{
	for (size_t i = 0; i < list->count; i++)
	{
		const char* name = list->elem(i, slide)->name;
		Protocol* p = objc_getProtocol(name);
		assert(p != nullptr);
		class_addProtocol(c, p);
	}
}

void RegisterProtocolMethods(Protocol* p, const method_list_t* list, const char** extTypes, size_t& extIndex, bool required, bool instance)
{
	LOG << "Registering Protocol methods (" << required << ", " << instance << "): " << list->count << " methods within\n";
	for (size_t i = 0; i < list->count; i++, extIndex++)
	{
		SEL sel = sel_registerName(list->method_list[i].selName);
		protocol_addMethodDescription(p, sel, list->method_list[i].types, required, instance);
		// TODO: what do we do with extTypes?
	}
}

Protocol* RegisterProtocol(const protocol_t* prot, intptr_t slide)
{
	Protocol *existingProtocol = objc_getProtocol(prot->name);

	// Return existing protocols
	if (existingProtocol)
	{
		LOG << "Found existing ObjC Protocol: " << prot->name << std::endl;
		return existingProtocol;
	}

	LOG << "Processing ObjC Protocol: " << prot->name << std::endl;
	Protocol* conv = objc_allocateProtocol(prot->name);
	size_t methodIndex = 0;

	if (prot->protocols)
	{
		for (size_t i = 0; i < prot->protocols->count; i++)
		{
			const char* name = prot->protocols->elem(i, slide)->name;
			protocol_addProtocol(conv, objc_getProtocol(name));
		}
	}

	if (prot->methods)
		RegisterProtocolMethods(conv, prot->methods, prot->extMethTypes, methodIndex, true, true);
	if (prot->staticMethods)
		RegisterProtocolMethods(conv, prot->staticMethods, prot->extMethTypes, methodIndex, true, false);
	if (prot->optMethods)
		RegisterProtocolMethods(conv, prot->optMethods, prot->extMethTypes, methodIndex, false, true);
	if (prot->optStaticMethods)
		RegisterProtocolMethods(conv, prot->optStaticMethods, prot->extMethTypes, methodIndex, false, false);

	if (prot->properties)
		ConvertProperties(prot->properties, [conv](const char* name, const objc_property_attribute_t* attr, unsigned int count) { protocol_addProperty(conv, name, attr, count, true, true);  });
	
	objc_registerProtocol(conv);
	return conv;
}

void ProcessProtocolsNew(const struct mach_header* mh, intptr_t slide)
{
	const protocol_t** protocol_list;
	unsigned long protosize;

	protocol_list = reinterpret_cast<const protocol_t**>(
		getsectdata(mh, SEG_OBJC_PROTOLIST_NEW, SECT_OBJC_PROTOLIST_NEW, &protosize)
	);

	if (protocol_list)
	{
		unsigned long refsize;
		protocol_t** protocol_refs;
		protocol_t** protocol_refs_end;
		std::set<const protocol_t*> setProtocols;
		std::vector<const protocol_t*> vecProtocols;

		protocol_refs = reinterpret_cast<protocol_t**>(
			getsectdata(mh, SEG_OBJC_PROTOREFS_NEW, SECT_OBJC_PROTOREFS_NEW, &refsize)
		);

		if (protocol_refs)
			protocol_refs_end = protocol_refs + refsize / sizeof(protocol_t*);

		std::copy(protocol_list, protocol_list+protosize/sizeof(protocol_t*), std::inserter(setProtocols, setProtocols.begin()));
		topology_sort<const protocol_t>(setProtocols, vecProtocols,
			[&setProtocols,slide](const protocol_t* p) { return p->protocols ?  std::set<const protocol_t*>(p->protocols->begin(slide), p->protocols->end(slide)) : std::set<const protocol_t*>();  });

		for (const protocol_t* proto : vecProtocols)
		{
			Protocol* p = RegisterProtocol(proto, slide);
			
			if (protocol_refs)
				find_and_fix(protocol_refs, protocol_refs_end, proto, p);
		}
	}
}

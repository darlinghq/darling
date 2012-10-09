#include "./protocol.h"
#include "../../util/log.h"
#include "../TopologySort.h"
#include "../common/property.h"
#include <cstring>

void RegisterProtocolMethods(Protocol* p, const old_method_decl_list* list, bool required, bool instance)
{
	LOG << "Registering Protocol methods (" << required << ", " << instance << "): " << list->count << " methods within\n";
	for (size_t i = 0; i < list->count; i++)
	{
		SEL sel = sel_registerName(list->list[i].name);
		protocol_addMethodDescription(p, sel, list->list[i].types, required, instance);
		// TODO: what do we do with extTypes?
	}
}

Protocol* RegisterProtocol(old_protocol* prot, uintptr_t extStart, unsigned long extLen)
{
	// For reasons unknown, the NSObject protocol is duplicated into the binaries
	if (strcmp(prot->name, "NSObject") == 0)
		return objc_getProtocol(prot->name);

	LOG << "Processing old ObjC Protocol: " << prot->name << std::endl;
	Protocol* conv = objc_allocateProtocol(prot->name);

	if (prot->protocols)
	{
		for (size_t i = 0; i < prot->protocols->count; i++)
		{
			const char* name = prot->protocols->list[i]->name;
			protocol_addProtocol(conv, objc_getProtocol(name));
		}
	}

	if (prot->methods)
		RegisterProtocolMethods(conv, prot->methods, true, true);
	if (prot->staticMethods)
		RegisterProtocolMethods(conv, prot->staticMethods, true, false);

	// Protocol EXT - weird stuff
	if (prot->ext_ptrValue >= extStart && prot->ext_ptrValue < extStart + extLen)
	{
		// TODO: check if rebase is present
		if (prot->ext->optMethods)
			RegisterProtocolMethods(conv, prot->ext->optMethods, false, true);
		if (prot->ext->optStaticMethods)
			RegisterProtocolMethods(conv, prot->ext->optStaticMethods, false, false);
		if (prot->ext->properties)
			ConvertProperties(prot->ext->properties, [conv](const char* name, const objc_property_attribute_t* attr, unsigned int count) { protocol_addProperty(conv, name, attr, count, true, true);  });
	}

	objc_registerProtocol(conv);
	return conv;
}


void ProcessProtocolsOld(const struct mach_header* mh, intptr_t slide)
{
	old_protocol* protocols;
	unsigned long protosize;

	protocols = reinterpret_cast<old_protocol*>(
		getsectdata(mh, SEG_OBJC_PROTOCOLS_OLD, SECT_OBJC_PROTOCOLS_OLD, &protosize)
	);

	if (protocols)
	{
		std::vector<old_protocol*> vecProtocols;
		std::set<old_protocol*> setProtocols;
		// std::map<const char*,old_protocol*> mapProtocols;

		for (size_t i = 0; i < protosize / sizeof(old_protocol); i++)
		{
			if (strcmp(protocols[i].name, "NSObject") != 0) // No need to re-register protocol NSObject
				setProtocols.insert(protocols + i);
			// mapProtocols[protocols[i].name] = protocols + i;
		}

		topology_sort<old_protocol>(setProtocols, vecProtocols,
			[&setProtocols](old_protocol* p) -> std::set<old_protocol*>
			{
				std::set<old_protocol*> set;
				if (p->protocols)
				{
					for (size_t i = 0; i < p->protocols->count; i++)
					{
						if (setProtocols.count(p->protocols->list[i]))
							set.insert(p->protocols->list[i]);
					}
				}
				return set;
			}
		);

		// Find the section that holds protocol_ext structs
		uintptr_t extStart;
		unsigned long extLen = 0;

		extStart = reinterpret_cast<uintptr_t>(
			getsectdata(mh, SEG_OBJC_PROTOEXT_OLD, SECT_OBJC_PROTOEXT_OLD, &extLen)
		);

		for (old_protocol* old : vecProtocols)
		{
			Protocol* p = RegisterProtocol(old, extStart, extLen);
			old->isa = (id) p; // This is what is referenced in the code
		}
	}
}

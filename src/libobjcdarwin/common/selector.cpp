#include "selector.h"
#include "../new/AppleLayout.h"
#include "../old/AppleLayout.h"
#include <libdyld/dyld_public.h>
#include <util/debug.h>
#include <utility>
#include <cstring>

void UpdateSelectors(const struct mach_header* mh, intptr_t slide)
{
	selref* sel_refs;
	msgref* msg_refs;
	unsigned long selsize, msgsize;

#ifdef OBJC_ABI_2
	sel_refs = reinterpret_cast<selref*>(
		getsectdata(mh, SEG_OBJC_SELREFS_NEW, SECT_OBJC_SELREFS_NEW, &selsize)
	);
	
	msg_refs = reinterpret_cast<msgref*>(
		getsectdata(mh, SEG_OBJC_MSGREFS_NEW, SECT_OBJC_MSGREFS_NEW, &msgsize)
	);
	
#else
	
	sel_refs = reinterpret_cast<selref*>(
		getsectdata(mh, SEG_OBJC_SELREFS_OLD, SECT_OBJC_SELREFS_OLD, &selsize)
	);
#endif

	if (sel_refs)
	{
		for (size_t i = 0; i < selsize / sizeof(selref); i++)
		{
			SEL native = sel_getUid(sel_refs[i].selName);
			LOG << "ObjC SEL \"" << sel_refs[i].selName << "\" fixup @" << (sel_refs+i) << ": " << sel_refs[i].sel << " -> " << native << std::endl;
			sel_refs[i].sel = native;
		}
	}
	
#ifdef OBJC_ABI_2
	if (msg_refs)
	{
		for (size_t i = 0; i < msgsize / sizeof(msgref); i++)
		{
			SEL native = sel_getUid(msg_refs[i].sel.selName);
			LOG << "ObjC msgref \"" << msg_refs[i].sel.selName << "\" fixup @" << &msg_refs[i].sel.sel << ": " << msg_refs[i].sel.sel << " -> " << native << std::endl;
			msg_refs[i].sel.sel = native;
		}
	}
#endif
}


static const std::pair<const char*, const char*> equivalentTypes[] = {
	std::make_pair("CGRect", "_NSRect"),
	std::make_pair("CGPoint", "_NSPoint"),
	std::make_pair("CGSize", "_NSSize")
};

void ConvertSelectorType(std::string& selType)
{
	bool changed = false;
	for (auto p : equivalentTypes)
	{
		size_t pos;
		
		while ((pos = selType.find(p.first)) != std::string::npos)
		{
			selType.replace(pos, strlen(p.first), p.second);
			changed = true;
		}
	}
	
	if (changed)
		LOG << "convertSelectorType: new type info is " << selType << std::endl;
}



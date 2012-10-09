#include "selector.h"
#include "../new/AppleLayout.h"
#include "../old/AppleLayout.h"
#include "../../dyld/public.h"
#include "../../util/log.h"

void UpdateSelectors(const struct mach_header* mh, intptr_t slide)
{
	selref* sel_refs;
	msgref* msg_refs;
	unsigned long selsize, msgsize;

	sel_refs = reinterpret_cast<selref*>(
		getsectdata(mh, SEG_OBJC_SELREFS_NEW, SECT_OBJC_SELREFS_NEW, &selsize)
	);
	
	msg_refs = reinterpret_cast<msgref*>(
		getsectdata(mh, SEG_OBJC_MSGREFS_NEW, SECT_OBJC_MSGREFS_NEW, &msgsize)
	);
	
	if (!sel_refs)
	{
		sel_refs = reinterpret_cast<selref*>(
			getsectdata(mh, SEG_OBJC_SELREFS_OLD, SECT_OBJC_SELREFS_OLD, &selsize)
		);
	}

	if (sel_refs)
	{
		for (size_t i = 0; i < selsize / sizeof(selref); i++)
		{
			SEL native = sel_getUid(sel_refs[i].selName);
			LOG << "ObjC SEL fixup @" << (sel_refs+i) << ": " << sel_refs[i].sel << " -> " << native << std::endl;
			sel_refs[i].sel = native;
		}
	}
	if (msg_refs)
	{
		for (size_t i = 0; i < msgsize / sizeof(msgref); i++)
		{
			SEL native = sel_getUid(msg_refs[i].sel.selName);
			LOG << "ObjC msgref fixup @" << &msg_refs[i].sel.sel << ": " << msg_refs[i].sel.sel << " -> " << native << std::endl;
			msg_refs[i].sel.sel = native;
		}
	}
}

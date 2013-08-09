#include <bsd/stdlib.h>
#include <libdyld/MachOMgr.h>
#include <libdyld/MachOObject.h>

using namespace Darling;

__attribute__((constructor)) static void initProgname()
{
	MachOObject* main = MachOMgr::instance()->mainModule();
	
	if (main)
		setprogname(main->name());
}

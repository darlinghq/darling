#include "../../dyld/public.h"
#include <stdint.h>
#include <cstring>
#include <algorithm>

#pragma pack(1)
struct apple_cfstring
{
	void* isa;
	uint8_t flags;
	uint8_t typeID;
	uint8_t unused[2];
#ifdef __x86_64__
	uint32_t extraFlags;
#endif
	void* data;
	unsigned long length;
};
// 32bit: 16 bytes
// 64bit: 28 bytes -> 32!
#pragma pack()

// gnustep-corebase
struct gnustep_cfstring
{
	void* isa;
	int16_t typeID;
	struct
	{
		int16_t ro:1;
		int16_t reserved:7;
		int16_t info:8;
	} _flags;
	void* data;
	uint32_t length;
	uintptr_t hashCode;
};
// 32bit: ? bytes
// 64bit: 36 bytes
// -> FIXME: hashes are broken!

enum { apple_immutable = 0x80, apple_notinline = 0x40, apple_unicode = 0x20, apple_hasnull = 0x10, apple_haslength = 8 };
enum { gnustep_mutable = 1, gnustep_inline = 2, gnustep_unicode = 4, gnustep_haslength = 8, gnustep_hasnull = 0x10 };

static const int TYPEID_STRING_APPLE = 7;
static const int TYPEID_STRING_GNUSTEP = 24;

static void FixupCFStrings(apple_cfstring* strings, size_t count);

void UpdateCFStrings(const struct mach_header* mh)
{
	unsigned long size;
	apple_cfstring* strings = reinterpret_cast<apple_cfstring*>(
		getsectdata(mh, "__DATA", "__cfstring", &size)
	);

	if (strings)
		FixupCFStrings(strings, size / sizeof(apple_cfstring));

	strings = reinterpret_cast<apple_cfstring*>(
		getsectdata(mh, "__DATA", "__ustring", &size)
	);

	if (strings)
		FixupCFStrings(strings, size / sizeof(apple_cfstring));
}

void FixupCFStrings(apple_cfstring* strings, size_t count)
{
	for (size_t i = 0; i < count; i++)
	{
		gnustep_cfstring fixed;

		memset(&fixed, 0, sizeof(fixed));

		fixed.isa = strings[i].isa; // This has already been "fixed" during linking
		fixed.length = strings[i].length;
		fixed.data = strings[i].data;
		fixed._flags.ro = 1;
		fixed.typeID = TYPEID_STRING_GNUSTEP;
		fixed._flags.info = gnustep_haslength | gnustep_hasnull;

		if (strings[i].flags & apple_unicode)
			fixed._flags.info |= gnustep_unicode;
		
		memcpy((void*) (strings+i), &fixed, std::min(sizeof(gnustep_cfstring), sizeof(apple_cfstring)));
	}
}


#include "arch.h"
#include <cstring>

// This was dumped on snow leopard.
static NXArchInfo __darwin_all_arch_infos[] = {
  // The first entry indicates the local arch.
  { "hppa", 11, 0, 2, "HP-PA" },
  { "i386", 7, 3, 1, "Intel 80x86" },
  { "x86_64", 16777223, 3, 1, "Intel x86-64" },
  { "i860", 15, 0, 2, "Intel 860" },
  { "m68k", 6, 1, 2, "Motorola 68K" },
  { "m88k", 13, 0, 2, "Motorola 88K" },
  { "ppc", 18, 0, 2, "PowerPC" },
  { "ppc64", 16777234, 0, 2, "PowerPC 64-bit" },
  { "sparc", 14, 0, 2, "SPARC" },
  { "arm", 12, 0, 1, "ARM" },
  { "any", -1, -1, 0, "Architecture Independent" },
  { "veo", 255, 2, 2, "veo" },
  { "hppa7100LC", 11, 1, 2, "HP-PA 7100LC" },
  { "m68030", 6, 3, 2, "Motorola 68030" },
  { "m68040", 6, 2, 2, "Motorola 68040" },
  { "i486", 7, 4, 1, "Intel 80486" },
  { "i486SX", 7, 132, 1, "Intel 80486SX" },
  { "pentium", 7, 5, 1, "Intel Pentium" },
  { "i586", 7, 5, 1, "Intel 80586" },
  { "pentpro", 7, 22, 1, "Intel Pentium Pro" },
  { "i686", 7, 22, 1, "Intel Pentium Pro" },
  { "pentIIm3", 7, 54, 1, "Intel Pentium II Model 3" },
  { "pentIIm5", 7, 86, 1, "Intel Pentium II Model 5" },
  { "pentium4", 7, 10, 1, "Intel Pentium 4" },
  { "ppc601", 18, 1, 2, "PowerPC 601" },
  { "ppc603", 18, 3, 2, "PowerPC 603" },
  { "ppc603e", 18, 4, 2, "PowerPC 603e" },
  { "ppc603ev", 18, 5, 2, "PowerPC 603ev" },
  { "ppc604", 18, 6, 2, "PowerPC 604" },
  { "ppc604e", 18, 7, 2, "PowerPC 604e" },
  { "ppc750", 18, 9, 2, "PowerPC 750" },
  { "ppc7400", 18, 10, 2, "PowerPC 7400" },
  { "ppc7450", 18, 11, 2, "PowerPC 7450" },
  { "ppc970", 18, 100, 2, "PowerPC 970" },
  { "ppc970-64", 16777234, 100, 2, "PowerPC 970 64-bit" },
  { "armv4t", 12, 5, 1, "arm v4t" },
  { "armv5", 12, 7, 1, "arm v5" },
  { "xscale", 12, 8, 1, "arm xscale" },
  { "armv6", 12, 6, 1, "arm v6" },
  { "armv7", 12, 9, 1, "arm v7" },
  { "little", -1, 0, 1, "Little Endian" },
  { "big", -1, 1, 2, "Big Endian" },
  { "veo1", 255, 1, 2, "veo 1" },
  { "veo2", 255, 2, 2, "veo 2" },
  { NULL, 0, 0, 0, NULL },
};

const NXArchInfo* NXGetAllArchInfos()
{
	return __darwin_all_arch_infos;
}

const NXArchInfo* NXGetLocalArchInfo()
{
#if defined(__i386__)
	return &__darwin_all_arch_infos[1];
#elif defined(__x86_64__)
	return &__darwin_all_arch_infos[2];
#else
#	error Unsupported architecture!
#endif
}

const NXArchInfo* NXGetArchInfoFromName(const char *name)
{
	NXArchInfo* info = __darwin_all_arch_infos;
	for (; info->name; info++)
	{
		if (!strcmp(info->name, name))
    		return info;
	}
	return 0;
}



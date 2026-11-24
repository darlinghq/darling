// Host test: the partition scheme a disk image's whole disk has, as hdiutil attach names it on macOS 26.7.3
// (tests/dmg-attach.md): "GUID_partition_scheme", "Apple_partition_scheme", or "" for an image without a partition
// map. Both the UDIF images and raw (UDRW) images are read.
// Build and run (from the darling repository root; needs ICU, libxml2, zlib and bzip2 development files):
//   d=src/external/darling-dmg/src; t=$(mktemp -d)
//   g++ -std=c++17 -D_FILE_OFFSET_BITS=64 -I$d -I/usr/include/libxml2 tests/dmg-partition-scheme-host.cpp \
//       $(ls $d/*.cpp | grep -v 'main-\|darling_dmg') -licuuc -lxml2 -lz -lbz2 -o $t/scheme
//   gunzip -c tests/fixtures/apm-udrw.dmg.gz > $t/apm-udrw.dmg; gunzip -c tests/fixtures/nolayout-udrw.dmg.gz > $t/nolayout-udrw.dmg
//   $t/scheme tests/fixtures/unmount-test.dmg tests/fixtures/apm.dmg tests/fixtures/nolayout.dmg $t/apm-udrw.dmg $t/nolayout-udrw.dmg
#include "DiskImage.h"
#include <cstdio>
#include <exception>
#include <string>

int main(int argc, char** argv)
{
	if (argc != 6)
		return 2;
	const char* expected[] = { "GUID_partition_scheme", "Apple_partition_scheme", "", "Apple_partition_scheme", "" };
	int failures = 0;

	for (int i = 0; i < 5; i++)
	{
		std::string scheme = "(not opened)";
		try
		{
			DiskImage image;
			openDiskImage(argv[i + 1], image);
			scheme = partitionScheme(image);
		}
		catch (const std::exception& e)
		{
			scheme = std::string("(") + e.what() + ")";
		}
		bool ok = scheme == expected[i];
		printf("%s: %s: '%s'%s%s%s\n", ok ? "PASS" : "FAIL", argv[i + 1], scheme.c_str(), ok ? "" : ", expected '",
			ok ? "" : expected[i], ok ? "" : "'");
		failures += !ok;
	}
	return failures != 0;
}

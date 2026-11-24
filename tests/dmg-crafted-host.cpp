// Host test, built with AddressSanitizer, that damaged images are read within bounds or refused.
// Build and run (from the darling repository root; needs ICU, libxml2, zlib and bzip2 development files):
//   d=src/external/darling-dmg/src
//   g++ -std=c++17 -g -fsanitize=address -D_FILE_OFFSET_BITS=64 -I$d -I/usr/include/libxml2 \
//       tests/dmg-crafted-host.cpp $(ls $d/*.cpp | grep -v 'main-\|darling_dmg') \
//       -licuuc -lxml2 -lz -lbz2 -o /tmp/dmg-crafted-host
//   gunzip -c tests/fixtures/long-name-key.hfs.gz > /tmp/long-name-key.hfs && \
//       ASAN_OPTIONS=detect_leaks=0 /tmp/dmg-crafted-host /tmp/long-name-key.hfs tests/fixtures/bad-blkx.dmg
// (leak checking is off: the readers leak in places this test is not about)
#include "DiskImage.h"
#include <cstdio>
#include <exception>

int main(int argc, char** argv)
{
	if (argc != 3)
		return 2;
	int failures = 0;

	try
	{
		DiskImage image;
		openDiskImage(argv[1], image);
		std::string name = image.volume->volumeName();
		bool ok = name.size() <= 765;
		printf("%s: volume name of a key claiming 65535 characters (%zu bytes)\n", ok ? "PASS" : "FAIL", name.size());
		failures += !ok;
	}
	catch (const std::exception& e)
	{
		printf("PASS: volume with a damaged name refused (%s)\n", e.what());
	}

	try
	{
		DiskImage image;
		openDiskImage(argv[2], image);
		printf("FAIL: image with corrupt partition table data opened\n");
		failures++;
	}
	catch (const std::exception& e)
	{
		printf("PASS: image with corrupt partition table data refused (%s)\n", e.what());
	}

	return failures ? 1 : 0;
}

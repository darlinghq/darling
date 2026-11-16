// Host unit test for how libdarling-dmg names mount points (darling-dmg src/MountPointName.cpp).
// Build and run (from the darling repository root):
//   g++ -std=c++11 -Isrc/external/darling-dmg/src tests/dmg-mount-point-name.cpp \
//       src/external/darling-dmg/src/MountPointName.cpp -o /tmp/dmg-mount-point-name && /tmp/dmg-mount-point-name
#include "MountPointName.h"
#include <cerrno>
#include <cstdio>
#include <cstdlib>
#include <string>
#include <sys/stat.h>
#include <unistd.h>

static int failures;

static void check(bool ok, const char* what)
{
	printf("%s: %s\n", ok ? "PASS" : "FAIL", what);
	if (!ok)
		failures++;
}

int main()
{
	check(mountPointName("Unmount Test") == "Unmount Test", "plain name kept");
	check(mountPointName("A/B") == "A:B", "slash shown as colon");
	check(mountPointName("") == "untitled", "empty name");
	check(mountPointName(".") == "untitled" && mountPointName("..") == "untitled", "dot names");
	check(mountPointName(std::string("\0", 1)) == "untitled", "a name of only NUL");
	check(mountPointName(std::string("A\0B", 3)) == "AB", "NUL inside a name is dropped");
	check(mountPointName(std::string("A\0B", 3)).find('\0') == std::string::npos, "no NUL in the result");

	char root[] = "/tmp/dmg-mount-point-name-XXXXXX";
	if (mkdtemp(root) == nullptr)
		return 2;
	std::string mp;
	check(makeMountPoint(root, "X", mp) == 0 && mp == std::string(root) + "/X", "first mount point");
	check(makeMountPoint(root, "X", mp) == 0 && mp == std::string(root) + "/X 2", "taken name, outside /Volumes: 2");

	// every candidate taken: give up instead of trying forever
	for (int i = 3; i <= 1100; i++)
		mkdir((std::string(root) + "/X " + std::to_string(i)).c_str(), 0777);
	int err = makeMountPoint(root, "X", mp);
	check(err == EEXIST, "gives up after a bounded number of names");

	std::string cleanup = std::string("rm -rf '") + root + "'";
	system(cleanup.c_str());
	return failures ? 1 : 0;
}

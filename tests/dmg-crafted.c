// Damaged disk images must not crash libdarling-dmg or make it read out of bounds.
// Usage: dmg-crafted <long-name-key.hfs> <bad-blkx.dmg>   (see fixtures/make-crafted-fixtures.py)
#include <stdio.h>
#include <string.h>

#include <darling-dmg/darling_dmg.h>

static int failures;

static void check(int ok, const char* what)
{
	printf("%s: %s\n", ok ? "PASS" : "FAIL", what);
	if (!ok)
		failures++;
}

int main(int argc, char** argv)
{
	if (argc != 3)
		return 2;

	// An HFS+ name is at most 255 UTF-16 units (at most 765 bytes of UTF-8), whatever the key claims.
	darling_dmg_image* image = NULL;
	int err = darling_dmg_open(argv[1], &image);
	if (err == 0) {
		const char* name = darling_dmg_volume_name(image);
		check(strlen(name) <= 765, "volume name of a key claiming 65535 characters stays within 255");
		printf("  name length: %zu\n", strlen(name));
		darling_dmg_close(image);
	} else
		check(1, "volume name of a key claiming 65535 characters stays within 255 (image refused)");

	image = NULL;
	err = darling_dmg_open(argv[2], &image);
	check(err != 0 && image == NULL, "image with corrupt partition table data is refused");
	if (image)
		darling_dmg_close(image);

	return failures ? 1 : 0;
}

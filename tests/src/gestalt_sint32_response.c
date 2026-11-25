// Gestalt takes an OSType selector and an SInt32 * response, as in Apple's Gestalt.h. In 32-bit
// processes SInt32 is `long`, so a client passing the address of an SInt32 must compile without
// a pointer-type diagnostic. Build for i386 and x86_64 with -Werror=incompatible-pointer-types
// and run with `darling shell`. Exits non-zero on failure.
#include <CoreServices/CoreServices.h>
#include <stdio.h>

static int g_failures = 0;

#define CHECK(cond) do { if (!(cond)) { printf("FAIL %d: %s\n", __LINE__, #cond); g_failures++; } } while (0)

int main(void)
{
	SInt32 response = 0;
	OSErr (*entry)(OSType, SInt32*) = Gestalt;

	CHECK(entry(gestaltSystemVersion, &response) == noErr);
	CHECK(response >= 0x1000);
	response = 0x5a5a5a5a;
	CHECK(Gestalt('?no?', &response) != noErr);
	printf("%s\n", g_failures ? "FAILED" : "PASSED");
	return g_failures != 0;
}

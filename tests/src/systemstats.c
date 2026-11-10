// Guest test: libsystemstats reports no recorded history, since Darling runs no systemstatsd.
// Build against /usr/lib/libsystemstats.dylib and run inside Darling; exits non-zero on failure.
#include <stdio.h>

void *systemstats_get_top_coalitions();
void *systemstats_get_battery_charge_graph();

int main(void)
{
	int failures = 0;
	if (systemstats_get_top_coalitions() != NULL) {
		fprintf(stderr, "FAIL: top coalitions reported without systemstatsd\n");
		failures++;
	}
	if (systemstats_get_battery_charge_graph() != NULL) {
		fprintf(stderr, "FAIL: battery history reported without systemstatsd\n");
		failures++;
	}
	printf("%s\n", failures ? "FAIL" : "PASS");
	return failures != 0;
}

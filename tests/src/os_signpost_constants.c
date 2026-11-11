// Copyright (C) 2026 Lubos Dolezel
//
// Values of the public os_signpost_type_t enumeration and the signpost ID
// constants, as in the macOS SDK <os/signpost.h>: the type is an 8-bit
// unsigned integer, EVENT is 0, INTERVAL_BEGIN is 1, INTERVAL_END is 2,
// OS_SIGNPOST_ID_NULL is 0, OS_SIGNPOST_ID_INVALID is all ones and
// OS_SIGNPOST_ID_EXCLUSIVE is 0xEEEEB0B5B2B2EEEE. The checks are made at
// compile time, so the program only has to build and run to pass.
#include <os/signpost.h>
#include <stdint.h>
#include <stdio.h>

_Static_assert(sizeof(os_signpost_type_t) == 1, "os_signpost_type_t is 8 bits wide");
_Static_assert((os_signpost_type_t) -1 > 0, "os_signpost_type_t is unsigned");
_Static_assert(OS_SIGNPOST_EVENT == 0, "OS_SIGNPOST_EVENT");
_Static_assert(OS_SIGNPOST_INTERVAL_BEGIN == 1, "OS_SIGNPOST_INTERVAL_BEGIN");
_Static_assert(OS_SIGNPOST_INTERVAL_END == 2, "OS_SIGNPOST_INTERVAL_END");
_Static_assert(OS_SIGNPOST_ID_NULL == 0, "OS_SIGNPOST_ID_NULL");
_Static_assert(OS_SIGNPOST_ID_INVALID == UINT64_MAX, "OS_SIGNPOST_ID_INVALID");
_Static_assert(OS_SIGNPOST_ID_EXCLUSIVE == UINT64_C(0xEEEEB0B5B2B2EEEE), "OS_SIGNPOST_ID_EXCLUSIVE");
_Static_assert(sizeof(os_signpost_id_t) == 8, "os_signpost_id_t is 64 bits wide");

int main(void)
{
	os_signpost_type_t type = OS_SIGNPOST_INTERVAL_END;
	os_signpost_id_t id = OS_SIGNPOST_ID_EXCLUSIVE;
	if (type != 2 || id != UINT64_C(0xEEEEB0B5B2B2EEEE)) {
		printf("FAILED\n");
		return 1;
	}
	printf("ALL PASSED\n");
	return 0;
}

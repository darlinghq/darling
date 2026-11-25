/* SPDX-License-Identifier: GPL-3.0-or-later */
#include <CoreServices/FileManager.h>
#include <stddef.h>

_Static_assert(sizeof(FSSpec) == 70, "FSSpec size");
#if !__LP64__
_Static_assert(_Alignof(FSSpec) == 2, "FSSpec alignment");
_Static_assert(offsetof(FSSpec, vRefNum) == 0, "volume offset");
_Static_assert(offsetof(FSSpec, parID) == 2, "parent offset");
_Static_assert(offsetof(FSSpec, name) == 6, "Pascal string offset");
_Static_assert(sizeof(((FSSpec *)0)->name) == 64, "Str63 capacity");
struct AfterFSSpec { char first; int second; };
_Static_assert(offsetof(struct AfterFSSpec, second) == 4, "packing restored");
#endif

int main(void)
{
#if !__LP64__
    FSSpec spec = {-1, 2, {3, 'a', 'i', 'f'}};
    return spec.name[0] != 3 || spec.name[3] != 'f';
#else
    return 0;
#endif
}

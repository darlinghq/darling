#include <stdint.h>
#include <stddef.h>
#include "elfcalls_setup.h"
#include <elfcalls.h>

extern int strncmp(const char *s1, const char *s2, __SIZE_TYPE__ n);
extern unsigned long long __simple_atoi16(const char* str, const char** endp);

__attribute__((visibility("default")))
struct elf_calls* _elfcalls;

void setup_elfcalls(const char** apple)
{
	int i;
	for (i = 0; apple[i] != NULL; i++)
	{
		if (strncmp(apple[i], "elf_calls=", 10) == 0)
		{
			uintptr_t table = (uintptr_t) __simple_atoi16(apple[i] + 12, NULL);
			_elfcalls = (struct elf_calls*) table;
		}
	}
}


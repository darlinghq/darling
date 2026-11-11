// Copyright (C) 2026 Lubos Dolezel
//
// macho_cpu_type_for_arch_name, macho_arch_name_for_cpu_type and
// macho_arch_name_for_mach_header from <mach-o/utils.h> (libdyld).
// Expected values were observed on macOS 27 (arm64).

#include <mach-o/utils.h>
#include <mach-o/loader.h>
#include <mach-o/dyld.h>
#include <stdio.h>
#include <string.h>

static int g_failures = 0;

static void check_name(cpu_type_t type, cpu_subtype_t subtype, const char* expected)
{
	const char* name = macho_arch_name_for_cpu_type(type, subtype);
	int ok = expected ? (name != NULL && strcmp(name, expected) == 0) : name == NULL;
	if (!ok) {
		printf("FAIL: macho_arch_name_for_cpu_type(%#x, %#x) = %s, expected %s\n",
			type, subtype, name ? name : "NULL", expected ? expected : "NULL");
		g_failures++;
	}
}

static void check_type(const char* name, bool expected_ok, cpu_type_t expected_type, cpu_subtype_t expected_subtype)
{
	cpu_type_t type = -7;
	cpu_subtype_t subtype = -7;
	bool ok = macho_cpu_type_for_arch_name(name, &type, &subtype);
	if (!expected_ok) {
		// unknown names leave the outputs alone
		expected_type = -7;
		expected_subtype = -7;
	}
	if (ok != expected_ok || type != expected_type || subtype != expected_subtype) {
		printf("FAIL: macho_cpu_type_for_arch_name(\"%s\") = %d %#x %#x\n", name, ok, type, subtype);
		g_failures++;
	}
}

int main(void)
{
	check_name(CPU_TYPE_ARM64, CPU_SUBTYPE_ARM64_ALL, "arm64");
	check_name(CPU_TYPE_ARM64, CPU_SUBTYPE_ARM64_V8, "arm64");
	check_name(CPU_TYPE_ARM64, CPU_SUBTYPE_ARM64E, "arm64e");
	check_name(CPU_TYPE_ARM64, CPU_SUBTYPE_ARM64E | CPU_SUBTYPE_PTRAUTH_ABI, "arm64e");
	check_name(CPU_TYPE_ARM64, 0x81000002, "arm64e");
	check_name(CPU_TYPE_ARM64, CPU_SUBTYPE_PTRAUTH_ABI, NULL);
	check_name(CPU_TYPE_ARM64, CPU_SUBTYPE_PTRAUTH_ABI | CPU_SUBTYPE_ARM64_V8, NULL);
	check_name(CPU_TYPE_ARM64, 5, NULL);
	check_name(CPU_TYPE_X86_64, CPU_SUBTYPE_X86_64_ALL, "x86_64");
	check_name(CPU_TYPE_X86_64, CPU_SUBTYPE_X86_64_ALL | CPU_SUBTYPE_LIB64, "x86_64");
	check_name(CPU_TYPE_X86_64, CPU_SUBTYPE_X86_64_H, "x86_64h");
	check_name(CPU_TYPE_X86_64, 0, NULL);
	check_name(CPU_TYPE_I386, CPU_SUBTYPE_I386_ALL, "i386");
	check_name(CPU_TYPE_I386, 0, NULL);
	check_name(CPU_TYPE_ARM, CPU_SUBTYPE_ARM_V7, "armv7");
	check_name(CPU_TYPE_ARM, CPU_SUBTYPE_ARM_V7S, "armv7s");
	check_name(CPU_TYPE_ARM, CPU_SUBTYPE_ARM_V7K, "armv7k");
	check_name(CPU_TYPE_ARM, CPU_SUBTYPE_ARM_V6, "armv6");
	check_name(CPU_TYPE_ARM, CPU_SUBTYPE_ARM_V4T, "armv4t");
	check_name(CPU_TYPE_ARM, CPU_SUBTYPE_ARM_V7EM, "armv7em");
	check_name(CPU_TYPE_ARM, CPU_SUBTYPE_ARM_ALL, NULL);
	check_name(CPU_TYPE_ARM, CPU_SUBTYPE_ARM_V8, NULL);
	check_name(CPU_TYPE_ARM64_32, CPU_SUBTYPE_ARM64_32_V8, "arm64_32");
	check_name(CPU_TYPE_POWERPC, CPU_SUBTYPE_POWERPC_ALL, "ppc");
	check_name(CPU_TYPE_POWERPC64, CPU_SUBTYPE_POWERPC_ALL, NULL);
	check_name(12345, 0, NULL);
	check_name(CPU_TYPE_ANY, 0, NULL);

	check_type("arm64", true, CPU_TYPE_ARM64, CPU_SUBTYPE_ARM64_ALL);
	check_type("arm64e", true, CPU_TYPE_ARM64, CPU_SUBTYPE_ARM64E | CPU_SUBTYPE_PTRAUTH_ABI);
	check_type("x86_64", true, CPU_TYPE_X86_64, CPU_SUBTYPE_X86_64_ALL);
	check_type("x86_64h", true, CPU_TYPE_X86_64, CPU_SUBTYPE_X86_64_H);
	check_type("i386", true, CPU_TYPE_I386, CPU_SUBTYPE_I386_ALL);
	check_type("armv7k", true, CPU_TYPE_ARM, CPU_SUBTYPE_ARM_V7K);
	check_type("arm64_32", true, CPU_TYPE_ARM64_32, CPU_SUBTYPE_ARM64_32_V8);
	check_type("ppc", true, CPU_TYPE_POWERPC, CPU_SUBTYPE_POWERPC_ALL);
	check_type("ARM64", false, 0, 0);
	check_type("x86", false, 0, 0);
	check_type("", false, 0, 0);
	check_type("bogus", false, 0, 0);

	// NULL means the main executable
	const char* own = macho_arch_name_for_mach_header((const struct mach_header*) _dyld_get_image_header(0));
	const char* main_name = macho_arch_name_for_mach_header(NULL);
	if (own == NULL || main_name == NULL || strcmp(own, main_name) != 0) {
		printf("FAIL: macho_arch_name_for_mach_header: %s vs %s\n", own ? own : "NULL", main_name ? main_name : "NULL");
		g_failures++;
	}
#if defined(__x86_64__)
	if (own == NULL || strncmp(own, "x86_64", 6) != 0) {
#elif defined(__arm64__)
	if (own == NULL || strncmp(own, "arm64", 5) != 0) {
#elif defined(__i386__)
	if (own == NULL || strcmp(own, "i386") != 0) {
#else
	if (0) {
#endif
		printf("FAIL: unexpected architecture name %s\n", own ? own : "NULL");
		g_failures++;
	}

	// not a Mach-O header
	static const uint32_t not_macho[8] = { 0x213c6172, 0x63683e0a };
	if (macho_arch_name_for_mach_header((const struct mach_header*) not_macho) != NULL) {
		printf("FAIL: a non-Mach-O header has an architecture name\n");
		g_failures++;
	}

	if (g_failures == 0)
		printf("PASS\n");
	return g_failures ? 1 : 0;
}

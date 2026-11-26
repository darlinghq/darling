// Copyright (C) 2026 Lubos Dolezel
//
// libc++.1.dylib must keep exporting what earlier releases of it exported and
// later libc++ sources no longer define, so that binaries built against older
// libc++ headers still bind: the tag objects that became inline variables, the
// std::sort helpers the LLVM 15 headers declared as extern templates, the key
// function, vtable and type info of std::format_error, the legacy debug mode
// support, and (on i386) the messages members with the catalog type of i386
// macOS (int). The sort helpers are also called, to check what they do: the
// LLVM 15 std::sort relies on __insertion_sort_incomplete returning true only
// for a sorted range and on __sort5 sorting five elements.
//   clang -o /tmp/libcxx-removed-symbols tests/libcxx-removed-symbols.c   (run inside Darling; any arch)
// Exits non-zero on failure.
#include <dlfcn.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static int fail;

static void check(bool ok, const char* what)
{
	printf("%s: %s\n", ok ? "PASS" : "FAIL", what);
	fail |= !ok;
}

static const char* const names[] = {
	"_ZNSt3__110adopt_lockE",
	"_ZNSt3__110defer_lockE",
	"_ZNSt3__111try_to_lockE",
	"_ZNSt3__113allocator_argE",
	"_ZNSt3__119piecewise_constructE",
	"_ZNSt3__112format_errorD0Ev",
	"_ZNSt3__112format_errorD1Ev",
	"_ZNSt3__112format_errorD2Ev",
	"_ZTINSt3__112format_errorE",
	"_ZTSNSt3__112format_errorE",
	"_ZTVNSt3__112format_errorE",
	"_ZNSt3__18__get_dbEv",
	"_ZNSt3__114__get_const_dbEv",
	"_ZNSt3__123__libcpp_debug_functionE",
	"_ZNSt3__127__libcpp_set_debug_functionEPFvRKNS_19__libcpp_debug_infoEE",
	"_ZNSt3__129__libcpp_abort_debug_functionERKNS_19__libcpp_debug_infoE",
	"_ZNKSt3__119__libcpp_debug_info4whatEv",
	"_ZNSt3__111__libcpp_db10__insert_iEPv",
	"_ZNSt3__111__libcpp_db9__erase_cEPv",
	"_ZNSt3__18__c_node5__addEPNS_8__i_nodeE",
	"_ZTVNSt3__18__c_nodeE",
	"_ZNSt3__127__insertion_sort_incompleteIRNS_6__lessIccEEPcEEbT0_S5_T_",
	"_ZNSt3__127__insertion_sort_incompleteIRNS_6__lessIwwEEPwEEbT0_S5_T_",
	"_ZNSt3__127__insertion_sort_incompleteIRNS_6__lessIaaEEPaEEbT0_S5_T_",
	"_ZNSt3__127__insertion_sort_incompleteIRNS_6__lessIhhEEPhEEbT0_S5_T_",
	"_ZNSt3__127__insertion_sort_incompleteIRNS_6__lessIssEEPsEEbT0_S5_T_",
	"_ZNSt3__127__insertion_sort_incompleteIRNS_6__lessIttEEPtEEbT0_S5_T_",
	"_ZNSt3__127__insertion_sort_incompleteIRNS_6__lessIiiEEPiEEbT0_S5_T_",
	"_ZNSt3__127__insertion_sort_incompleteIRNS_6__lessIjjEEPjEEbT0_S5_T_",
	"_ZNSt3__127__insertion_sort_incompleteIRNS_6__lessIllEEPlEEbT0_S5_T_",
	"_ZNSt3__127__insertion_sort_incompleteIRNS_6__lessImmEEPmEEbT0_S5_T_",
	"_ZNSt3__127__insertion_sort_incompleteIRNS_6__lessIxxEEPxEEbT0_S5_T_",
	"_ZNSt3__127__insertion_sort_incompleteIRNS_6__lessIyyEEPyEEbT0_S5_T_",
	"_ZNSt3__127__insertion_sort_incompleteIRNS_6__lessIffEEPfEEbT0_S5_T_",
	"_ZNSt3__127__insertion_sort_incompleteIRNS_6__lessIddEEPdEEbT0_S5_T_",
	"_ZNSt3__127__insertion_sort_incompleteIRNS_6__lessIeeEEPeEEbT0_S5_T_",
	"_ZNSt3__17__sort5IRNS_6__lessIeeEEPeEEjT0_S5_S5_S5_S5_T_",
#ifdef __i386__
	"_ZNKSt3__18messagesIcE6do_getEiiiRKNS_12basic_stringIcNS_11char_traitsIcEENS_9allocatorIcEEEE",
	"_ZNKSt3__18messagesIcE8do_closeEi",
	"_ZNKSt3__18messagesIwE6do_getEiiiRKNS_12basic_stringIwNS_11char_traitsIwEENS_9allocatorIwEEEE",
	"_ZNKSt3__18messagesIwE8do_closeEi",
#endif
};

typedef bool (*insertion_sort_incomplete_int)(int*, int*, void*);
typedef unsigned (*sort5_long_double)(long double*, long double*, long double*, long double*, long double*, void*);

static bool sorted(const int* v, int n)
{
	for (int i = 1; i < n; i++)
		if (v[i] < v[i - 1])
			return false;
	return true;
}

// the multiset of the values must not change
static bool same_values(const int* a, const int* b, int n)
{
	int count[64] = { 0 };
	for (int i = 0; i < n; i++) {
		count[a[i]]++;
		count[b[i]]--;
	}
	for (int i = 0; i < 64; i++)
		if (count[i])
			return false;
	return true;
}

int main(void)
{
	void* h = dlopen("/usr/lib/libc++.1.dylib", RTLD_NOW);
	if (!h) {
		printf("FAIL: dlopen libc++\n");
		return 1;
	}
	char what[256];
	for (unsigned i = 0; i < sizeof names / sizeof *names; i++) {
		snprintf(what, sizeof what, "export %s", names[i]);
		check(dlsym(h, names[i]) != NULL, what);
	}

	// The comparator is std::__less, an empty type passed by reference.
	char less;
	insertion_sort_incomplete_int isi = (insertion_sort_incomplete_int)
		dlsym(h, "_ZNSt3__127__insertion_sort_incompleteIRNS_6__lessIiiEEPiEEbT0_S5_T_");
	if (isi) {
		// every length up to 40, values in reverse, random and almost sorted order
		srand(1);
		for (int n = 0; n <= 40; n++) {
			for (int kind = 0; kind < 3; kind++) {
				int v[40], orig[40];
				for (int i = 0; i < n; i++) {
					v[i] = kind == 0 ? n - i : kind == 1 ? rand() % 64 : i;
					if (kind == 2 && i > 0 && i % 7 == 0) {
						v[i] = v[i - 1];
						v[i - 1] = i;
					}
				}
				memcpy(orig, v, sizeof(int) * n);
				bool done = isi(v, v + n, &less);
				snprintf(what, sizeof what, "__insertion_sort_incomplete n=%d order=%d keeps the values", n, kind);
				if (!same_values(v, orig, n))
					check(false, what);
				// true only for a sorted range; short ranges are always sorted
				snprintf(what, sizeof what, "__insertion_sort_incomplete n=%d order=%d result", n, kind);
				if ((done && !sorted(v, n)) || (n <= 5 && !done))
					check(false, what);
			}
		}
		check(true, "__insertion_sort_incomplete results");
		// an almost sorted range is finished within the move limit
		int few[10] = { 0, 1, 2, 4, 3, 5, 6, 8, 7, 9 };
		check(isi(few, few + 10, &less) && sorted(few, 10), "__insertion_sort_incomplete finishes an almost sorted range");
	}

	sort5_long_double s5 = (sort5_long_double)dlsym(h, "_ZNSt3__17__sort5IRNS_6__lessIeeEEPeEEjT0_S5_S5_S5_S5_T_");
	if (s5) {
		long double a[5] = { 5, 4, 3, 2, 1 };
		unsigned swaps = s5(a, a + 1, a + 2, a + 3, a + 4, &less);
		check(a[0] == 1 && a[1] == 2 && a[2] == 3 && a[3] == 4 && a[4] == 5, "__sort5 sorts five elements");
		check(swaps > 0, "__sort5 counts its swaps");
		long double b[5] = { 1, 2, 3, 4, 5 };
		check(s5(b, b + 1, b + 2, b + 3, b + 4, &less) == 0, "__sort5 of a sorted range swaps nothing");
	}
	return fail;
}

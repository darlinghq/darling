// Host-side test of darling-prefsd's checks on the system preferences files
// and their values (src/prefsd/prefsd-validate.c).
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#include "prefsd-validate.h"

static int g_failures = 0;

#define CHECK(cond) do { \
		if (!(cond)) { printf("FAIL %d: %s\n", __LINE__, #cond); g_failures++; } \
	} while (0)

static struct stat file_with(mode_t mode, uid_t uid, off_t size)
{
	struct stat st;

	memset(&st, 0, sizeof(st));
	st.st_mode = mode;
	st.st_uid = uid;
	st.st_size = size;
	return st;
}

static void test_files(void)
{
	struct stat st;

	st = file_with(S_IFREG | 0644, 0, 100);
	CHECK(prefsd_check_file(&st) == PREFSD_FILE_OK);
	st = file_with(S_IFREG | 0600, 0, 0);
	CHECK(prefsd_check_file(&st) == PREFSD_FILE_OK);
	st = file_with(S_IFREG | 0444, 0, PREFSD_FILE_SIZE_MAX);
	CHECK(prefsd_check_file(&st) == PREFSD_FILE_OK);

	st = file_with(S_IFREG | 0664, 0, 100);
	CHECK(prefsd_check_file(&st) == PREFSD_FILE_WRITABLE_BY_OTHERS);
	st = file_with(S_IFREG | 0646, 0, 100);
	CHECK(prefsd_check_file(&st) == PREFSD_FILE_WRITABLE_BY_OTHERS);
	st = file_with(S_IFREG | 0666, 0, 100);
	CHECK(prefsd_check_file(&st) == PREFSD_FILE_WRITABLE_BY_OTHERS);

	st = file_with(S_IFREG | 0644, 501, 100);
	CHECK(prefsd_check_file(&st) == PREFSD_FILE_NOT_ROOT_OWNED);

	st = file_with(S_IFDIR | 0755, 0, 100);
	CHECK(prefsd_check_file(&st) == PREFSD_FILE_NOT_REGULAR);
	st = file_with(S_IFIFO | 0644, 0, 0);
	CHECK(prefsd_check_file(&st) == PREFSD_FILE_NOT_REGULAR);
	st = file_with(S_IFLNK | 0777, 0, 10);
	CHECK(prefsd_check_file(&st) == PREFSD_FILE_NOT_REGULAR);

	st = file_with(S_IFREG | 0644, 0, PREFSD_FILE_SIZE_MAX + 1);
	CHECK(prefsd_check_file(&st) == PREFSD_FILE_TOO_LARGE);

	CHECK(strcmp(prefsd_file_verdict_string(PREFSD_FILE_WRITABLE_BY_OTHERS), "writable by its group or by others") == 0);
}

static void test_strings(void)
{
	char long_string[64];

	CHECK(prefsd_is_printable_ascii("Macmini8,1", 10, 64));
	CHECK(prefsd_is_printable_ascii("Intel(R) Core(TM) i9 ~", 22, 64));
	CHECK(!prefsd_is_printable_ascii("", 0, 64));
	CHECK(!prefsd_is_printable_ascii("tab\there", 8, 64));
	CHECK(!prefsd_is_printable_ascii("new\nline", 8, 64));
	CHECK(!prefsd_is_printable_ascii("nul\0byte", 8, 64));
	CHECK(!prefsd_is_printable_ascii("del\x7f", 4, 64));
	CHECK(!prefsd_is_printable_ascii("caf\xc3\xa9", 5, 64));

	memset(long_string, 'A', sizeof(long_string));
	CHECK(prefsd_is_printable_ascii(long_string, 63, 64));
	CHECK(!prefsd_is_printable_ascii(long_string, 64, 64));
}

static void test_uuids(void)
{
	static const uint8_t expected[16] = {
		0x12, 0x34, 0x56, 0x78, 0x9a, 0xbc, 0xde, 0xf0,
		0x0f, 0xed, 0xcb, 0xa9, 0x87, 0x65, 0x43, 0x21,
	};
	uint8_t uuid[16];
	uint8_t untouched[16];
	const char* text;

	text = "12345678-9abc-def0-0fed-cba987654321";
	CHECK(prefsd_parse_uuid(text, strlen(text), uuid));
	CHECK(memcmp(uuid, expected, sizeof(uuid)) == 0);

	text = "12345678-9ABC-DEF0-0FED-CBA987654321";
	CHECK(prefsd_parse_uuid(text, strlen(text), uuid));
	CHECK(memcmp(uuid, expected, sizeof(uuid)) == 0);

	text = "00000000-0000-0000-0000-000000000000";
	CHECK(prefsd_parse_uuid(text, strlen(text), uuid));
	CHECK(prefsd_is_all_zero(uuid, sizeof(uuid)));
	CHECK(!prefsd_is_all_zero(expected, sizeof(expected)));

	// a rejected string leaves the output alone
	memset(untouched, 0xaa, sizeof(untouched));
	memcpy(uuid, untouched, sizeof(uuid));
	text = "12345678-9abc-def0-0fed-cba98765432g";
	CHECK(!prefsd_parse_uuid(text, strlen(text), uuid));
	CHECK(memcmp(uuid, untouched, sizeof(uuid)) == 0);

	text = "123456789abcdef00fedcba987654321";
	CHECK(!prefsd_parse_uuid(text, strlen(text), uuid));
	text = "{12345678-9abc-def0-0fed-cba987654321}";
	CHECK(!prefsd_parse_uuid(text, strlen(text), uuid));
	text = " 12345678-9abc-def0-0fed-cba98765432";
	CHECK(!prefsd_parse_uuid(text, strlen(text), uuid));
	text = "12345678-9abc-def0-0fed_cba987654321";
	CHECK(!prefsd_parse_uuid(text, strlen(text), uuid));
	text = "12345678-9abc-def00fed-cba9876543210";
	CHECK(!prefsd_parse_uuid(text, strlen(text), uuid));
	CHECK(!prefsd_parse_uuid("", 0, uuid));
}

static void test_paths(void)
{
	char long_path[1024];

	CHECK(prefsd_is_absolute_path("/var/db/trust", 13, 1024));
	CHECK(prefsd_is_absolute_path("/", 1, 1024));
	CHECK(!prefsd_is_absolute_path("var/db/trust", 12, 1024));
	CHECK(!prefsd_is_absolute_path("", 0, 1024));
	CHECK(!prefsd_is_absolute_path("/var\0/db", 8, 1024));

	memset(long_path, 'a', sizeof(long_path));
	long_path[0] = '/';
	CHECK(prefsd_is_absolute_path(long_path, 1023, 1024));
	CHECK(!prefsd_is_absolute_path(long_path, 1024, 1024));
}

static void test_policies(void)
{
	enum prefsd_code_signing_policy policy = PREFSD_CODE_SIGNING_WARN;

	CHECK(prefsd_parse_code_signing_policy("off", 3, &policy) && policy == PREFSD_CODE_SIGNING_OFF);
	CHECK(prefsd_parse_code_signing_policy("warn", 4, &policy) && policy == PREFSD_CODE_SIGNING_WARN);
	CHECK(prefsd_parse_code_signing_policy("enforce", 7, &policy) && policy == PREFSD_CODE_SIGNING_ENFORCE);

	policy = PREFSD_CODE_SIGNING_WARN;
	CHECK(!prefsd_parse_code_signing_policy("Enforce", 7, &policy));
	CHECK(!prefsd_parse_code_signing_policy("enforc", 6, &policy));
	CHECK(!prefsd_parse_code_signing_policy("enforce ", 8, &policy));
	CHECK(!prefsd_parse_code_signing_policy("", 0, &policy));
	CHECK(!prefsd_parse_code_signing_policy("of", 2, &policy));
	CHECK(policy == PREFSD_CODE_SIGNING_WARN);
}

static void test_version_formats(void)
{
	CHECK(prefsd_is_valid_os_product_version("26.4", 4));
	CHECK(prefsd_is_valid_os_product_version("26.7.3", 6));
	CHECK(prefsd_is_valid_os_product_version("12.6.8", 6));
	CHECK(prefsd_is_valid_os_product_version("11.0", 4));
	CHECK(prefsd_is_valid_os_product_version("10.15.7", 7));
	CHECK(prefsd_is_valid_os_product_version("10.14", 5));

	CHECK(!prefsd_is_valid_os_product_version("", 0));
	CHECK(!prefsd_is_valid_os_product_version("<script>", 8));
	CHECK(!prefsd_is_valid_os_product_version("12.6.8 ", 7));
	CHECK(!prefsd_is_valid_os_product_version(" 12.6.8", 7));
	CHECK(!prefsd_is_valid_os_product_version("12..6", 5));
	CHECK(!prefsd_is_valid_os_product_version(".12", 3));
	CHECK(!prefsd_is_valid_os_product_version("12.", 3));
	CHECK(!prefsd_is_valid_os_product_version("abc", 3));
	CHECK(!prefsd_is_valid_os_product_version("12.6.8.1", 8));

	CHECK(prefsd_is_valid_os_version("25E246", 6));
	CHECK(prefsd_is_valid_os_version("21G725", 6));
	CHECK(prefsd_is_valid_os_version("19H2", 4));
	CHECK(prefsd_is_valid_os_version("18G103", 6));

	CHECK(!prefsd_is_valid_os_version("", 0));
	CHECK(!prefsd_is_valid_os_version("<25E246>", 8));
	CHECK(!prefsd_is_valid_os_version("25E246 ", 7));
	CHECK(!prefsd_is_valid_os_version("25E 246", 7));
	CHECK(!prefsd_is_valid_os_version("25E.246", 7));

	CHECK(prefsd_is_valid_os_release("25.4.0", 6));
	CHECK(prefsd_is_valid_os_release("21.6.0", 6));
	CHECK(prefsd_is_valid_os_release("19.6.0", 6));
	CHECK(prefsd_is_valid_os_release("21.6", 4));

	CHECK(!prefsd_is_valid_os_release("", 0));
	CHECK(!prefsd_is_valid_os_release("<21.6.0>", 8));
	CHECK(!prefsd_is_valid_os_release("21.6.0 ", 7));
	CHECK(!prefsd_is_valid_os_release("21..6", 5));
	CHECK(!prefsd_is_valid_os_release("21.6.0-alpha", 12));

	CHECK(prefsd_is_valid_ios_support_version("none", 4));
	CHECK(prefsd_is_valid_ios_support_version("15.6", 4));
	CHECK(prefsd_is_valid_ios_support_version("12.2", 4));
	CHECK(!prefsd_is_valid_ios_support_version("", 0));
	CHECK(!prefsd_is_valid_ios_support_version("<15.6>", 6));
	CHECK(!prefsd_is_valid_ios_support_version("other", 5));
}

static void test_product_name_and_compat(void)
{
	CHECK(strcmp(prefsd_product_name_for_version("10.14.6"), "Mac OS X") == 0);
	CHECK(strcmp(prefsd_product_name_for_version("10.15.7"), "Mac OS X") == 0);
	CHECK(strcmp(prefsd_product_name_for_version("11.0"), "macOS") == 0);
	CHECK(strcmp(prefsd_product_name_for_version("12.6.8"), "macOS") == 0);
	CHECK(strcmp(prefsd_product_name_for_version("26.4"), "macOS") == 0);

	CHECK(!prefsd_should_have_system_version_compat("10.14.6"));
	CHECK(!prefsd_should_have_system_version_compat("10.15.7"));
	CHECK(prefsd_should_have_system_version_compat("11.0"));
	CHECK(prefsd_should_have_system_version_compat("12.6.8"));
	CHECK(prefsd_should_have_system_version_compat("26.4"));
}

static void test_version_mapping(void)
{
	char ios[64];
	char compat[64];

	// macOS 26+: iOSSupportVersion matches macOS version (major.minor)
	CHECK(prefsd_compute_ios_support_version("26.4", NULL, ios, sizeof(ios)));
	CHECK(strcmp(ios, "26.4") == 0);
	CHECK(prefsd_compute_ios_support_version("26.7.3", NULL, ios, sizeof(ios)));
	CHECK(strcmp(ios, "26.7") == 0);
	prefsd_compute_compat_product_version("26.4", compat, sizeof(compat));
	CHECK(strcmp(compat, "10.16") == 0);

	// macOS 11..15: (major + 3).minor
	CHECK(prefsd_compute_ios_support_version("12.6.8", NULL, ios, sizeof(ios)));
	CHECK(strcmp(ios, "15.6") == 0);
	prefsd_compute_compat_product_version("12.6.8", compat, sizeof(compat));
	CHECK(strcmp(compat, "10.16") == 0);

	CHECK(prefsd_compute_ios_support_version("11.0", NULL, ios, sizeof(ios)));
	CHECK(strcmp(ios, "14.0") == 0);

	// macOS 10.15: 13.x
	CHECK(prefsd_compute_ios_support_version("10.15.7", NULL, ios, sizeof(ios)));
	CHECK(strcmp(ios, "13.4") == 0);
	CHECK(prefsd_compute_ios_support_version("10.15.0", NULL, ios, sizeof(ios)));
	CHECK(strcmp(ios, "13.0") == 0);

	// macOS 10.14: 12.2
	CHECK(prefsd_compute_ios_support_version("10.14.4", NULL, ios, sizeof(ios)));
	CHECK(strcmp(ios, "12.2") == 0);
	CHECK(prefsd_compute_ios_support_version("10.14.6", NULL, ios, sizeof(ios)));
	CHECK(strcmp(ios, "12.2") == 0);

	// macOS < 10.14: omitted
	CHECK(!prefsd_compute_ios_support_version("10.13.6", NULL, ios, sizeof(ios)));
	CHECK(!prefsd_compute_ios_support_version("10.12.0", NULL, ios, sizeof(ios)));
	CHECK(!prefsd_compute_ios_support_version("10.9", NULL, ios, sizeof(ios)));

	// Explicit override
	CHECK(prefsd_compute_ios_support_version("12.6.8", "16.1", ios, sizeof(ios)));
	CHECK(strcmp(ios, "16.1") == 0);
	CHECK(!prefsd_compute_ios_support_version("12.6.8", "none", ios, sizeof(ios)));
}

int main(void)
{
	test_files();
	test_strings();
	test_uuids();
	test_paths();
	test_policies();
	test_version_formats();
	test_product_name_and_compat();
	test_version_mapping();

	if (g_failures != 0)
	{
		printf("%d check(s) failed\n", g_failures);
		return 1;
	}
	printf("all checks passed\n");
	return 0;
}

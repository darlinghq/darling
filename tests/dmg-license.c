// Licence agreements embedded in disk images, read through libdarling-dmg.
// Usage: dmg-license <sla-test.dmg> <unmount-test.dmg> <sla-short-lpic.dmg> <sla-overcount.dmg>
// The sla-*.dmg fixtures are made by fixtures/make-sla-fixture.py (see dmg-license.md).
#include <errno.h>
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

static int same(const char* a, const char* b)
{
	return a != NULL && b != NULL && strcmp(a, b) == 0;
}

static int contains(const char* haystack, const char* needle)
{
	return haystack != NULL && strstr(haystack, needle) != NULL;
}

static darling_dmg_image* openImage(const char* path)
{
	darling_dmg_image* image = NULL;
	int err = darling_dmg_open(path, &image);
	if (err != 0)
		printf("FAIL: cannot open %s: %s\n", path, strerror(err));
	return image;
}

static const char* chosen(darling_dmg_image* image, const char* const* preferred, size_t count)
{
	const struct darling_dmg_license* license = darling_dmg_license_for_languages(image, preferred, count);
	return license ? license->language : "(none)";
}

int main(int argc, char** argv)
{
	if (argc != 5) {
		fprintf(stderr, "usage: %s <sla-test.dmg> <unmount-test.dmg> <sla-short-lpic.dmg> <sla-overcount.dmg>\n", argv[0]);
		return 2;
	}

	darling_dmg_image* none = openImage(argv[2]);
	if (none) {
		const char* en[] = { "en" };
		check(darling_dmg_license_count(none) == 0, "image without an agreement: no licences");
		check(darling_dmg_license_for_languages(none, en, 1) == NULL, "image without an agreement: nothing to choose");
		darling_dmg_close(none);
	}

	darling_dmg_image* image = openImage(argv[1]);
	if (!image)
		return 1;

	check(darling_dmg_license_count(image) == 3, "three languages");

	const struct darling_dmg_license* en = darling_dmg_license_at(image, 0);
	const struct darling_dmg_license* de = darling_dmg_license_at(image, 1);
	const struct darling_dmg_license* ja = darling_dmg_license_at(image, 2);
	check(darling_dmg_license_at(image, 3) == NULL, "no fourth language");

	if (en && de && ja) {
		check(same(en->language, "en-US") && same(de->language, "de-DE") && same(ja->language, "ja-JP"),
			"languages from the region codes (0, 3, 14)");
		check(en->is_default && !de->is_default && !ja->is_default, "English is the default");
		check(!en->is_rtf && !de->is_rtf && !ja->is_rtf, "plain-text agreements");

		check(same(en->text, "SLA TEST AGREEMENT (English)\nThis is the English test agreement.\n"),
			"English text, Mac line breaks as \\n");
		check(contains(de->text, "Testvereinbarung: Gr\xc3\xb6\xc3\x9f" "e, \xc3\x9c" "bung."), "German text from Mac Roman to UTF-8");
		check(contains(ja->text, "\xe3\x81\x93\xe3\x82\x8c\xe3\x81\xaf\xe3\x83\x86\xe3\x82\xb9\xe3\x83\x88"
			"\xe5\xa5\x91\xe7\xb4\x84\xe3\x81\xa7\xe3\x81\x99\xe3\x80\x82"), "Japanese text from Shift JIS to UTF-8");

		check(same(en->language_name, "English") && same(en->agree, "Agree") && same(en->disagree, "Disagree")
			&& same(en->print, "Print") && same(en->save, "Save...")
			&& contains(en->message, "press \"Agree\" to install"), "English buttons and prompt");
		check(same(de->language_name, "Deutsch") && same(de->agree, "Akzeptieren") && same(de->disagree, "Ablehnen")
			&& contains(de->message, "\xe2\x80\x9e" "Akzeptieren\xe2\x80\x9c"), "German buttons and prompt");
		check(same(ja->language_name, "\xe6\x97\xa5\xe6\x9c\xac\xe8\xaa\x9e")
			&& same(ja->agree, "\xe5\x90\x8c\xe6\x84\x8f\xe3\x81\x97\xe3\x81\xbe\xe3\x81\x99"), "Japanese buttons");
	}

	const char* p_de[] = { "de" };
	const char* p_deAT[] = { "de-AT" };
	const char* p_frja[] = { "fr", "ja" };
	const char* p_fr[] = { "fr" };
	const char* p_jaJP_underscore[] = { "ja_JP" };
	check(same(chosen(image, p_de, 1), "de-DE"), "preferred de: German");
	check(same(chosen(image, p_deAT, 1), "de-DE"), "preferred de-AT: German (same language)");
	check(same(chosen(image, p_frja, 2), "ja-JP"), "preferred fr, ja: Japanese (first one the image has)");
	check(same(chosen(image, p_fr, 1), "en-US"), "preferred fr: the default");
	check(same(chosen(image, NULL, 0), "en-US"), "no preference: the default");
	check(same(chosen(image, p_jaJP_underscore, 1), "ja-JP"), "preferred ja_JP: Japanese");
	darling_dmg_close(image);

	// macOS attaches images with a malformed language table as if they had no agreement
	for (int i = 3; i <= 4; i++) {
		darling_dmg_image* bad = openImage(argv[i]);
		if (bad) {
			char what[128];
			snprintf(what, sizeof(what), "malformed LPic (%s): no licences", i == 3 ? "cut short" : "count too large");
			check(darling_dmg_license_count(bad) == 0, what);
			darling_dmg_close(bad);
		}
	}

	return failures ? 1 : 0;
}

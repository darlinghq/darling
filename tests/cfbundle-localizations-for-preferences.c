// Guest test: CFBundleCopyLocalizationsForPreferences(). Expected values were
// observed on macOS 26.7.3 (x86_64) with the same inputs. Where ICU's
// ualoc_localizationsToUse() finds a match, CFBundle gives the same answer
// (zh-TW picks zh-Hant, en-AU picks en-GB, en picks both en and English);
// where it finds none, the first localization is given.
#include <CoreFoundation/CoreFoundation.h>
#include <stdio.h>
#include <string.h>

static int failures;

static CFArrayRef makeList(const char *commaSeparated)
{
    CFMutableArrayRef list = CFArrayCreateMutable(NULL, 0, &kCFTypeArrayCallBacks);
    char buffer[256];
    strlcpy(buffer, commaSeparated, sizeof buffer);
    for (char *item = strtok(buffer, ","); item != NULL; item = strtok(NULL, ",")) {
        CFStringRef string = CFStringCreateWithCString(NULL, item, kCFStringEncodingUTF8);
        CFArrayAppendValue(list, string);
        CFRelease(string);
    }
    return list;
}

static void expect(const char *preferred, const char *available, const char *expected)
{
    CFArrayRef preferredList = makeList(preferred), availableList = makeList(available), expectedList = makeList(expected);
    CFArrayRef result = CFBundleCopyLocalizationsForPreferences(availableList, preferredList);
    int ok = result != NULL && CFEqual(result, expectedList);
    printf("%s: %s in (%s) -> (%s)\n", ok ? "PASS" : "FAIL", preferred, available, expected);
    if (!ok) {
        failures++;
        printf("  got (");
        for (CFIndex i = 0; result != NULL && i < CFArrayGetCount(result); i++) {
            char name[64];
            CFStringGetCString(CFArrayGetValueAtIndex(result, i), name, sizeof name, kCFStringEncodingUTF8);
            printf("%s%s", i ? "," : "", name);
        }
        printf(")\n");
    }
    if (result != NULL)
        CFRelease(result);
    CFRelease(preferredList);
    CFRelease(availableList);
    CFRelease(expectedList);
}

int main(void)
{
    expect("fr", "en,fr,de", "fr");
    expect("fr-CA", "en,fr,de", "fr");
    expect("fr-CA", "en,fr-CA,fr", "fr-CA,fr");
    expect("en-AU", "en-US,en-GB", "en-GB");
    expect("ja", "en,fr", "en");
    expect("zh-TW", "zh-Hans,zh-Hant", "zh-Hant");
    expect("pt", "pt-BR,pt-PT", "pt-BR");
    expect("es-MX", "en,es", "es");
    expect("en", "EN,fr", "EN");
    expect("en-US", "en_US,fr", "en_US");
    expect("de,fr", "en,fr,de", "de");
    expect("ja,fr", "en,fr", "fr");
    expect("en", "English,French", "English");
    expect("fr", "English,French", "French");
    expect("de", "English,German,Base", "German");
    expect("en-GB", "en,en-GB,fr", "en-GB,en");
    expect("en-US", "en,en-GB", "en");
    expect("zh-Hans", "zh_CN,zh_TW", "zh_CN");
    expect("zh-HK", "zh-Hans,zh-Hant", "zh-Hant");
    expect("zh-HK", "zh_TW,zh_CN", "zh_TW");
    expect("es-419", "es,es-MX", "es");
    expect("es-AR", "es,es-419", "es-419,es");
    expect("nb", "no,nb,en", "nb,no");
    expect("no", "nb,en", "nb");
    expect("pt-PT", "pt,pt-BR", "pt");
    expect("sr-Latn", "sr,hr", "sr");
    expect("en,fr", "Base,fr", "fr");
    expect("he", "iw,en", "iw");
    expect("iw", "he,en", "he");
    expect("in", "id,en", "id");
    expect("en-IN", "en,en-GB", "en-GB,en");
    expect("ar-SA", "ar,en", "ar");
    expect("ko-KR", "ko,ja", "ko");
    expect("x-bogus", "en,fr", "en");
    expect("en", "en,English", "en,English");
    return failures != 0;
}

// Guest test: the per-user directories confstr() gives for _CS_DARWIN_USER_DIR,
// _CS_DARWIN_USER_TEMP_DIR and _CS_DARWIN_USER_CACHE_DIR.
// - confstr(3) documents that each directory is created when it doesn't exist,
//   the user directory with mode 0755, the temporary and cache ones with 0700.
// - DIRHELPER_USER_DIR_SUFFIX names a subdirectory of each (WebKit's helper
//   processes set it). On macOS 26.7.3 (x86_64) the new directories get those
//   modes even under a umask of 077, and an empty suffix is no suffix.
// - Quirk: a suffix with a slash makes the user and cache directories fail
//   with EIO; the temporary directory then falls back to TMPDIR.
// Expected values were observed on macOS 26.7.3 with the same calls.
#include <errno.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>

static int failures;

static void report(const char *name, int ok)
{
    printf("%s: %s\n", ok ? "PASS" : "FAIL", name);
    if (!ok)
        failures++;
}

static const int names[] = { _CS_DARWIN_USER_DIR, _CS_DARWIN_USER_TEMP_DIR, _CS_DARWIN_USER_CACHE_DIR };
static const char *labels[] = { "user", "temp", "cache" };
static const mode_t modes[] = { 0755, 0700, 0700 };

static int has_suffix(const char *path, const char *suffix)
{
    char expected[PATH_MAX];
    snprintf(expected, sizeof expected, "/%s/", suffix);
    size_t length = strlen(path), suffixLength = strlen(expected);
    return length >= suffixLength && strcmp(path + length - suffixLength, expected) == 0;
}

int main(void)
{
    char suffix[64], paths[3][PATH_MAX], plain[3][PATH_MAX], name[128];
    snprintf(suffix, sizeof suffix, "org.darlinghq.confstr-test.%d", (int)getpid());

    unsetenv("DIRHELPER_USER_DIR_SUFFIX");
    for (int i = 0; i < 3; i++) {
        snprintf(name, sizeof name, "%s directory without a suffix", labels[i]);
        report(name, confstr(names[i], plain[i], PATH_MAX) > 0);
    }

    setenv("DIRHELPER_USER_DIR_SUFFIX", "", 1);
    for (int i = 0; i < 3; i++) {
        char path[PATH_MAX];
        snprintf(name, sizeof name, "%s directory: an empty suffix is no suffix", labels[i]);
        report(name, confstr(names[i], path, sizeof path) > 0 && strcmp(path, plain[i]) == 0);
    }

    mode_t oldMask = umask(077);
    setenv("DIRHELPER_USER_DIR_SUFFIX", suffix, 1);
    for (int i = 0; i < 3; i++) {
        struct stat st;
        char resolved[PATH_MAX];
        int ok = confstr(names[i], paths[i], PATH_MAX) > 0;
        snprintf(name, sizeof name, "%s directory with the suffix", labels[i]);
        report(name, ok && has_suffix(paths[i], suffix));
        snprintf(name, sizeof name, "%s directory created", labels[i]);
        report(name, ok && stat(paths[i], &st) == 0 && S_ISDIR(st.st_mode) && realpath(paths[i], resolved) != NULL);
        snprintf(name, sizeof name, "%s directory mode %o despite the umask", labels[i], (unsigned)modes[i]);
        report(name, ok && stat(paths[i], &st) == 0 && (st.st_mode & 07777) == modes[i]);
    }
    umask(oldMask);

    setenv("DIRHELPER_USER_DIR_SUFFIX", "a/b", 1);
    for (int i = 0; i < 3; i++) {
        char path[PATH_MAX];
        errno = 0;
        size_t length = confstr(names[i], path, sizeof path);
        snprintf(name, sizeof name, "quirk: suffix with a slash: %s directory", labels[i]);
        if (names[i] == _CS_DARWIN_USER_TEMP_DIR)
            report(name, length > 0 && strstr(path, "a/b") == NULL);
        else
            report(name, length == 0 && errno == EIO);
    }

    for (int i = 0; i < 3; i++)
        rmdir(paths[i]);
    return failures != 0;
}

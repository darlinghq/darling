// Copyright (C) 2026 Lubos Dolezel
//
// "launchservicesd --register BUNDLE" registers one bundle in the LaunchServices
// database and exits: 0 when the registration is committed (also for a bundle that has
// not changed), 1 when it fails, 2 for a bad command line. A registration is one
// transaction: when any write fails, nothing of it is kept. A changed UTI description
// replaces the stored one.
//
// This writes to the system LaunchServices database (and removes its own rows again),
// so run it in a disposable prefix.
#include <dlfcn.h>
#include <errno.h>
#include <spawn.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <unistd.h>

extern char** environ;

static int g_failures = 0;

#define CHECK(cond) do { if (!(cond)) { printf("FAIL %d: %s\n", __LINE__, #cond); g_failures++; } } while (0)

#define DAEMON "/System/Library/CoreServices/launchservicesd"
#define DATABASE "/private/var/db/launchservices.db"

typedef struct sqlite3 sqlite3;
typedef struct sqlite3_stmt sqlite3_stmt;

static int (*p_open)(const char*, sqlite3**);
static int (*p_close)(sqlite3*);
static int (*p_exec)(sqlite3*, const char*, void*, void*, char**);
static int (*p_busy_timeout)(sqlite3*, int);
static int (*p_prepare)(sqlite3*, const char*, int, sqlite3_stmt**, const char**);
static int (*p_bind_text)(sqlite3_stmt*, int, const char*, int, void*);
static int (*p_step)(sqlite3_stmt*);
static const unsigned char* (*p_column_text)(sqlite3_stmt*, int);
static int (*p_finalize)(sqlite3_stmt*);

static int loadSqlite(void)
{
	void* lib = dlopen("/usr/lib/libsqlite3.dylib", RTLD_NOW);
	if (!lib)
		return 0;
	p_open = dlsym(lib, "sqlite3_open");
	p_close = dlsym(lib, "sqlite3_close");
	p_exec = dlsym(lib, "sqlite3_exec");
	p_busy_timeout = dlsym(lib, "sqlite3_busy_timeout");
	p_prepare = dlsym(lib, "sqlite3_prepare_v2");
	p_bind_text = dlsym(lib, "sqlite3_bind_text");
	p_step = dlsym(lib, "sqlite3_step");
	p_column_text = dlsym(lib, "sqlite3_column_text");
	p_finalize = dlsym(lib, "sqlite3_finalize");
	return p_open && p_close && p_exec && p_busy_timeout && p_prepare && p_bind_text && p_step && p_column_text
		&& p_finalize;
}

static int runDaemon(const char* arg1, const char* arg2)
{
	char* argv[4] = { (char*) DAEMON, (char*) arg1, (char*) arg2, NULL };
	pid_t pid;
	int status;

	if (posix_spawn(&pid, DAEMON, NULL, NULL, argv, environ) != 0)
		return -1;
	if (waitpid(pid, &status, 0) != pid || !WIFEXITED(status))
		return -1;
	return WEXITSTATUS(status);
}

// First column of the first row of a query with one text parameter, or "" without a row
static void queryText(sqlite3* db, const char* sql, const char* parameter, char* out, size_t size)
{
	sqlite3_stmt* statement = NULL;

	out[0] = 0;
	if (p_prepare(db, sql, -1, &statement, NULL) != 0)
		return;
	p_bind_text(statement, 1, parameter, -1, NULL);
	if (p_step(statement) == 100 /* SQLITE_ROW */)
	{
		const unsigned char* text = p_column_text(statement, 0);
		if (text)
			snprintf(out, size, "%s", (const char*) text);
	}
	p_finalize(statement);
}

static void writeFixture(const char* bundle, const char* identifier, const char* utiDescription)
{
	char path[1024];
	FILE* file;

	snprintf(path, sizeof(path), "%s/Contents", bundle);
	mkdir(path, 0755);
	snprintf(path, sizeof(path), "%s/Contents/Info.plist", bundle);
	file = fopen(path, "w");
	CHECK(file != NULL);
	if (!file)
		return;
	fprintf(file,
		"<?xml version=\"1.0\"?><plist version=\"1.0\"><dict>"
		"<key>CFBundleIdentifier</key><string>%s</string>"
		"<key>CFBundleExecutable</key><string>fixture</string>"
		"<key>CFBundleURLTypes</key><array><dict><key>CFBundleURLName</key><string>Fixture</string>"
		"<key>CFBundleURLSchemes</key><array><string>darlingfixture</string></array></dict></array>"
		"<key>UTExportedTypeDeclarations</key><array><dict>"
		"<key>UTTypeIdentifier</key><string>org.darling.fixture.type</string>"
		"<key>UTTypeDescription</key><string>%s</string>"
		"<key>UTTypeConformsTo</key><string>public.data</string>"
		"<key>UTTypeTagSpecification</key><dict><key>public.filename-extension</key>"
		"<string>darlingfixture</string></dict>"
		"</dict></array>"
		"</dict></plist>", identifier, utiDescription);
	fclose(file);

	snprintf(path, sizeof(path), "%s/Contents/PkgInfo", bundle);
	file = fopen(path, "w");
	CHECK(file != NULL);
	if (file)
	{
		fputs("APPLTst!", file);
		fclose(file);
	}
}

int main(void)
{
	char root[] = "/tmp/launchservicesd_register.XXXXXX";
	char bundle[1024], identifier[128], text[256];
	sqlite3* db = NULL;

	if (access(DAEMON, X_OK) != 0 || !loadSqlite())
	{
		printf("SKIP: launchservicesd or libsqlite3 not available\n");
		return 0;
	}
	if (!mkdtemp(root))
	{
		printf("FAIL mkdtemp\n");
		return 1;
	}
	snprintf(bundle, sizeof(bundle), "%s/Fixture.app", root);
	mkdir(bundle, 0755);
	snprintf(identifier, sizeof(identifier), "org.darling.test.register.%d", (int) getpid());

	// Command line
	CHECK(runDaemon("--register", NULL) == 2);
	CHECK(runDaemon("--bogus", bundle) == 2);

	// Not a bundle
	CHECK(runDaemon("--register", "/nonexistent/path/Missing.app") == 1);

	// A new bundle
	writeFixture(bundle, identifier, "First");
	CHECK(runDaemon("--register", bundle) == 0);

	CHECK(p_open(DATABASE, &db) == 0);
	p_busy_timeout(db, 5000);
	p_exec(db, "PRAGMA foreign_keys = ON", NULL, NULL, NULL);

	queryText(db, "select bundle_id from bundle where path = ?", bundle, text, sizeof(text));
	CHECK(strcmp(text, identifier) == 0);
	queryText(db, "select package_type || creator from bundle where path = ?", bundle, text, sizeof(text));
	CHECK(strcmp(text, "APPLTst!") == 0);
	queryText(db, "select description from uti where bundle = (select id from bundle where path = ?)",
		bundle, text, sizeof(text));
	CHECK(strcmp(text, "First") == 0);
	queryText(db, "select count(*) from bundle_url_type_scheme where scheme = 'darlingfixture' and type in "
		"(select id from bundle_url_type where bundle = (select id from bundle where path = ?))",
		bundle, text, sizeof(text));
	CHECK(strcmp(text, "1") == 0);

	// An unchanged bundle registers again without duplicating anything
	CHECK(runDaemon("--register", bundle) == 0);
	queryText(db, "select count(*) from bundle where path = ?", bundle, text, sizeof(text));
	CHECK(strcmp(text, "1") == 0);

	// A changed UTI description replaces the stored one
	writeFixture(bundle, identifier, "Second");
	CHECK(runDaemon("--register", bundle) == 0);
	queryText(db, "select description from uti where bundle = (select id from bundle where path = ?)",
		bundle, text, sizeof(text));
	CHECK(strcmp(text, "Second") == 0);

	// A failing write rolls the whole registration back and reports failure
	CHECK(p_exec(db, "create trigger darling_test_fail before insert on uti_tag "
		"begin select raise(ABORT, 'fixture failure'); end", NULL, NULL, NULL) == 0);
	writeFixture(bundle, identifier, "Third");
	CHECK(runDaemon("--register", bundle) == 1);
	queryText(db, "select description from uti where bundle = (select id from bundle where path = ?)",
		bundle, text, sizeof(text));
	CHECK(strcmp(text, "Second") == 0);

	// Without the failure the same bundle registers
	CHECK(p_exec(db, "drop trigger darling_test_fail", NULL, NULL, NULL) == 0);
	CHECK(runDaemon("--register", bundle) == 0);
	queryText(db, "select description from uti where bundle = (select id from bundle where path = ?)",
		bundle, text, sizeof(text));
	CHECK(strcmp(text, "Third") == 0);

	// Remove the rows again (the child tables go with them)
	sqlite3_stmt* cleanup = NULL;
	if (p_prepare(db, "delete from bundle where path = ?", -1, &cleanup, NULL) == 0)
	{
		p_bind_text(cleanup, 1, bundle, -1, NULL);
		p_step(cleanup);
		p_finalize(cleanup);
	}
	p_close(db);

	snprintf(text, sizeof(text), "rm -rf '%s'", root);
	system(text);

	if (g_failures)
	{
		printf("%d failures\n", g_failures);
		return 1;
	}
	printf("PASS\n");
	return 0;
}

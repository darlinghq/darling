// Copyright (C) 2026 fr4nck
//
// The Resource Manager calls that look at all open resource files (GetResource, GetNamedResource, CountResources)
// answer for the type they are asked about, also right after a call about another type: two types with the same IDs
// and names in one resource file, asked in turn.
#include <CoreServices/CoreServices.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

#pragma clang diagnostic ignored "-Wdeprecated-declarations"

static int g_failures;
#define CHECK(cond) do { if (!(cond)) { printf("FAIL line %d: %s\n", __LINE__, #cond); g_failures++; } } while (0)

static void put16(unsigned char* p, unsigned v) { p[0] = v >> 8; p[1] = v; }
static void put32(unsigned char* p, unsigned v) { p[0] = v >> 24; p[1] = v >> 16; p[2] = v >> 8; p[3] = v; }

struct Res { const char* type; short id; const char* name; const char* data; };

// A resource file (resources in the data fork); the resources of a type are next to each other in the array
static size_t resourceFile(unsigned char* out, const struct Res* res, int count)
{
	size_t data = 256, p = data;
	unsigned offsets[16];
	int types = 0, firstOfType[16], countOfType[16];

	for (int i = 0; i < count; i++)
	{
		size_t len = strlen(res[i].data);

		offsets[i] = (unsigned) (p - data);
		put32(out + p, (unsigned) len);
		memcpy(out + p + 4, res[i].data, len);
		p += 4 + len;
		if (i == 0 || memcmp(res[i].type, res[i - 1].type, 4))
		{
			firstOfType[types] = i;
			countOfType[types++] = 0;
		}
		countOfType[types - 1]++;
	}

	size_t map = p, typeList = 28, refList = typeList + 2 + 8 * types, nameList = refList + 12 * count, names = 0;

	memset(out + map, 0, nameList);
	put16(out + map + typeList, (unsigned) (types - 1));
	for (int t = 0; t < types; t++)
	{
		unsigned char* e = out + map + typeList + 2 + 8 * t;

		memcpy(e, res[firstOfType[t]].type, 4);
		put16(e + 4, (unsigned) (countOfType[t] - 1));
		put16(e + 6, (unsigned) (refList - typeList + 12 * firstOfType[t]));
	}
	for (int i = 0; i < count; i++)
	{
		unsigned char* r = out + map + refList + 12 * i;

		put16(r, (unsigned) res[i].id);
		put16(r + 2, 0xFFFF);
		if (res[i].name)
		{
			size_t len = strlen(res[i].name);

			put16(r + 2, (unsigned) names);
			out[map + nameList + names] = (unsigned char) len;
			memcpy(out + map + nameList + names + 1, res[i].name, len);
			names += 1 + len;
		}
		put32(r + 4, offsets[i]);
	}

	size_t mapLen = nameList + names;

	put32(out, 256); put32(out + 4, (unsigned) map); put32(out + 8, (unsigned) (map - data)); put32(out + 12, (unsigned) mapLen);
	memset(out + 16, 0, 240);
	memcpy(out + map, out, 16);
	put16(out + map + 24, (unsigned) typeList); put16(out + map + 26, (unsigned) nameList);
	return map + mapLen;
}

static int dataIs(Handle h, const char* expected)
{
	return h && *h && GetHandleSize(h) == (Size) strlen(expected) && memcmp(*h, expected, strlen(expected)) == 0;
}

int main(void)
{
	static const struct Res res[] = {
		{ "AAAA", 128, "one", "a128" },
		{ "BBBB", 128, "one", "b128" }, { "BBBB", 129, "two", "b129" },
	};
	unsigned char file[1024];
	size_t len = resourceFile(file, res, 3);
	char path[] = "/tmp/carboncore_resource_type_caches.XXXXXX";
	int fd = mkstemp(path);

	if (fd < 0 || write(fd, file, len) != (ssize_t) len)
		return 1;
	close(fd);

	FSRef ref;
	HFSUniStr255 dataFork;
	ResFileRefNum rf = kResFileNotOpened;

	FSGetDataForkName(&dataFork);
	CHECK(FSPathMakeRef((const UInt8*) path, &ref, NULL) == noErr);
	CHECK(FSOpenResourceFile(&ref, dataFork.length, dataFork.unicode, fsRdPerm, &rf) == noErr);
	UseResFile(rf);

	// By ID
	CHECK(dataIs(GetResource('AAAA', 128), "a128"));
	CHECK(dataIs(GetResource('BBBB', 128), "b128"));
	CHECK(dataIs(GetResource('AAAA', 128), "a128"));
	CHECK(GetResource('AAAA', 129) == NULL);

	// By name
	CHECK(dataIs(GetNamedResource('AAAA', "\pone"), "a128"));
	CHECK(dataIs(GetNamedResource('BBBB', "\pone"), "b128"));
	CHECK(GetNamedResource('AAAA', "\ptwo") == NULL);
	CHECK(dataIs(GetNamedResource('BBBB', "\ptwo"), "b129"));

	// Counts
	CHECK(CountResources('AAAA') == 1);
	CHECK(CountResources('BBBB') == 2);
	CHECK(CountResources('AAAA') == 1);

	CloseResFile(rf);
	unlink(path);
	printf(g_failures ? "FAILED: %d\n" : "PASSED\n", g_failures);
	return g_failures != 0;
}

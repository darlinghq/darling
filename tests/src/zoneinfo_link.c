/*
 * Copyright (C) 2026 Lubos Dolezel
 *
 * The zone files must be readable both under /usr/share/zoneinfo and under
 * /var/db/timezone/zoneinfo (where macOS 10.13 and later keep them), and
 * localtime() must honour TZ=Europe/Prague through libc.
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <sys/stat.h>

static int fails;
#define CHECK(c) do { if (!(c)) { printf("FAIL: %s (line %d)\n", #c, __LINE__); fails++; } } while (0)

int main(void)
{
	struct stat st;
	CHECK(stat("/usr/share/zoneinfo/Europe/Prague", &st) == 0);
	CHECK(stat("/var/db/timezone/zoneinfo/Europe/Prague", &st) == 0);

	/* 2021-07-01 12:00:00 UTC is 14:00 CEST (UTC+2) in Prague, 12:00 in UTC. */
	setenv("TZ", "Europe/Prague", 1);
	tzset();
	time_t t = 1625140800;
	struct tm tm;
	CHECK(localtime_r(&t, &tm) != NULL);
	CHECK(tm.tm_hour == 14);
	CHECK(tm.tm_gmtoff == 7200);
	CHECK(strcmp(tm.tm_zone, "CEST") == 0);

	/* 2021-01-15 12:00:00 UTC is 13:00 CET (UTC+1). */
	t = 1610712000;
	CHECK(localtime_r(&t, &tm) != NULL);
	CHECK(tm.tm_hour == 13);
	CHECK(tm.tm_gmtoff == 3600);

	printf(fails ? "FAIL\n" : "PASS\n");
	return fails != 0;
}

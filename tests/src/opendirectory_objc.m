// Copyright (C) 2026 Lubos Dolezel
//
// The OpenDirectory classes reach the local node through CFOpenDirectory:
// a node and record looked up with ODNode/ODRecord carry the same values the
// C API returns, queries return ODRecord objects, and errors arrive as
// NSErrors in ODFrameworkErrorDomain with the CFOpenDirectory codes.
// The program passes unchanged on macOS 27.0.1.
#import <Foundation/Foundation.h>
#import <OpenDirectory/OpenDirectory.h>
#include <signal.h>
#include <stdio.h>
#include <unistd.h>

static int g_failures = 0;

#define CHECK(cond) do { if (!(cond)) { printf("FAIL %d: %s\n", __LINE__, #cond); g_failures++; } } while (0)

static void finish(int status)
{
	fflush(stdout);
	_exit(status);
}

static void on_alarm(int sig)
{
	(void) sig;
	printf("FAIL: timed out\n");
	finish(2);
}

int main(void)
{
	signal(SIGALRM, on_alarm);
	alarm(60);

	@autoreleasepool
	{
		NSError *error = nil;
		ODSession *session = [ODSession defaultSession];
		CHECK(session != nil);
		CHECK([[session nodeNamesAndReturnError: &error] containsObject: @"/Local/Default"]);

		ODNode *local = [ODNode nodeWithSession: session type: kODNodeTypeLocalNodes error: &error];
		CHECK(local != nil && error == nil);
		CHECK([[local nodeName] isEqualToString: @"/Local/Default"]);

		error = nil;
		CHECK([ODNode nodeWithSession: session name: @"/No/Such/Node" error: &error] == nil);
		CHECK([[error domain] isEqualToString: ODFrameworkErrorDomain] && [error code] == kODErrorNodeUnknownName);

		error = nil;
		ODRecord *root = [local recordWithRecordType: kODRecordTypeUsers name: @"root" attributes: nil error: &error];
		CHECK(root != nil && error == nil);
		CHECK([[root recordName] isEqualToString: @"root"]);
		CHECK([[root recordType] isEqualToString: kODRecordTypeUsers]);
		CHECK([[root valuesForAttribute: kODAttributeTypeUniqueID error: &error] containsObject: @"0"]);

		NSDictionary *details = [root recordDetailsForAttributes: nil error: &error];
		CHECK([details[kODAttributeTypeUniqueID] containsObject: @"0"]);

		CHECK([local recordWithRecordType: kODRecordTypeUsers name: @"no_such_user_od_test" attributes: nil error: &error] == nil);

		error = nil;
		ODQuery *query = [ODQuery queryWithNode: local forRecordTypes: kODRecordTypeUsers attribute: kODAttributeTypeRecordName
			matchType: kODMatchEqualTo queryValues: @"root" returnAttributes: nil maximumResults: 0 error: &error];
		CHECK(query != nil && error == nil);
		NSArray *results = [query resultsAllowingPartial: NO error: &error];
		CHECK([results count] == 1);
		CHECK([[results firstObject] isKindOfClass: [ODRecord class]]);
		CHECK([[[results firstObject] recordName] isEqualToString: @"root"]);
	}

	if (g_failures)
		printf("FAIL: %d check(s) failed\n", g_failures);
	else
		printf("PASS\n");
	finish(g_failures ? 1 : 0);
}

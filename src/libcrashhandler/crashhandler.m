#import <Foundation/NSXMLElement.h>
#import <Foundation/NSXMLDocument.h>
#import <Foundation/NSXMLNode.h>
#import <Foundation/NSString.h>
#import <Foundation/NSAutoreleasePool.h>
#include <signal.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
//#include "../../config.h"

#ifndef TEST
__attribute__((constructor))
#endif
	static void setSignalHandlers();

__attribute__((destructor)) static void freeMemory();
static void crashHandler(int signo, siginfo_t* info, void*);

// Defined in Darling dyld
extern char** g_argv;
extern int g_argc;
static char** g_argvCopy = NULL; // out private copy
static char* g_originalCwd = NULL;

void setSignalHandlers()
{
	if (getenv("DARLING_NO_HANDLER") == NULL)
	{
		struct sigaction act;

		g_argvCopy = (char**) malloc(sizeof(char**) * (g_argc+1));
		for (int i = 0; i < g_argc; i++)
			g_argvCopy[i] = strdup(g_argv[i]);
		g_argvCopy[g_argc] = NULL;

		g_originalCwd = getcwd(NULL, 0);

		memset(&act, 0, sizeof act);
		act.sa_sigaction = crashHandler;
		act.sa_flags = SA_SIGINFO;
		sigemptyset (&act.sa_mask);

		sigaction(SIGSEGV, &act, NULL);
		sigaction(SIGABRT, &act, NULL);
		sigaction(SIGILL, &act, NULL);
		sigaction(SIGFPE, &act, NULL);
		sigaction(SIGBUS, &act, NULL);
	}
}

void freeMemory()
{
	if (g_argvCopy)
	{
		for (int i = 0; g_argvCopy[i] != NULL; i++)
			free(g_argvCopy[i]);
		free(g_argvCopy);
		g_argvCopy = NULL;
	}
	free(g_originalCwd);
}

void crashHandler(int signo, siginfo_t* info, void* p)
{
	NSAutoreleasePool* pool = [[NSAutoreleasePool alloc] init];
	// Generate a XML file with error description
	NSXMLElement *root = (NSXMLElement *)[NSXMLNode elementWithName:@"crash"];
	NSXMLDocument *xmlDoc = [[NSXMLDocument alloc] initWithRootElement:root];
	NSXMLElement* sroot;
	NSXMLElement* sub;

	[xmlDoc setVersion:@"1.0"];
	[xmlDoc setCharacterEncoding:@"UTF-8"];

	// Add runtime information
	sroot = (NSXMLElement *)[NSXMLNode elementWithName:@"application"];
	sub = (NSXMLElement *)  [NSXMLNode elementWithName: @"wd" stringValue:[NSString stringWithUTF8String: g_originalCwd]];
	[sroot addChild:sub];

	// Add arguments (to restart the app)
	sub = (NSXMLElement *)[NSXMLNode elementWithName:@"arguments"];
	for (int i = 0; i < g_argc; i++)
	{
		NSXMLElement* value = (NSXMLElement *)[NSXMLNode elementWithName:@"argument" stringValue:[NSString stringWithUTF8String: g_argvCopy[i]]];
		[sub addChild: value];
	}
	[sroot addChild:sub];
	[root addChild:sroot];

	// Signal information
	sroot = (NSXMLElement *)  [NSXMLNode elementWithName: @"signal" stringValue:[NSString stringWithFormat: @"%d", info->si_signo ]];
	[root addChild:sroot];

	// Save the file
	NSData *xmlData = [xmlDoc XMLDataWithOptions:NSXMLNodePreserveAll];
	NSString* path = [NSString stringWithFormat: @"/tmp/crashinfo.%d.xml", getpid()];
	[xmlData writeToFile:path atomically:YES];

	NSLog(@"Crash info stored at %@\n", path);

	[pool release];

	// Execute the crash dialog
	
	exit(signo);
}

#ifdef TEST
int g_argc;
char** g_argv;
int main(int argc, char** argv)
{
	g_argc = argc;
	g_argv = argv;

	setSignalHandlers();
	*((volatile int*) NULL) = 0;

	return 0;
}
#endif


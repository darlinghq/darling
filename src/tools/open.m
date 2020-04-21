/*
 This file is part of Darling.

 Copyright (C) 2019-2020 Lubos Dolezel

 Darling is free software: you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation, either version 3 of the License, or
 (at your option) any later version.

 Darling is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details.

 You should have received a copy of the GNU General Public License
 along with Darling.  If not, see <http://www.gnu.org/licenses/>.
*/

#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <stdlib.h>
#include <getopt.h>
#include <stdbool.h>

#import <Foundation/NSArray.h>
#import <Foundation/NSString.h>
#import <Foundation/NSURL.h>
#import <AppKit/NSWorkspace.h>
#include <CoreServices/CoreServices.h>

static void usage(void);

int main(int argc, char *argv[])
{
	bool fresh = false;
	bool reveal = false;
	bool wait = false;
	bool _new = false;
	bool hide = false;
	bool background = false;
	bool header = false;
	bool useDefTextEditor = false;
	bool readStdin = false;

	const char* sdk = NULL;
	const char* bundle = NULL;
	const char* appname = NULL;
	NSArray<NSString*>* arguments = nil;
	NSArray<NSURL*>* filenames = nil;
	FSRef fsref;
	bool hasFSRef = false;
	CFURLRef appURL = NULL;

	for (int i = 1; i < argc; i++)
	{
		if (strcmp(argv[i], "--args") == 0)
		{
			int count = argc-i-1;
			NSString** strings = (NSString**) malloc(count * sizeof(NSString*));

			for (int j = 0; j < count; j++)
				strings[j] = [NSString stringWithCString: argv[j+i+1] encoding: NSUTF8StringEncoding];

			arguments = [NSArray arrayWithObjects: strings count: count];
			free(strings);
			
			argv[i] = NULL;
			argc = i;
			break;
		}
	}

	while (true)
	{
		static struct option long_options[] = {
			{ "fresh", no_argument, 0, 'F' },
			{ "reveal", no_argument, 0, 'R' },
			{ "wait-apps", no_argument, 0, 'W' },
			{ "new", no_argument, 0, 'n' },
			{ "hide", no_argument, 0, 'j' },
			{ "background", no_argument, 0, 'g' },
			{ "header", no_argument, 0, 'h' },
			{ NULL, 0, 0, 0 }
		};
		int c = getopt_long(argc, argv, "a:b:etfFRWnjghs:", long_options, NULL);

		if (c == -1)
			break;

		switch (c)
		{
			case 'a':
				appname = optarg;
				break;
			case 'b':
				bundle = optarg;
				break;
			case 'e':
				bundle = "com.apple.TextEdit";
				break;
			case 't':
				useDefTextEditor = true;
				break;
			case 'f':
				readStdin = true;
				break;
			case 'F':
				fresh = true;
				break;
			case 'R':
				reveal = true;
				break;
			case 'W':
				wait = true;
				break;
			case 'n':
				_new = true;
				break;
			case 'j':
				hide = true;
				break;
			case 'g':
				background = true;
				break;
			case 'h':
				header = true;
				break;
			case 's':
				sdk = optarg;
				break;
			default:
				usage();
				return EXIT_FAILURE;
		}
	}

	if (header || reveal || readStdin)
	{
		fprintf(stderr, "This operation is not implemented yet. Sorry.\n");
		return EXIT_FAILURE;
	}

	if (optind < argc)
	{
		int count = argc - optind;
		NSURL** urls = (NSURL**) malloc(count * sizeof(NSURL*));

		for (int i = 0; i < count; i++)
			urls[i] = [NSURL URLWithString:[NSString stringWithCString: argv[optind+i] encoding: NSUTF8StringEncoding]];
		
		filenames = [NSArray arrayWithObjects: urls count: count];
		free(urls);
	}

	OSStatus result = noErr;
	if (useDefTextEditor)
	{
		result = LSGetApplicationForInfo(kLSUnknownType, kLSUnknownCreator, CFSTR("txt"), kLSRolesEditor, &fsref, &appURL);
		hasFSRef = true;
	}
	else if (bundle != NULL)
	{
		result = LSFindApplicationForInfo(kLSUnknownCreator, (CFStringRef) [NSString stringWithCString:bundle encoding:NSUTF8StringEncoding], NULL, &fsref, &appURL);
		hasFSRef = true;
	}
	else if (appname != NULL)
	{
		NSString* appPath = [[NSWorkspace sharedWorkspace] fullPathForApplication: [NSString stringWithCString:appname encoding:NSUTF8StringEncoding]];
		if (appPath == nil)
		{
			fprintf(stderr, "Cannot find required application: %s\n", appname);
			return EXIT_FAILURE;
		}

		appURL = (CFURLRef) [NSURL URLWithString: appPath];
		result = FSPathMakeRef((const UInt8*) [appPath UTF8String], &fsref, NULL);
	}

	if (result != noErr)
	{
		fprintf(stderr, "Cannot find required application: %d\n", result);
		return EXIT_FAILURE;
	}

	NSArray<NSURL*>* urlsToOpen;
	LSApplicationParameters params;

	memset(&params, 0, sizeof(params));

	if (hasFSRef)
		params.application = &fsref;
	params.argv = (CFArrayRef) arguments;

	if (_new)
		params.flags |= kLSLaunchNewInstance;
	if (background)
		params.flags |= kLSLaunchDontSwitch;
	if (hide)
		params.flags |= kLSLaunchAndHide;

	if (filenames)
	{
		urlsToOpen = filenames;
	}
	else if (appURL)
	{
		urlsToOpen = [NSArray arrayWithObjects: (const id*) &appURL count: 1];
	}
	else
	{
		usage();
		return EXIT_FAILURE;
	}

	// TODO: Apple Events, PSNs (for waiting)
	result = LSOpenURLsWithRole((CFArrayRef) urlsToOpen, kLSRolesAll, NULL, &params, NULL, 0);

	if (result != noErr)
	{
		fprintf(stderr, "Failed to launch: %d\n", result);
		return EXIT_FAILURE;
	}

	return EXIT_SUCCESS;
}

static void usage(void)
{
	printf("Usage: open [filenames]\n"
		"Help: Use open to open files, folders, and URLs from the command line.\n");
}

/*
 * Copyright (c) 2005 Finlay Dobbie
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. Neither the name of Finlay Dobbie nor the names of his contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */

#include <CoreFoundation/CoreFoundation.h>
#include <CoreFoundation/CFPriv.h>

void usage(char *progname) {
	fprintf(stderr, "Usage: %s [-productName|-productVersion|-buildVersion]\n", progname);
	exit(1);
}

int main(int argc, char *argv[]) {
	CFDictionaryRef dict= NULL;
	CFStringRef str = NULL;
	char cstr[256];
	
	dict = _CFCopyServerVersionDictionary();
	if (dict == NULL)
		dict = _CFCopySystemVersionDictionary();
	if (dict == NULL)
		exit(1);
	
	if (argc == 2) {
		if (!strcmp(argv[1], "-productName"))
			str = CFDictionaryGetValue(dict, _kCFSystemVersionProductNameKey);
		else if (!strcmp(argv[1], "-productVersion")) {
			/* On Darwin, we set MacOSXProductVersion to the corresponding OS X release.
			   This is for compatibility with scripts that set MACOSX_DEPLOYMENT_TARGET
			   based on sw_vers -productVersion */
			str = CFDictionaryGetValue(dict, CFSTR("MacOSXProductVersion"));
			if (str == NULL)
				str = CFDictionaryGetValue(dict, _kCFSystemVersionProductVersionKey);
		}
		else if (!strcmp(argv[1], "-buildVersion"))
			str = CFDictionaryGetValue(dict, _kCFSystemVersionBuildVersionKey);
		else
			usage(argv[0]);
		CFRetain(str);
	} else if (argc == 1) {
		str = CFStringCreateWithFormat(NULL, NULL, 
			CFSTR("ProductName:    %@\n"
				  "ProductVersion: %@\n"
				  "BuildVersion:   %@"), 
			CFDictionaryGetValue(dict, _kCFSystemVersionProductNameKey),
			CFDictionaryGetValue(dict, _kCFSystemVersionProductVersionKey),
			CFDictionaryGetValue(dict, _kCFSystemVersionBuildVersionKey));
	}  else {
		usage(argv[0]);
	}
	
	if (!CFStringGetCString(str, cstr, sizeof(cstr), CFStringGetSystemEncoding()))
		exit(1);
		
	printf("%s\n", cstr);
	
	CFRelease(str);
	CFRelease(dict);
	return 0;
}

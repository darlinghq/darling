#include "LaunchServices.h"
#include "MacErrors.h"
#include <unistd.h>
#include <string>
#include <memory>
#include <vector>
#include <algorithm>
#include <errno.h>
#include <fcntl.h>
#include <unicode/unistr.h>
#include "UniChar.h"

// in FileManager.cpp
bool FSRefMakePath(const FSRef* ref, std::string& out);

OSStatus LSInit(LSInitializeFlags flags)
{
	return noErr;
}

OSStatus LSTerm(void)
{
	return noErr;
}

OSStatus LSOpenApplication(const LSApplicationParameters *appParams, ProcessSerialNumber *outPSN)
{
	if (!appParams)
		return paramErr;

	std::string exePath;
	std::vector<char*> argv;
	std::unique_ptr<std::vector<char*>> envp;
	int pipefds[2];
	pid_t pid;
	OSStatus rv = noErr;

	if (!FSRefMakePath(appParams->application, exePath))
		return fnfErr;

	if (appParams->argv != nullptr)
	{
		CFIndex count = CFArrayGetCount(appParams->argv);

		for (CFIndex i = 0; i < count; i++)
		{
			CFStringRef ref = (CFStringRef) CFArrayGetValueAtIndex(appParams->argv, i);
			if (CFGetTypeID(ref) != CFStringGetTypeID())
				return paramErr;

			argv.push_back((char*) CFStringGetCStringPtr((CFStringRef) CFArrayGetValueAtIndex(appParams->argv, i), kCFStringEncodingUTF8));
		}
		argv.push_back(nullptr);
	}
	else
	{
		argv.push_back((char*) exePath.c_str());
		argv.push_back(nullptr);
	}

	if (appParams->environment != nullptr)
	{
		CFIndex count = CFDictionaryGetCount(appParams->environment);
		envp.reset(new std::vector<char*>);
		
		CFDictionaryApplyFunction(appParams->environment, [](const void* key, const void* value, void* context) {
					CFStringRef skey = (CFStringRef) key;
					CFStringRef svalue = (CFStringRef) value;
					std::vector<const char*>* envp = (std::vector<const char*>*) context;

					if (CFGetTypeID(skey) == CFStringGetTypeID() && CFGetTypeID(svalue) == CFStringGetTypeID())
					{
						char* str = new char[CFStringGetLength(skey) + CFStringGetLength(svalue) + 1];

						strcpy(str, CFStringGetCStringPtr(skey, kCFStringEncodingUTF8));
						strcat(str, "=");
						strcat(str, CFStringGetCStringPtr(svalue, kCFStringEncodingUTF8));

						envp->push_back(str);
					}
				}, envp.get());
	}

	// https://stackoverflow.com/questions/1584956/how-to-handle-execvp-errors-after-fork
	
	if (pipe(pipefds))
	{
		if (envp)
			std::for_each(envp->begin(), envp->end(), [](char* p) { delete [] p; });

		return makeOSStatus(errno);
	}

	fcntl(pipefds[1], F_SETFD, fcntl(pipefds[1], F_GETFD) | FD_CLOEXEC);

	switch (pid = fork())
	{
		case -1:
			rv = makeOSStatus(errno);
			break;
		case 0:
			close(pipefds[0]);
			execvpe(exePath.c_str(), &argv[0], envp ? &(*envp)[0] : nullptr);
			write(pipefds[1], &errno, sizeof(int));
			_exit(1);
			break;
		default:
		{
			int err, count;
			close(pipefds[1]);

			while ((count = read(pipefds[0], &err, sizeof(errno))) == -1)
			{
				if (errno != EAGAIN && errno != EINTR)
					break;
			}

			if (count > 0)
				rv = makeOSStatus(err);
			close(pipefds[0]);
		}
	}
	
	if (envp)
		std::for_each(envp->begin(), envp->end(), [](char* p) { delete [] p; });

	return rv;
}

OSStatus LSSetExtensionHiddenForRef(const FSRef *inRef, Boolean hide)
{
	return noErr; // dummy operation
}

OSStatus LSSetExtensionHiddenForURL(CFURLRef inURL, Boolean hide)
{
	return noErr; // dummy operation
}

OSStatus LSOpenCFURLRef(CFURLRef inURL, CFURLRef *outLaunchedURL)
{
	// TODO: use 'xdg-mime query' and 'xdg-mime default' to determine the app?
	// TODO: use xdg-open to start the app
	return unimpErr;
}

OSStatus LSOpenFSRef(const FSRef *inRef, FSRef *outLaunchedRef)
{
	return unimpErr;
}

OSStatus LSGetExtensionInfo(UniCharCount inNameLen, const UniChar* inNameBuffer, UniCharCount *outExtStartIndex)
{
	if (!outExtStartIndex)
		return paramErr;

	UnicodeString str((const char*) inNameBuffer, inNameLen*2, UNICHAR_ENCODING);
	int32_t pos = str.lastIndexOf('.');

	if (pos == -1)
		*outExtStartIndex = inNameLen;
	else
		*outExtStartIndex = pos;

	return noErr;
}


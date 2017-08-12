#include <stdlib.h>
#include <stdio.h>
#include <CoreFoundation/CoreFoundation.h>
#include <stdbool.h>

bool processArgs(int argc, const char** argv, const char** command);
void resolvePlistEntry(const char* whatStr, CFPropertyListRef* parent, CFPropertyListRef* entry, char** last, bool autoCreate);
void setEntry(const char* entryName, const char* entryValue);
void printUsage(void);
void printHelp(void);
bool revertToFile(void);
bool saveToFile(void);
void runCommand(const char* cmd);
void doPrint(const char* what);

enum PropertyType {
	typeUnknown = 0, typeString, typeArray, typeDict, typeBool, typeReal, typeInteger, typeDate, typeData
};
void addEntry(const char* entry, enum PropertyType type, const char* value);
CFPropertyListRef parseValue(enum PropertyType type, const char* string);
void deleteEntry(const char* entry);
void copyEntry(const char* src, const char* dst);
void mergeFile(const char* path, const char* entry);

// Forces XML output when printing to screen
bool forceXML = false;
CFPropertyListRef plist = NULL;
const char* outputFile = NULL;

int main(int argc, const char **argv)
{
	const char* command = NULL;

	if (!processArgs(argc, argv, &command))
	{
		printUsage();
		return 1;
	}
	if (!outputFile)
		return 0;

	if (!revertToFile())
		return 1;

	if (command != NULL)
	{
		runCommand(command);
		if (!saveToFile())
			return 1;
	}
	else
	{
		while (true)
		{
			char buffer[200];

			printf("Command: ");

			if (!fgets(buffer, sizeof(buffer), stdin))
				break;

			size_t len = strlen(buffer);
			if (buffer[len-1] == '\n')
				buffer[len-1] = '\0';
			runCommand(buffer);
		}
	}

	return EXIT_SUCCESS;
}

void printUsage()
{
	puts("Usage: PlistBuddy [-cxh] <file.plist>\n"
			"    -c \"<command>\" execute command, otherwise run in interactive mode\n"
			"    -x output will be in the form of an xml plist where appropriate\n"
			"    -h print the complete help info, with command guide\n");
}

void printHelp()
{
	puts("Command Format:\n\n"
	"    Help - Prints this information\n"
	"    Exit - Exits the program, changes are not saved to the file\n"
	"    Save - Saves the current changes to the file\n"
	"    Revert - Reloads the last saved version of the file\n"
	"    Clear [<Type>] - Clears out all existing entries, and creates root of Type\n"
	"    Print [<Entry>] - Prints value of Entry.  Otherwise, prints file\n"
	"    Set <Entry> <Value> - Sets the value at Entry to Value\n"
	"    Add <Entry> <Type> [<Value>] - Adds Entry to the plist, with value Value\n"
	"    Copy <EntrySrc> <EntryDst> - Copies the EntrySrc property to EntryDst\n"
	"    Delete <Entry> - Deletes Entry from the plist\n"
	"    Merge <file.plist> [<Entry>] - Adds the contents of file.plist to Entry\n"
	"    Import <Entry> <file> - Creates or sets Entry the contents of file\n"
	"\n"
	"Entry Format:\n"
	"    Entries consist of property key names delimited by colons.  Array items\n"
	"    are specified by a zero-based integer index.  Examples:\n"
	"        :CFBundleShortVersionString\n"
	"        :CFBundleDocumentTypes:2:CFBundleTypeExtensions\n"
	"\n"
	"Types:\n"
	"    string\n"
	"    array\n"
	"    dict\n"
	"    bool\n"
	"    real\n"
	"    integer\n"
	"    date\n"
	"    data\n"
	"\n"
	"Examples:\n"
	"    Set :CFBundleIdentifier com.apple.plistbuddy\n"
	"        Sets the CFBundleIdentifier property to com.apple.plistbuddy\n"
	"    Add :CFBundleGetInfoString string \"App version 1.0.1\"\n"
	"        Adds the CFBundleGetInfoString property to the plist\n"
	"    Add :CFBundleDocumentTypes: dict\n"
	"        Adds a new item of type dict to the CFBundleDocumentTypes array\n"
	"    Add :CFBundleDocumentTypes:0 dict\n"
	"        Adds the new item to the beginning of the array\n"
	"    Delete :CFBundleDocumentTypes:0 dict\n"
	"        Deletes the FIRST item in the array\n"
	"    Delete :CFBundleDocumentTypes\n"
	"        Deletes the ENTIRE CFBundleDocumentTypes array\n");
}

bool processArgs(int argc, const char** argv, const char** command)
{
	if (argc <= 1)
		return false;

	for (int i = 1; i < argc; i++)
	{
		if (strcmp(argv[i], "-c") == 0)
		{
			if (i+1 >= argc)
				return false;
			*command = argv[++i];
		}
		else if (strcmp(argv[i], "-x") == 0)
		{
			forceXML = true;
		}
		else if (strcmp(argv[i], "-h") == 0)
		{
			printHelp();
			return true;
		}
		else if (i+1 >= argc)
		{
			outputFile = argv[i];
		}
		else
		{
			puts("Invalid Arguments");
			exit(1);
		}
	}

	return outputFile != NULL;
}

CFPropertyListRef loadPlist(const char* filePath)
{
	SInt32 errorCode = 0;
	CFStringRef errorString = NULL;
	CFDataRef data = NULL;
	CFPropertyListRef plist;
	CFStringRef path = CFStringCreateWithCString(kCFAllocatorDefault, filePath, kCFStringEncodingUTF8);
	CFURLRef url = CFURLCreateWithFileSystemPath(kCFAllocatorDefault, path, kCFURLPOSIXPathStyle, false);

	CFRelease(path);

	CFURLCreateDataAndPropertiesFromResource(kCFAllocatorDefault, url, &data, NULL, NULL, &errorCode);
	CFRelease(url);

	if (errorCode != 0)
	{
		printf("Error Reading File: %s\n", filePath);
		return false;
	}

	plist = CFPropertyListCreateFromXMLData(kCFAllocatorDefault, data, kCFPropertyListMutableContainers, &errorString);

	if (errorString != NULL)
	{
		CFShow(errorString);
		CFRelease(errorString);

		printf("Error Reading File: %s\n", filePath);
	}

	return plist;
}

bool revertToFile(void)
{
	if (plist != NULL)
		CFRelease(plist);

	if (access(outputFile, F_OK) != 0)
	{
		printf("File Doesn't Exist, Will Create: %s\n", outputFile);
		plist = CFDictionaryCreateMutable(kCFAllocatorDefault, 0, &kCFTypeDictionaryKeyCallBacks, &kCFTypeDictionaryValueCallBacks);
		return true;
	}
	else
	{
		bool rv;

		plist = loadPlist(outputFile);
		rv = plist != NULL;

		if (plist == NULL)
			plist = CFDictionaryCreateMutable(kCFAllocatorDefault, 0, &kCFTypeDictionaryKeyCallBacks, &kCFTypeDictionaryValueCallBacks);

		return rv;
	}
}

bool saveToFile(void)
{
	bool rv = true;
	CFDataRef data;
	CFStringRef path = CFStringCreateWithCString(kCFAllocatorDefault, outputFile, kCFStringEncodingUTF8);
	CFURLRef url = CFURLCreateWithFileSystemPath(kCFAllocatorDefault, path, kCFURLPOSIXPathStyle, false);

	CFRelease(path);

	data = CFPropertyListCreateXMLData(kCFAllocatorDefault, plist);
	if (data != NULL)
	{
		SInt32 errorCode = 0;

		CFURLWriteDataAndPropertiesToResource(url, data, NULL, &errorCode);
		CFRelease(data);
		
		if (errorCode != 0)
		{
			puts("Cannot Write File");
			rv = false;
		}
	}
	else
	{
		puts("Cannot Format Plist");
		rv = false;
	}

	CFRelease(url);
	return rv;
}

static const char* nextWord(const char* input)
{
	int i;

	for (i = 0; isspace(input[i]) && input[i] != '\0'; i++)
		;

	if (input[i])
		return &input[i];
	else
		return NULL;
}

bool isCommand(const char* input, const char* cmd, const char** next)
{
	size_t len = strlen(cmd);

	if (strncasecmp(input, cmd, len) == 0 && (input[len] == ' ' || input[len] == '\0'))
	{
		if (next != NULL)
			*next = nextWord(input + len);
		return true;
	}
	else
		return false;
}

static enum PropertyType parseType(const char* type)
{
	if (strcasecmp(type, "string") == 0)
		return typeString;
	else if (strcasecmp(type, "array") == 0)
		return typeArray;
	else if (strcasecmp(type, "dict") == 0)
		return typeDict;
	else if (strcasecmp(type, "bool") == 0)
		return typeBool;
	else if (strcasecmp(type, "real") == 0)
		return typeReal;
	else if (strcasecmp(type, "integer") == 0)
		return typeInteger;
	else if (strcasecmp(type, "date") == 0)
		return typeDate;
	else if (strcasecmp(type, "data") == 0)
		return typeData;
	else
	{
		printf("Unrecognized Type: %s\n", type);
		return typeUnknown;
	}
}

static enum PropertyType inferType(CFPropertyListRef obj)
{
	CFTypeID typeId = CFGetTypeID(obj);

	if (typeId == CFStringGetTypeID())
		return typeString;
	else if (typeId == CFArrayGetTypeID())
		return typeArray;
	else if (typeId == CFDictionaryGetTypeID())
		return typeDict;
	else if (typeId == CFBooleanGetTypeID())
		return typeBool;
	else if (typeId == CFNumberGetTypeID())
	{
		if (CFNumberIsFloatType((CFNumberRef) obj))
			return typeReal;
		else
			return typeInteger;
	}
	else if (typeId == CFDateGetTypeID())
		return typeDate;
	else if (typeId == CFDataGetTypeID())
		return typeData;
	else
		return typeUnknown;
}

char* getWord(const char* cmd, const char** next)
{
	char* p = strchr(cmd, ' ');
	if (p == NULL)
	{
		if (next)
			*next = NULL;
		return strdup(cmd);
	}
	else
	{
		char* rv = strndup(cmd, p-cmd);
		if (next)
			*next = nextWord(p);
		return rv;
	}
}

void runCommand(const char* cmd)
{
	const char* next = NULL;

	if (isCommand(cmd, "Exit", NULL) || isCommand(cmd, "Quit", NULL) || isCommand(cmd, "Bye", NULL))
	{
		exit(0);
	}
	else if (isCommand(cmd, "Help", NULL))
	{
		printHelp();
	}
	else if (isCommand(cmd, "Save", &next))
	{
		puts("Saving...");
		saveToFile();
	}
	else if (isCommand(cmd, "Revert", NULL))
	{
		puts("Reverting to last saved state...");
		revertToFile();
	}
	else if (isCommand(cmd, "Print", &next) || isCommand(cmd, "ls", &next))
	{
		doPrint(next);
	}
	else if (isCommand(cmd, "Clear", &next))
	{
		enum PropertyType type = typeUnknown;
		if (next != NULL)
			type = parseType(next);
		if (type == typeUnknown)
			type = typeDict;
	}
	else if (isCommand(cmd, "Set", &next) || isCommand(cmd, "=", &next))
	{
		// entry value
		if (!next)
		{
			puts("Missing arguments");
			return;
		}

		char* entry = getWord(next, &next);

		if (!next)
			next = "";

		setEntry(entry, next);

		free(entry);
	}
	else if (isCommand(cmd, "Add", &next) || isCommand(cmd, "+", &next))
	{
		// entry type [value]
		if (!next)
		{
			puts("Missing arguments");
			return;
		}

		char* entry = getWord(next, &next);

		if (!next)
		{
			free(entry);
			puts("Missing arguments");
			return;
		}

		char* type = getWord(next, &next);
		enum PropertyType typeV = parseType(type);

		if (typeV == typeUnknown)
		{
			free(entry);
			free(type);
			return;
		}

		if (!next)
			next = "";

		addEntry(entry, typeV, next);

		free(entry);
		free(type);
	}
	else if (isCommand(cmd, "Copy", &next) || isCommand(cmd, "cp", &next))
	{
		// entrySrc entryDst
		if (!next)
		{
			puts("Missing arguments");
			return;
		}

		char* src = getWord(next, &next);
		if (!next)
		{
			puts("Missing arguments");
			return;
		}

		char* dst = getWord(next, NULL);

		copyEntry(src, dst);

		free(src);
		free(dst);
	}
	else if (isCommand(cmd, "Delete", &next) || isCommand(cmd, "rm", &next) || isCommand(cmd, "-", &next))
	{
		// entry
		if (!next)
		{
			puts("Missing arguments");
			return;
		}

		deleteEntry(next);
	}
	else if (isCommand(cmd, "Merge", &next))
	{
		// file.plist [entryDst]
		if (!next)
		{
			puts("Missing arguments");
			return;
		}

		// FIXME: This isn't quite correct, this doesn't support paths with spaces
		char* path = getWord(next, &next);

		mergeFile(path, next);
		free(path);
	}
	else if (isCommand(cmd, "Import", &next))
	{
		// entry file.plist
	}
	else
	{
		puts("Unrecognized Command");
	}
}

void addEntry(const char* entry, enum PropertyType type, const char* value)
{
	CFPropertyListRef parent, existing;
	CFPropertyListRef newValue;
	char* leafName = NULL;
	CFTypeID parentType;

	// printf("add entry (type %d) with value: %s\n", type, value);

	if (type == typeArray)
	{
		newValue = CFArrayCreateMutable(kCFAllocatorDefault, 0, &kCFTypeArrayCallBacks);
	}
	else if (type == typeDict)
	{
		newValue = CFDictionaryCreateMutable(kCFAllocatorDefault, 0, &kCFTypeDictionaryKeyCallBacks, &kCFTypeDictionaryValueCallBacks);
	}
	else
	{
		newValue = parseValue(type, value);
		if (newValue == NULL)
			return;
	}

	resolvePlistEntry(entry, &parent, &existing, &leafName, true);

	if (parent == NULL)
	{
		printf("Add: Entry, \"%s\", Does Not Exist\n", entry);
		free(leafName);
		return;
	}

	parentType = CFGetTypeID(parent);

	// in case of arrays, we just insert at given position
	if (existing != NULL && parentType != CFArrayGetTypeID())
	{
		printf("Add: \"%s\" Entry Already Exists\n", entry);
		goto out;
	}

	if (parentType == CFArrayGetTypeID())
	{
		CFMutableArrayRef array = (CFMutableArrayRef) parent;
		CFIndex index = atoi(leafName);

		if (index < 0)
			index = 0;
		else if (index > CFArrayGetCount(array))
			index = CFArrayGetCount(array);

		CFArrayInsertValueAtIndex(array, index, newValue);
	}
	else if (parentType == CFDictionaryGetTypeID())
	{
		// printf("Add new entry named %s into dict %p, root is %p\n", leafName, parent, plist);
		CFStringRef key = CFStringCreateWithCString(kCFAllocatorDefault, leafName, kCFStringEncodingUTF8);
		CFDictionaryAddValue((CFMutableDictionaryRef) parent, key, newValue);
		CFRelease(key);
	}
	else
	{
		printf("Add: Can't Add Entry, \"%s\", to Parent\n", entry);
	}

out:
	free(leafName);
	CFRelease(newValue);
}

CFPropertyListRef parseValue(enum PropertyType type, const char* string)
{
	switch (type)
	{
		case typeString:
		case typeData:
		{
			char* stringOut;
			bool hasQuotes = string[0] == '"';

			size_t len = strlen(string);
			size_t i = 0, j = 0;

			if (hasQuotes)
			{
				if (string[len-1] != '"')
				{
					puts("Parse Error: Unclosed Quotes");
					return NULL;
				}
				i++;
				len--;
			}

			stringOut = (char*) malloc(len+1);

			for (; i < len; i++)
			{
				if (string[i] != '\\')
				{
					stringOut[j] = string[i];
					j++;
				}
				else
				{
					if (i+1 == len)
					{
						free(stringOut);
						puts("Parse Error: Unclosed Quotes");
						return NULL;
					}

					switch (string[++i])
					{
						case '"':
							stringOut[j++] = '"';
							break;
						case 'n':
							stringOut[j++] = '\n';
							break;
						case 't':
							stringOut[j++] = '\t';
							break;
						default:
							stringOut[j++] = string[i];
					}
				}
			}
			stringOut[j] = '\0';

			CFPropertyListRef rv;
			if (type == typeString)
			   	rv = CFStringCreateWithCString(kCFAllocatorDefault, stringOut, kCFStringEncodingUTF8);
			else
				rv = CFDataCreate(kCFAllocatorDefault, (const UInt8*) string, strlen(string));

			free(stringOut);
			return rv;
		}
		case typeBool:
			if (strcasecmp(string, "true") == 0 || strcasecmp(string, "yes") == 0 || strcmp(string, "1") == 0)
				return kCFBooleanTrue;
			return kCFBooleanFalse;

		case typeInteger:
		{
			char* endptr;
			long long v;

			v = strtoll(string, &endptr, 0);
			if (endptr == string)
			{
				puts("Unrecognized Integer Format");
				return NULL;
			}

			return CFNumberCreate(kCFAllocatorDefault, kCFNumberLongLongType, &v);
		}

		case typeReal:
		{
			char* endptr;
			double v;

			v = strtod(string, &endptr);
			if (endptr == string)
			{
				puts("Unrecognized Real Format");
				return NULL;
			}

			return CFNumberCreate(kCFAllocatorDefault, kCFNumberFloat64Type, &v);
		}
		case typeDate:
		{
			struct tm tm;

			if (!strptime(string, "%a %b %d %H:%M:%S %Z %Y", &tm)
					&& !strptime(string, "%c", &tm)
					&& !strptime(string, "%D", &tm))
			{
				puts("Unrecognized Date Format");
				return NULL;
			}

			CFTimeZoneRef tz = CFTimeZoneCopyDefault();
			CFAbsoluteTime at;
			CFGregorianDate date;

			date.day = tm.tm_mday;
			date.hour = tm.tm_hour;
			date.minute = tm.tm_min;
			date.month = tm.tm_mon + 1;
			date.second = tm.tm_sec;
			date.year = tm.tm_year + 1900;

			at = CFGregorianDateGetAbsoluteTime(date, tz);

			CFDateRef rv = CFDateCreate(kCFAllocatorDefault, at);

			CFRelease(tz);
			return rv;
		}
		default:
		{
			puts("Cannot parse this entry type");
			return NULL;
		}
	}
}

void setEntry(const char* entry, const char* entryValue)
{
	CFPropertyListRef parent, existing;
	CFPropertyListRef newValue;
	char* leafName = NULL;
	enum PropertyType type;

	resolvePlistEntry(entry, &parent, &existing, &leafName, false);
	if (existing == NULL)
	{
		free(leafName);
		printf("Set: Entry, \"%s\", Does Not Exist\n", entry);
		return;
	}

	type = inferType(existing);
	if (type == typeArray || type == typeDict)
	{
		free(leafName);
		puts("Set: Cannot Perform Set On Containers");
		return;
	}

	newValue = parseValue(type, entryValue);
	if (newValue == NULL)
	{
		free(leafName);
		return;
	}

	if (CFGetTypeID(parent) == CFDictionaryGetTypeID())
	{
		CFStringRef leafNameStr = CFStringCreateWithCString(kCFAllocatorDefault, leafName, kCFStringEncodingUTF8);
		CFDictionaryReplaceValue((CFMutableDictionaryRef) parent, leafNameStr, newValue);
		CFRelease(leafNameStr);
	}
	else
	{
		int index = atoi(leafName);

		CFArraySetValueAtIndex((CFMutableArrayRef) parent, index, newValue);
	}

	free(leafName);
	CFRelease(newValue);
}

char* strtok_empty(char* str, char delim)
{
	static char* src = NULL;
	char* p, *ret = NULL;

	if (str != NULL)
		src = str;

	if (src == NULL)
		return NULL;

	if ((p = strchr (src, delim)) != NULL)
	{
		*p = '\0';
		ret = src;
		src = ++p;
	}
	else
	{
		ret = src;
		src = NULL;
	}

	return ret;
}

void resolvePlistEntry(const char* whatStr, CFPropertyListRef* parent, CFPropertyListRef* entry, char** last, bool autoCreate)
{
	CFPropertyListRef pos = plist;
	CFPropertyListRef lastPos = NULL;

	const char *tok, *lastTok = "";

	if (whatStr[0] == ':')
		whatStr++;

	char* whatStr2 = strdup(whatStr);
	const char* finalTok = strrchr(whatStr2, ':');

	if (finalTok == NULL)
		finalTok = whatStr2;
	else
		finalTok++;

	if (*whatStr2)
		tok = strtok_empty(whatStr2, ':');
	else
		tok = NULL;

	while (tok != NULL && pos != NULL)
	{
		lastPos = pos;
		if (*tok)
		{
			CFTypeID typeId = CFGetTypeID(pos);

			if (typeId == CFDictionaryGetTypeID())
			{
				CFStringRef key = CFStringCreateWithCString(kCFAllocatorDefault, tok, kCFStringEncodingUTF8);
				pos = (CFPropertyListRef) CFDictionaryGetValue((CFDictionaryRef) pos, key);

				if (pos == NULL && autoCreate && tok != finalTok)
				{
					pos = CFDictionaryCreateMutable(kCFAllocatorDefault, 0, &kCFTypeDictionaryKeyCallBacks, &kCFTypeDictionaryValueCallBacks);
					CFDictionaryAddValue((CFMutableDictionaryRef) lastPos, key, pos);
				}
				CFRelease(key);
			}
			else if (typeId == CFArrayGetTypeID())
			{
				int index = atoi(tok);
				if (index < 0 || index >= CFArrayGetCount((CFArrayRef) pos))
				{
					pos = NULL;
				}
				else
				{
					pos = (CFPropertyListRef) CFArrayGetValueAtIndex((CFArrayRef) pos, index);
				}
			}
			else
			{
				pos = NULL;
			}
		}
		else
			pos = NULL;

		lastTok = tok;
		tok = strtok_empty(NULL, ':');
	}

	if (last != NULL)
		*last = strdup(lastTok);

	if (entry != NULL)
		*entry = pos;

	if (parent != NULL)
		*parent = (!tok) ? lastPos : NULL;

	free(whatStr2);
}

void prettyPrintPlist(CFPropertyListRef what, int indentNum)
{
	CFTypeID typeId = CFGetTypeID(what);
	char* indent = __builtin_alloca(indentNum + 1);

	memset(indent, ' ', indentNum);
	indent[indentNum] = '\0';

	if (typeId == CFStringGetTypeID())
	{
		const char* str = CFStringGetCStringPtr((CFStringRef) what, kCFStringEncodingUTF8);
		printf("%s", str);
	}
	else if (typeId == CFArrayGetTypeID())
	{
		CFArrayRef array = (CFArrayRef) what;
		CFIndex count = CFArrayGetCount(array);

		printf("Array {\n");
		for (CFIndex i = 0; i < count; i++)
		{
			CFPropertyListRef elem = (CFPropertyListRef) CFArrayGetValueAtIndex(array, i);
			printf("%s    ", indent);

			prettyPrintPlist(elem, indentNum + 4);
			printf("\n");
		}

		printf("%s}\n", indent);
	}
	else if (typeId == CFDictionaryGetTypeID())
	{
		CFDictionaryRef dict = (CFDictionaryRef) what;
		CFStringRef* keys;
		CFPropertyListRef* values;

		CFIndex count = CFDictionaryGetCount(dict);

		keys = malloc(sizeof(CFStringRef*) * count);
		values = malloc(sizeof(CFStringRef*) * count);

		CFDictionaryGetKeysAndValues(dict, (const void**) keys, (const void**) values);

		printf("Dict {\n");
		for (CFIndex i = 0; i < count; i++)
		{
			printf("%s    ", indent);
			prettyPrintPlist(keys[i], 0);
			printf(" = ");
			prettyPrintPlist(values[i], indentNum + 4);
			printf("\n");
		}

		printf("%s}\n", indent);

		free(keys);
		free(values);
	}
	else if (typeId == CFBooleanGetTypeID())
	{
		if (what == kCFBooleanTrue)
			printf("true");
		else
			printf("false");
	}
	else if (typeId == CFNumberGetTypeID())
	{
		CFNumberRef num = (CFNumberRef) what;

		if (CFNumberIsFloatType(num))
		{
			Float64 d;
			CFNumberGetValue(num, kCFNumberFloat64Type, &d);
			printf("%f", d);
		}
		else
		{
			long long l;
			CFNumberGetValue(num, kCFNumberLongLongType, &l);
			printf("%lld", l);
		}
	}
	else if (typeId == CFDateGetTypeID())
	{
		CFAbsoluteTime t = CFDateGetAbsoluteTime((CFDateRef) what);
		CFTimeZoneRef tz = CFTimeZoneCopyDefault();
		CFGregorianDate d = CFAbsoluteTimeGetGregorianDate(t, tz);
		struct tm tm;
		char buf[150];

		tm.tm_mday = d.day;
		tm.tm_mon = d.month - 1;
		tm.tm_year = d.year - 1900;
		tm.tm_hour = d.hour;
		tm.tm_min = d.minute;
		tm.tm_sec = d.second;
		tm.tm_wday = CFAbsoluteTimeGetDayOfWeek(t, tz);

		if (tm.tm_wday == 7)
			tm.tm_wday = 0;

		tm.tm_zone = 0;
		tm.tm_isdst = 0;

		strftime(buf, sizeof(buf), "%a %b %d %H:%M:%S %Z %Y", &tm);
		printf("%s", buf);

		CFRelease(tz);
	}
	else if (typeId == CFDataGetTypeID())
	{
		CFDataRef data = (CFDataRef) what;
		const UInt8* bytes = CFDataGetBytePtr(data);
		CFIndex len = CFDataGetLength(data);

		fwrite(bytes, 1, len, stdout);
		putchar('\n');
	}
}

void doPrint(const char* whatStr)
{
	CFPropertyListRef what = plist;

	if (whatStr != NULL)
	{
		resolvePlistEntry(whatStr, &what, NULL, NULL, false);
		if (what == NULL)
		{
			printf("Print: Entry, \"%s\", Does Not Exist\n", whatStr);
			return;
		}
	}

	if (!forceXML)
		prettyPrintPlist(what, 0);
	else
	{
		CFDataRef data = CFPropertyListCreateXMLData(kCFAllocatorDefault, plist);

		if (data != NULL)
		{
			prettyPrintPlist(data, 0);

			CFRelease(data);
		}
	}
}

void deleteEntry(const char* entry)
{
	CFPropertyListRef parent, existing;
	CFPropertyListRef newValue;
	char* leafName = NULL;

	resolvePlistEntry(entry, &parent, &existing, &leafName, false);
	if (existing == NULL)
	{
		printf("Delete: Entry, \"%s\", Does Not Exist\n", entry);
		goto out;
	}

	CFTypeID typeID = CFGetTypeID(parent);
	if (typeID == CFDictionaryGetTypeID())
	{
		CFStringRef str = CFStringCreateWithCString(kCFAllocatorDefault, leafName, kCFStringEncodingUTF8);
		CFDictionaryRemoveValue((CFMutableDictionaryRef) parent, str);
		CFRelease(str);
	}
	else
	{
		CFIndex idx = atoi(leafName);
		CFArrayRemoveValueAtIndex((CFMutableArrayRef) parent, idx);
	}

out:
	free(leafName);
}

void copyEntry(const char* src, const char* dst)
{
	CFPropertyListRef existing, entry, newParent, entryCopy;
	char* leafName = NULL;

	resolvePlistEntry(src, NULL, &entry, NULL, false);
	resolvePlistEntry(dst, &newParent, &existing, &leafName, true);

	if (entry == NULL)
	{
		printf("Copy: Entry, \"%s\", Does Not Exist\n", src);
		goto out;
	}
	if (existing != NULL)
	{
		printf("Copy: \"%s\" Entry Already Exists\n", dst);
		goto out;
	}

	entryCopy = CFPropertyListCreateDeepCopy(kCFAllocatorDefault, entry, kCFPropertyListMutableContainers);

	CFTypeID typeID = CFGetTypeID(newParent);
	if (typeID == CFDictionaryGetTypeID())
	{
		CFStringRef str = CFStringCreateWithCString(kCFAllocatorDefault, leafName, kCFStringEncodingUTF8);
		CFDictionaryAddValue((CFMutableDictionaryRef) newParent, str, entryCopy);
		CFRelease(str);
	}
	else
	{
		CFIndex idx = atoi(leafName);
		CFArrayInsertValueAtIndex((CFMutableArrayRef) newParent, idx, entryCopy);
	}

	CFRelease(entryCopy);
out:
	free(leafName);
}

void mergeFile(const char* path, const char* entry)
{
	CFPropertyListRef dest, fileContents;

	fileContents = loadPlist(path);
	if (fileContents == NULL)
		return;

	if (entry != NULL && *entry)
	{
		resolvePlistEntry(entry, NULL, &dest, NULL, true);
		if (dest == NULL)
		{
			printf("Merge: Entry, \"%s\", Does Not Exist\n", entry);
			CFRelease(fileContents);
			return;
		}
	}
	else
		dest = plist;
	
	CFTypeID typeID = CFGetTypeID(dest);
	CFTypeID sourceTypeID = CFGetTypeID(fileContents);

	if (typeID == CFArrayGetTypeID())
	{
		if (sourceTypeID == CFArrayGetTypeID())
		{
			CFArrayAppendArray((CFMutableArrayRef) dest, fileContents, CFRangeMake(0, CFArrayGetCount(fileContents)));
		}
		else if (sourceTypeID == CFDictionaryGetTypeID())
		{
			CFIndex count = CFDictionaryGetCount((CFDictionaryRef) fileContents);
			void** values = malloc(sizeof(void*) * count);
			CFDictionaryGetKeysAndValues((CFDictionaryRef) fileContents, NULL, (const void**) values);

			CFArrayReplaceValues((CFMutableArrayRef) dest, CFRangeMake(CFArrayGetCount((CFArrayRef) dest) - 1, 0), (const void**) values, count);
			free(values);
		}
		else
		{
			CFArrayAppendValue((CFMutableArrayRef) dest, fileContents);
		}
	}
	else if (typeID == CFDictionaryGetTypeID())
	{
		if (sourceTypeID == CFDictionaryGetTypeID())
		{
			CFIndex count = CFDictionaryGetCount((CFDictionaryRef) fileContents);
			void** values = malloc(sizeof(void*) * count);
			void** keys = malloc(sizeof(void*) * count);

			CFDictionaryGetKeysAndValues((CFDictionaryRef) fileContents, (const void**) keys, (const void**) values);

			for (CFIndex i = 0; i < count; i++)
				CFDictionarySetValue((CFMutableDictionaryRef) dest, keys[i], values[i]);

			free(keys);
			free(values);
		}
		else if (sourceTypeID == CFArrayGetTypeID())
		{
			puts("Merge: Can't Add array Entries to dict");
		}
		else
		{
			CFDictionarySetValue((CFMutableDictionaryRef) dest, CFSTR(""), fileContents);
		}
	}
	else
	{
		puts("Merge: Specified Entry Must Be a Container");
	}

	CFRelease(fileContents);
}


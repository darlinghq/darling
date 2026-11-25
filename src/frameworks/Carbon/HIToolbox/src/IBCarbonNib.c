/*
 This file is part of Darling.

 Copyright (C) 2026 Lubos Dolezel

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

// The reader of Carbon nibs: a nib is a directory with the objects of the interface in the file
// objects.xib, an XML document. Its root element is an object of the class NSIBObjectData with
// the attribute targetFramework, the list allObjects and the dictionary nameTable, which gives the
// objects that were named in Interface Builder. Elements are typed (string, int, boolean, ostype,
// array, dictionary, object, reference), carry the name of the attribute they stand for in their
// name attribute, and objects have an id that references (idRef) point to.

#include <HIToolbox/IBCarbonRuntime.h>
#include <CoreFoundation/CoreFoundation.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <unistd.h>
#include "IBCarbonNib.h"

// ---------------------------------------------------------------------------------------
// XML
// ---------------------------------------------------------------------------------------

struct XAttribute
{
	char* name;
	char* value;
};

struct XNode
{
	char* name;
	struct XAttribute* attributes;
	size_t attributeCount;
	struct XNode** children;
	size_t childCount;
	char* text;
	size_t textLength;
};

struct XParser
{
	const char* data;
	size_t size;
	size_t position;
	bool failed;
};

static void freeNode(struct XNode* node)
{
	if (!node)
		return;
	for (size_t i = 0; i < node->attributeCount; i++)
	{
		free(node->attributes[i].name);
		free(node->attributes[i].value);
	}
	free(node->attributes);
	for (size_t i = 0; i < node->childCount; i++)
		freeNode(node->children[i]);
	free(node->children);
	free(node->name);
	free(node->text);
	free(node);
}

static const char* attributeOf(const struct XNode* node, const char* name)
{
	for (size_t i = 0; i < node->attributeCount; i++)
	{
		if (strcmp(node->attributes[i].name, name) == 0)
			return node->attributes[i].value;
	}
	return NULL;
}

static bool startsWith(struct XParser* parser, const char* prefix)
{
	size_t length = strlen(prefix);

	return parser->position + length <= parser->size && memcmp(parser->data + parser->position, prefix, length) == 0;
}

static void skipSpace(struct XParser* parser)
{
	while (parser->position < parser->size && (parser->data[parser->position] == ' ' || parser->data[parser->position] == '\t' || parser->data[parser->position] == '\n' || parser->data[parser->position] == '\r'))
		parser->position++;
}

// Skips to the end of a construct that ends with the text
static void skipPast(struct XParser* parser, const char* end)
{
	size_t length = strlen(end);

	while (parser->position + length <= parser->size && memcmp(parser->data + parser->position, end, length) != 0)
		parser->position++;
	if (parser->position + length > parser->size)
	{
		parser->failed = true;
		parser->position = parser->size;
	}
	else
		parser->position += length;
}

// The text with the character references and the predefined entities resolved
static char* decode(const char* text, size_t length)
{
	char* out = malloc(length + 1);
	size_t used = 0;

	if (!out)
		return NULL;
	for (size_t i = 0; i < length;)
	{
		if (text[i] != '&')
		{
			out[used++] = text[i++];
			continue;
		}
		const char* semicolon = memchr(text + i, ';', length - i);

		if (!semicolon || semicolon - (text + i) > 10)
		{
			out[used++] = text[i++];
			continue;
		}
		size_t nameLength = (size_t) (semicolon - (text + i)) - 1;
		const char* name = text + i + 1;
		unsigned long code = 0;
		bool known = true;

		if (nameLength == 3 && memcmp(name, "amp", 3) == 0)
			code = '&';
		else if (nameLength == 2 && memcmp(name, "lt", 2) == 0)
			code = '<';
		else if (nameLength == 2 && memcmp(name, "gt", 2) == 0)
			code = '>';
		else if (nameLength == 4 && memcmp(name, "quot", 4) == 0)
			code = '"';
		else if (nameLength == 4 && memcmp(name, "apos", 4) == 0)
			code = '\'';
		else if (nameLength >= 2 && name[0] == '#')
		{
			char* end;
			char number[16];

			memcpy(number, name + 1, nameLength - 1);
			number[nameLength - 1] = 0;
			code = number[0] == 'x' ? strtoul(number + 1, &end, 16) : strtoul(number, &end, 10);
			known = *end == 0 && code != 0;
		}
		else
			known = false;
		if (!known)
		{
			out[used++] = text[i++];
			continue;
		}
		// UTF-8
		if (code < 0x80)
			out[used++] = (char) code;
		else if (code < 0x800)
		{
			out[used++] = (char) (0xC0 | (code >> 6));
			out[used++] = (char) (0x80 | (code & 0x3F));
		}
		else if (code < 0x10000)
		{
			out[used++] = (char) (0xE0 | (code >> 12));
			out[used++] = (char) (0x80 | ((code >> 6) & 0x3F));
			out[used++] = (char) (0x80 | (code & 0x3F));
		}
		else
		{
			out[used++] = (char) (0xF0 | (code >> 18));
			out[used++] = (char) (0x80 | ((code >> 12) & 0x3F));
			out[used++] = (char) (0x80 | ((code >> 6) & 0x3F));
			out[used++] = (char) (0x80 | (code & 0x3F));
		}
		i = (size_t) (semicolon - text) + 1;
	}
	out[used] = 0;
	return out;
}

static char* readName(struct XParser* parser)
{
	size_t start = parser->position;

	while (parser->position < parser->size)
	{
		char c = parser->data[parser->position];

		if (c == ' ' || c == '\t' || c == '\n' || c == '\r' || c == '=' || c == '>' || c == '/' || c == '<')
			break;
		parser->position++;
	}
	if (parser->position == start)
	{
		parser->failed = true;
		return NULL;
	}
	return strndup(parser->data + start, parser->position - start);
}

static void appendChild(struct XNode* parent, struct XNode* child)
{
	struct XNode** list = realloc(parent->children, (parent->childCount + 1) * sizeof(*list));

	if (!list)
		abort();
	parent->children = list;
	list[parent->childCount++] = child;
}

static struct XNode* parseElement(struct XParser* parser, int depth)
{
	if (depth > 64 || parser->position >= parser->size || parser->data[parser->position] != '<')
	{
		parser->failed = true;
		return NULL;
	}
	parser->position++;

	struct XNode* node = calloc(1, sizeof(*node));

	node->name = readName(parser);
	if (!node->name)
	{
		freeNode(node);
		return NULL;
	}
	for (;;)
	{
		skipSpace(parser);
		if (parser->position >= parser->size)
		{
			parser->failed = true;
			break;
		}
		char c = parser->data[parser->position];

		if (c == '/' || c == '>')
			break;

		char* name = readName(parser);

		if (!name)
			break;
		skipSpace(parser);
		if (parser->position >= parser->size || parser->data[parser->position] != '=')
		{
			free(name);
			parser->failed = true;
			break;
		}
		parser->position++;
		skipSpace(parser);
		if (parser->position >= parser->size || (parser->data[parser->position] != '"' && parser->data[parser->position] != '\''))
		{
			free(name);
			parser->failed = true;
			break;
		}
		char quote = parser->data[parser->position++];
		size_t start = parser->position;

		while (parser->position < parser->size && parser->data[parser->position] != quote)
			parser->position++;
		if (parser->position >= parser->size)
		{
			free(name);
			parser->failed = true;
			break;
		}
		struct XAttribute* list = realloc(node->attributes, (node->attributeCount + 1) * sizeof(*list));

		if (!list)
			abort();
		node->attributes = list;
		list[node->attributeCount].name = name;
		list[node->attributeCount].value = decode(parser->data + start, parser->position - start);
		node->attributeCount++;
		parser->position++;
	}
	if (parser->failed)
	{
		freeNode(node);
		return NULL;
	}
	if (parser->data[parser->position] == '/')
	{
		// <name ... />
		if (!startsWith(parser, "/>"))
		{
			parser->failed = true;
			freeNode(node);
			return NULL;
		}
		parser->position += 2;
		return node;
	}
	parser->position++;   // >

	// The content: text, comments and child elements, up to the end tag
	size_t textStart = parser->position;
	bool hasChildren = false;

	while (parser->position < parser->size && !parser->failed)
	{
		if (startsWith(parser, "</"))
		{
			if (!hasChildren)
				node->text = decode(parser->data + textStart, parser->position - textStart);
			if (node->text)
				node->textLength = strlen(node->text);
			skipPast(parser, ">");
			return parser->failed ? (freeNode(node), NULL) : node;
		}
		if (startsWith(parser, "<!--"))
		{
			skipPast(parser, "-->");
			continue;
		}
		if (startsWith(parser, "<![CDATA["))
		{
			size_t start = parser->position + 9;

			skipPast(parser, "]]>");
			if (!hasChildren && !parser->failed)
			{
				free(node->text);
				node->text = strndup(parser->data + start, parser->position - 3 - start);
				// the text of the element is only this section
				textStart = parser->position;
			}
			continue;
		}
		if (parser->data[parser->position] == '<')
		{
			struct XNode* child = parseElement(parser, depth + 1);

			if (!child)
				break;
			hasChildren = true;
			appendChild(node, child);
			continue;
		}
		parser->position++;
	}
	parser->failed = true;
	freeNode(node);
	return NULL;
}

static struct XNode* parseDocument(const char* data, size_t size)
{
	struct XParser parser = { data, size, 0, false };

	for (;;)
	{
		skipSpace(&parser);
		if (startsWith(&parser, "<?"))
			skipPast(&parser, "?>");
		else if (startsWith(&parser, "<!--"))
			skipPast(&parser, "-->");
		else if (startsWith(&parser, "<!"))
			skipPast(&parser, ">");
		else
			break;
		if (parser.failed)
			return NULL;
	}
	return parseElement(&parser, 0);
}

// ---------------------------------------------------------------------------------------
// The nib
// ---------------------------------------------------------------------------------------

struct IdEntry
{
	long id;
	struct XNode* node;
};

struct OpaqueIBNibRef
{
	CFStringRef path;           // of the objects.xib
	struct XNode* root;
	bool parsed;
	struct IdEntry* ids;
	size_t idCount;
};

static void collectIds(struct OpaqueIBNibRef* nib, struct XNode* node)
{
	const char* id = attributeOf(node, "id");

	if (id && strcmp(node->name, "object") == 0)
	{
		struct IdEntry* list = realloc(nib->ids, (nib->idCount + 1) * sizeof(*list));

		if (!list)
			abort();
		nib->ids = list;
		list[nib->idCount].id = strtol(id, NULL, 10);
		list[nib->idCount].node = node;
		nib->idCount++;
	}
	for (size_t i = 0; i < node->childCount; i++)
		collectIds(nib, node->children[i]);
}

static void ensureParsed(struct OpaqueIBNibRef* nib)
{
	if (nib->parsed)
		return;
	nib->parsed = true;

	char path[4096];

	if (!CFStringGetFileSystemRepresentation(nib->path, path, sizeof(path)))
		return;
	FILE* file = fopen(path, "rb");

	if (!file)
		return;
	fseek(file, 0, SEEK_END);
	long size = ftell(file);

	fseek(file, 0, SEEK_SET);
	if (size > 0 && size < 64 * 1024 * 1024)
	{
		char* data = malloc((size_t) size);

		if (data && fread(data, 1, (size_t) size, file) == (size_t) size)
		{
			nib->root = parseDocument(data, (size_t) size);
			if (nib->root)
				collectIds(nib, nib->root);
		}
		free(data);
	}
	fclose(file);
}

static struct XNode* findId(struct OpaqueIBNibRef* nib, long id)
{
	for (size_t i = 0; i < nib->idCount; i++)
	{
		if (nib->ids[i].id == id)
			return nib->ids[i].node;
	}
	return NULL;
}

static struct XNode* childNamed(const struct XNode* node, const char* name)
{
	for (size_t i = 0; i < node->childCount; i++)
	{
		const char* childName = attributeOf(node->children[i], "name");

		if (childName && strcmp(childName, name) == 0)
			return node->children[i];
	}
	return NULL;
}

OSStatus _IBCarbonNibOpen(CFStringRef objectsPath, IBNibRef* outNib)
{
	struct OpaqueIBNibRef* nib = calloc(1, sizeof(*nib));

	if (!nib)
		return memFullErr;
	nib->path = CFStringCreateCopy(NULL, objectsPath);
	*outNib = nib;
	return noErr;
}

bool _IBCarbonNibExists(IBNibRef nibRef)
{
	struct OpaqueIBNibRef* nib = nibRef;
	char path[4096];

	return CFStringGetFileSystemRepresentation(nib->path, path, sizeof(path)) && access(path, R_OK) == 0;
}

void _IBCarbonNibClose(IBNibRef nibRef)
{
	struct OpaqueIBNibRef* nib = nibRef;

	if (!nib)
		return;
	if (nib->path)
		CFRelease(nib->path);
	freeNode(nib->root);
	free(nib->ids);
	free(nib);
}

static CFTypeRef convert(struct OpaqueIBNibRef* nib, const struct XNode* node, int depth);

static CFStringRef stringOfNode(const struct XNode* node)
{
	return CFStringCreateWithBytes(NULL, (const UInt8*) (node->text ? node->text : ""), (CFIndex) node->textLength, kCFStringEncodingUTF8, false);
}

static CFTypeRef convertObject(struct OpaqueIBNibRef* nib, const struct XNode* node, int depth)
{
	CFMutableDictionaryRef dictionary = CFDictionaryCreateMutable(NULL, 0, &kCFTypeDictionaryKeyCallBacks, &kCFTypeDictionaryValueCallBacks);
	const char* className = attributeOf(node, "class");
	const char* id = attributeOf(node, "id");

	if (className)
	{
		CFStringRef value = CFStringCreateWithCString(NULL, className, kCFStringEncodingUTF8);

		CFDictionarySetValue(dictionary, CFSTR("class"), value);
		CFRelease(value);
	}
	if (id)
	{
		SInt64 number = strtol(id, NULL, 10);
		CFNumberRef value = CFNumberCreate(NULL, kCFNumberSInt64Type, &number);

		CFDictionarySetValue(dictionary, CFSTR("id"), value);
		CFRelease(value);
	}
	for (size_t i = 0; i < node->childCount; i++)
	{
		const char* name = attributeOf(node->children[i], "name");

		if (!name)
			continue;

		CFTypeRef value = convert(nib, node->children[i], depth + 1);

		if (value)
		{
			CFStringRef key = CFStringCreateWithCString(NULL, name, kCFStringEncodingUTF8);

			CFDictionarySetValue(dictionary, key, value);
			CFRelease(key);
			CFRelease(value);
		}
	}
	return dictionary;
}

static CFTypeRef convert(struct OpaqueIBNibRef* nib, const struct XNode* node, int depth)
{
	const char* type = node->name;

	if (depth > 40)
		return NULL;
	if (strcmp(type, "object") == 0)
		return convertObject(nib, node, depth);
	if (strcmp(type, "reference") == 0)
	{
		const char* idRef = attributeOf(node, "idRef");
		struct XNode* target = idRef ? findId(nib, strtol(idRef, NULL, 10)) : NULL;

		return target ? convertObject(nib, target, depth) : NULL;
	}
	if (strcmp(type, "string") == 0)
		return stringOfNode(node);
	if (strcmp(type, "int") == 0 || strcmp(type, "unsigned_int") == 0 || strcmp(type, "integer") == 0 || strcmp(type, "short") == 0 || strcmp(type, "long") == 0)
	{
		SInt64 number = node->text ? strtoll(node->text, NULL, 10) : 0;

		return CFNumberCreate(NULL, kCFNumberSInt64Type, &number);
	}
	if (strcmp(type, "float") == 0 || strcmp(type, "double") == 0)
	{
		double number = node->text ? strtod(node->text, NULL) : 0;

		return CFNumberCreate(NULL, kCFNumberDoubleType, &number);
	}
	if (strcmp(type, "boolean") == 0)
	{
		bool value = node->text && (strcasecmp(node->text, "TRUE") == 0 || strcmp(node->text, "1") == 0 || strcasecmp(node->text, "YES") == 0);

		return CFRetain(value ? kCFBooleanTrue : kCFBooleanFalse);
	}
	if (strcmp(type, "ostype") == 0)
	{
		// Four characters; a shorter code is padded with spaces
		unsigned char code[4] = { ' ', ' ', ' ', ' ' };

		for (size_t i = 0; i < 4 && node->text && i < node->textLength; i++)
			code[i] = (unsigned char) node->text[i];
		SInt64 number = ((SInt64) code[0] << 24) | ((SInt64) code[1] << 16) | ((SInt64) code[2] << 8) | code[3];

		return CFNumberCreate(NULL, kCFNumberSInt64Type, &number);
	}
	if (strcmp(type, "array") == 0)
	{
		CFMutableArrayRef array = CFArrayCreateMutable(NULL, 0, &kCFTypeArrayCallBacks);

		for (size_t i = 0; i < node->childCount; i++)
		{
			CFTypeRef value = convert(nib, node->children[i], depth + 1);

			if (value)
			{
				CFArrayAppendValue(array, value);
				CFRelease(value);
			}
		}
		return array;
	}
	if (strcmp(type, "dictionary") == 0)
	{
		CFMutableDictionaryRef dictionary = CFDictionaryCreateMutable(NULL, 0, &kCFTypeDictionaryKeyCallBacks, &kCFTypeDictionaryValueCallBacks);

		for (size_t i = 0; i + 1 < node->childCount; i += 2)
		{
			CFTypeRef key = convert(nib, node->children[i], depth + 1);
			CFTypeRef value = convert(nib, node->children[i + 1], depth + 1);

			if (key && value && CFGetTypeID(key) == CFStringGetTypeID())
				CFDictionarySetValue(dictionary, key, value);
			if (key)
				CFRelease(key);
			if (value)
				CFRelease(value);
		}
		return dictionary;
	}
	// An element of a kind that is not known is left out
	return NULL;
}

CFDictionaryRef _IBCarbonNibCopyObjectNamed(IBNibRef nibRef, CFStringRef name)
{
	struct OpaqueIBNibRef* nib = nibRef;

	ensureParsed(nib);
	if (!nib->root || !name)
		return NULL;

	struct XNode* table = childNamed(nib->root, "nameTable");

	if (!table)
		return NULL;
	for (size_t i = 0; i + 1 < table->childCount; i += 2)
	{
		struct XNode* key = table->children[i];
		struct XNode* value = table->children[i + 1];

		if (strcmp(key->name, "string") != 0)
			continue;

		CFStringRef keyString = stringOfNode(key);
		bool matches = CFStringCompare(keyString, name, 0) == kCFCompareEqualTo;

		CFRelease(keyString);
		if (matches)
			return (CFDictionaryRef) convert(nib, value, 0);
	}
	return NULL;
}

CFStringRef _IBCarbonNibCopyTargetFramework(IBNibRef nibRef)
{
	struct OpaqueIBNibRef* nib = nibRef;

	ensureParsed(nib);
	if (!nib->root)
		return NULL;

	struct XNode* node = childNamed(nib->root, "targetFramework");

	return node ? stringOfNode(node) : NULL;
}

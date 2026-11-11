/*
 This file is part of Darling.

 Copyright (C) 2026 Darling Developers

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

#include <CoreServices/CoreServices.h>
#include <HIToolbox/CarbonEvents.h>
#include <HIToolbox/TextServices.h>
#include <pthread.h>
#include <stdlib.h>
#include <string.h>

// Text Services Manager documents. There are no input methods to talk to,
// but a document is still a real object: it can be activated, and it keeps
// the properties that the application stores on it.

struct TSMProperty
{
	struct TSMProperty* next;
	TSMDocumentPropertyTag tag;
	UInt32 size;
	unsigned char data[];
};

struct OpaqueTSMDocumentID
{
	struct OpaqueTSMDocumentID* next;
	struct TSMProperty* properties;
	Boolean useInputWindow;
};

static pthread_mutex_t g_tsmLock = PTHREAD_MUTEX_INITIALIZER;
static TSMDocumentID g_documents;
static TSMDocumentID g_activeDocument;

// Must be called with g_tsmLock held.
static Boolean isValidDocument(TSMDocumentID docID)
{
	if (docID == NULL)
		return false;

	for (TSMDocumentID doc = g_documents; doc != NULL; doc = doc->next)
	{
		if (doc == docID)
			return true;
	}
	return false;
}

// Must be called with g_tsmLock held.
static struct TSMProperty** findProperty(TSMDocumentID docID, TSMDocumentPropertyTag tag)
{
	struct TSMProperty** link = &docID->properties;

	while (*link != NULL && (*link)->tag != tag)
		link = &(*link)->next;
	return link;
}

OSErr NewTSMDocument(short numOfInterface, InterfaceTypeList supportedInterfaces, TSMDocumentID* idocID, long refcon)
{
	TSMDocumentID doc;

	if (idocID == NULL)
		return paramErr;
	*idocID = NULL;

	doc = calloc(1, sizeof(*doc));
	if (doc == NULL)
		return memFullErr;

	pthread_mutex_lock(&g_tsmLock);
	doc->next = g_documents;
	g_documents = doc;
	pthread_mutex_unlock(&g_tsmLock);

	*idocID = doc;
	return noErr;
}

OSErr DeleteTSMDocument(TSMDocumentID idocID)
{
	TSMDocumentID* link;

	pthread_mutex_lock(&g_tsmLock);

	link = &g_documents;
	while (*link != NULL && *link != idocID)
		link = &(*link)->next;

	if (idocID == NULL || *link == NULL)
	{
		pthread_mutex_unlock(&g_tsmLock);
		return tsmInvalidDocIDErr;
	}

	*link = idocID->next;
	if (g_activeDocument == idocID)
		g_activeDocument = NULL;

	pthread_mutex_unlock(&g_tsmLock);

	while (idocID->properties != NULL)
	{
		struct TSMProperty* prop = idocID->properties;
		idocID->properties = prop->next;
		free(prop);
	}
	free(idocID);

	return noErr;
}

OSErr ActivateTSMDocument(TSMDocumentID idocID)
{
	OSErr err = noErr;

	pthread_mutex_lock(&g_tsmLock);
	if (isValidDocument(idocID))
		g_activeDocument = idocID;
	else
		err = tsmInvalidDocIDErr;
	pthread_mutex_unlock(&g_tsmLock);

	return err;
}

OSErr DeactivateTSMDocument(TSMDocumentID idocID)
{
	OSErr err = noErr;

	pthread_mutex_lock(&g_tsmLock);
	if (!isValidDocument(idocID))
		err = tsmInvalidDocIDErr;
	else if (g_activeDocument != idocID)
		err = tsmDocNotActiveErr;
	else
		g_activeDocument = NULL;
	pthread_mutex_unlock(&g_tsmLock);

	return err;
}

TSMDocumentID TSMGetActiveDocument(void)
{
	TSMDocumentID doc;

	pthread_mutex_lock(&g_tsmLock);
	doc = g_activeDocument;
	pthread_mutex_unlock(&g_tsmLock);

	return doc;
}

OSErr UseInputWindow(TSMDocumentID idocID, Boolean useWindow)
{
	OSErr err = noErr;

	pthread_mutex_lock(&g_tsmLock);
	if (idocID == NULL)
	{
		// NULL applies the setting to every document of the application.
		for (TSMDocumentID doc = g_documents; doc != NULL; doc = doc->next)
			doc->useInputWindow = useWindow;
	}
	else if (isValidDocument(idocID))
		idocID->useInputWindow = useWindow;
	else
		err = tsmInvalidDocIDErr;
	pthread_mutex_unlock(&g_tsmLock);

	return err;
}

OSStatus TSMSetDocumentProperty(TSMDocumentID docID, TSMDocumentPropertyTag propertyTag, UInt32 propertySize, const void *propertyData)
{
	struct TSMProperty* prop;
	struct TSMProperty** link;

	if (propertySize > 0 && propertyData == NULL)
		return paramErr;

	prop = malloc(sizeof(*prop) + propertySize);
	if (prop == NULL)
		return memFullErr;

	prop->tag = propertyTag;
	prop->size = propertySize;
	if (propertySize > 0)
		memcpy(prop->data, propertyData, propertySize);

	pthread_mutex_lock(&g_tsmLock);

	if (!isValidDocument(docID))
	{
		pthread_mutex_unlock(&g_tsmLock);
		free(prop);
		return tsmInvalidDocIDErr;
	}

	// Replace an existing value in place.
	link = findProperty(docID, propertyTag);
	if (*link != NULL)
	{
		prop->next = (*link)->next;
		free(*link);
	}
	else
		prop->next = NULL;
	*link = prop;

	pthread_mutex_unlock(&g_tsmLock);
	return noErr;
}

OSStatus TSMGetDocumentProperty(TSMDocumentID docID, TSMDocumentPropertyTag propertyTag, UInt32 bufferSize, UInt32 *actualSize, void *propertyBuffer)
{
	struct TSMProperty* prop;
	OSStatus err = noErr;

	pthread_mutex_lock(&g_tsmLock);

	if (!isValidDocument(docID))
	{
		pthread_mutex_unlock(&g_tsmLock);
		return tsmInvalidDocIDErr;
	}

	prop = *findProperty(docID, propertyTag);
	if (prop == NULL)
		err = tsmDocPropertyNotFoundErr;
	else
	{
		if (actualSize != NULL)
			*actualSize = prop->size;

		// A NULL buffer only asks for the size.
		if (propertyBuffer != NULL)
		{
			if (bufferSize < prop->size)
				err = tsmDocPropertyBufferTooSmallErr;
			else if (prop->size > 0)
				memcpy(propertyBuffer, prop->data, prop->size);
		}
	}

	pthread_mutex_unlock(&g_tsmLock);
	return err;
}

OSStatus TSMRemoveDocumentProperty(TSMDocumentID docID, TSMDocumentPropertyTag propertyTag)
{
	struct TSMProperty** link;
	struct TSMProperty* prop = NULL;
	OSStatus err = noErr;

	pthread_mutex_lock(&g_tsmLock);

	if (!isValidDocument(docID))
		err = tsmInvalidDocIDErr;
	else
	{
		link = findProperty(docID, propertyTag);
		prop = *link;
		if (prop == NULL)
			err = tsmDocPropertyNotFoundErr;
		else
			*link = prop->next;
	}

	pthread_mutex_unlock(&g_tsmLock);

	free(prop);
	return err;
}

// There are no input methods, so raw key codes never produce dead-key text.
static UInt32 g_deadKeyState;

UInt32 TSMGetDeadKeyState(void)
{
	return g_deadKeyState;
}

void TSMSetDeadKeyState(UInt32 state)
{
	g_deadKeyState = state;
}

OSStatus TSMProcessRawKeyCode(EventRef event)
{
	return eventNotHandledErr;
}

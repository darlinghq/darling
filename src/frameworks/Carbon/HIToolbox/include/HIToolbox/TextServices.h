#ifndef _Carbon_TextServices_H_
#define _Carbon_TextServices_H_

#include <CoreServices/MacTypes.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct OpaqueTSMDocumentID* TSMDocumentID;
typedef OSType InterfaceTypeList[1];
typedef OSType TSMDocumentPropertyTag;

OSErr NewTSMDocument(short numOfInterface, InterfaceTypeList supportedInterfaces, TSMDocumentID* idocID, long refcon);
OSErr DeleteTSMDocument(TSMDocumentID idocID);
OSErr ActivateTSMDocument(TSMDocumentID idocID);
OSErr DeactivateTSMDocument(TSMDocumentID idocID);
TSMDocumentID TSMGetActiveDocument(void);

OSErr UseInputWindow(TSMDocumentID idocID, Boolean useWindow);

OSStatus TSMSetDocumentProperty(TSMDocumentID docID, TSMDocumentPropertyTag propertyTag, UInt32 propertySize, const void *propertyData);
OSStatus TSMGetDocumentProperty(TSMDocumentID docID, TSMDocumentPropertyTag propertyTag, UInt32 bufferSize, UInt32 *actualSize, void *propertyBuffer);
OSStatus TSMRemoveDocumentProperty(TSMDocumentID docID, TSMDocumentPropertyTag propertyTag);

#ifdef __cplusplus
}
#endif

#endif

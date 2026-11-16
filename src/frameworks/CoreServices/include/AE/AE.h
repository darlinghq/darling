#ifndef __AE_H__
#define __AE_H__

#include <AE/AEDataModel.h>
#include <AE/AppleEvents.h>
#include <stdarg.h>

#ifdef __cplusplus
extern "C" {
#endif

void AEInitializeDesc(AEDesc *desc);
OSErr AECreateDesc(DescType typeCode, const void *dataPtr, Size dataSize, AEDesc *result);
OSErr AEDisposeDesc(AEDesc *theAEDesc);
OSErr AEDuplicateDesc(const AEDesc *theAEDesc, AEDesc *result);
Size AEGetDescDataSize(const AEDesc *theAEDesc);
OSErr AEGetDescData(const AEDesc *theAEDesc, void *dataPtr, Size maximumSize);
OSErr AEReplaceDescData(DescType typeCode, const void *dataPtr, Size dataSize, AEDesc *theAEDesc);
OSErr AECoercePtr(DescType typeCode, const void *dataPtr, Size dataSize, DescType toType, AEDesc *result);
OSErr AECoerceDesc(const AEDesc *theAEDesc, DescType toType, AEDesc *result);
Boolean AECheckIsRecord(const AEDesc *theDesc);

OSErr AECreateList(const void *factoringPtr, Size factoredSize, Boolean isRecord, AEDescList *resultList);
OSErr AECountItems(const AEDescList *theAEDescList, long *theCount);
OSErr AEPutDesc(AEDescList *theAEDescList, long index, const AEDesc *theAEDesc);
OSErr AEPutPtr(AEDescList *theAEDescList, long index, DescType typeCode, const void *dataPtr, Size dataSize);
OSErr AEGetNthDesc(const AEDescList *theAEDescList, long index, DescType desiredType, AEKeyword *theAEKeyword, AEDesc *result);
OSErr AEGetNthPtr(const AEDescList *theAEDescList, long index, DescType desiredType, AEKeyword *theAEKeyword,
	DescType *typeCode, void *dataPtr, Size maximumSize, Size *actualSize);
OSErr AEDeleteItem(AEDescList *theAEDescList, long index);
OSErr AESizeOfNthItem(const AEDescList *theAEDescList, long index, DescType *typeCode, Size *dataSize);

OSErr AEPutParamDesc(AERecord *theAERecord, AEKeyword theAEKeyword, const AEDesc *theAEDesc);
OSErr AEPutParamPtr(AERecord *theAERecord, AEKeyword theAEKeyword, DescType typeCode, const void *dataPtr, Size dataSize);
OSErr AEGetParamDesc(const AERecord *theAERecord, AEKeyword theAEKeyword, DescType desiredType, AEDesc *result);
OSErr AEGetParamPtr(const AERecord *theAERecord, AEKeyword theAEKeyword, DescType desiredType, DescType *typeCode,
	void *dataPtr, Size maximumSize, Size *actualSize);
OSErr AESizeOfParam(const AERecord *theAERecord, AEKeyword theAEKeyword, DescType *typeCode, Size *dataSize);
OSErr AEDeleteParam(AERecord *theAERecord, AEKeyword theAEKeyword);

OSErr AECreateAppleEvent(AEEventClass theAEEventClass, AEEventID theAEEventID, const AEAddressDesc *target,
	AEReturnID returnID, AETransactionID transactionID, AppleEvent *result);
OSErr AEPutAttributeDesc(AppleEvent *theAppleEvent, AEKeyword theAEKeyword, const AEDesc *theAEDesc);
OSErr AEPutAttributePtr(AppleEvent *theAppleEvent, AEKeyword theAEKeyword, DescType typeCode, const void *dataPtr, Size dataSize);
OSErr AEGetAttributeDesc(const AppleEvent *theAppleEvent, AEKeyword theAEKeyword, DescType desiredType, AEDesc *result);
OSErr AEGetAttributePtr(const AppleEvent *theAppleEvent, AEKeyword theAEKeyword, DescType desiredType, DescType *typeCode,
	void *dataPtr, Size maximumSize, Size *actualSize);
OSErr AESizeOfAttribute(const AppleEvent *theAppleEvent, AEKeyword theAEKeyword, DescType *typeCode, Size *dataSize);
OSStatus AESendMessage(const AppleEvent *event, AppleEvent *reply, AESendMode sendMode, long timeOutInTicks);
OSErr AESend(const AppleEvent *theAppleEvent, AppleEvent *reply, AESendMode sendMode, AESendPriority sendPriority,
	SInt32 timeOutInTicks, AEIdleUPP idleProc, AEFilterUPP filterProc);

OSErr CreateObjSpecifier(DescType desiredClass, AEDesc *theContainer, DescType keyForm, AEDesc *keyData,
	Boolean disposeInputs, AEDesc *objSpecifier);

OSStatus AEFlattenDesc(const AEDesc *theAEDesc, Ptr buffer, Size bufferSize, Size *actualSize);
OSStatus AEUnflattenDesc(const void *buffer, AEDesc *result);

OSErr AEInstallSpecialHandler(AEKeyword functionClass, AEEventHandlerUPP handler, Boolean isSysHandler);
OSErr AERemoveSpecialHandler(AEKeyword functionClass, AEEventHandlerUPP handler, Boolean isSysHandler);
OSErr AEGetSpecialHandler(AEKeyword functionClass, AEEventHandlerUPP *handler, Boolean isSysHandler);

OSStatus AEPrintDescToHandle(const AEDesc *desc, Handle *result);

// Why parsing a format string failed.
enum {
	aeBuildSyntaxNoErr = 0,
	aeBuildSyntaxBadToken = 1,           // unexpected character
	aeBuildSyntaxBadEOF = 2,             // the string ended too early
	aeBuildSyntaxNoEOF = 3,              // extra text after the end of the description
	aeBuildSyntaxBadNegative = 4,        // "-" not followed by digits
	aeBuildSyntaxMissingQuote = 5,       // unterminated quoted four-character code
	aeBuildSyntaxBadHex = 6,             // non-hex character between $ signs
	aeBuildSyntaxOddHex = 7,             // odd number of hex digits
	aeBuildSyntaxNoCloseHex = 8,         // missing closing $
	aeBuildSyntaxUncoercedHex = 9,       // hex string without a type
	aeBuildSyntaxNoCloseString = 10,     // missing closing quote of a string
	aeBuildSyntaxBadDesc = 11,           // not a descriptor
	aeBuildSyntaxBadData = 12,           // bad data inside parentheses
	aeBuildSyntaxNoCloseParen = 13,      // missing ")"
	aeBuildSyntaxNoCloseBracket = 14,    // expected "," or "]"
	aeBuildSyntaxNoCloseBrace = 15,      // expected "," or "}"
	aeBuildSyntaxNoKey = 16,             // missing key in a record
	aeBuildSyntaxNoColon = 17,           // missing ":" after a key
	aeBuildSyntaxCoercedList = 18,       // a list cannot be given a type
	aeBuildSyntaxUncoercedDoubleAt = 19, // "@@" without a type
};
typedef UInt32 AEBuildErrorCode;

typedef struct AEBuildError {
	AEBuildErrorCode fError;
	UInt32 fErrorPos;
} AEBuildError;

OSStatus AEBuildDesc(AEDesc *dst, AEBuildError *error, const char *src, ...);
OSStatus vAEBuildDesc(AEDesc *dst, AEBuildError *error, const char *src, va_list args);
OSStatus AEBuildParameters(AppleEvent *event, AEBuildError *error, const char *format, ...);
OSStatus vAEBuildParameters(AppleEvent *event, AEBuildError *error, const char *format, va_list args);
OSStatus AEBuildAppleEvent(AEEventClass theClass, AEEventID theID, DescType addressType, const void *addressData,
	long addressLength, SInt16 returnID, SInt32 transactionID, AppleEvent *result, AEBuildError *error,
	const char *paramsFmt, ...);
OSStatus vAEBuildAppleEvent(AEEventClass theClass, AEEventID theID, DescType addressType, const void *addressData,
	long addressLength, SInt16 returnID, SInt32 transactionID, AppleEvent *result, AEBuildError *error,
	const char *paramsFmt, va_list args);

#ifdef __cplusplus
}
#endif

#endif

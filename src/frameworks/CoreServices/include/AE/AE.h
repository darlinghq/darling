#ifndef __AE_H__
#define __AE_H__

#include <AE/AEDataModel.h>
#include <AE/AppleEvents.h>

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

OSErr AEInstallSpecialHandler(AEKeyword functionClass, AEEventHandlerUPP handler, Boolean isSysHandler);
OSErr AERemoveSpecialHandler(AEKeyword functionClass, AEEventHandlerUPP handler, Boolean isSysHandler);
OSErr AEGetSpecialHandler(AEKeyword functionClass, AEEventHandlerUPP *handler, Boolean isSysHandler);

OSStatus AEPrintDescToHandle(const AEDesc *desc, Handle *result);

#ifdef __cplusplus
}
#endif

#endif

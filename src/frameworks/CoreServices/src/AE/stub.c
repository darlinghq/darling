#include <AE/AE.h>

static int verbose = 0;
__attribute__((constructor)) static void initme(void) {
    verbose = getenv("STUB_VERBOSE") != NULL;
}

OSErr AECreateDesc(DescType typeCode, const void *dataPtr, Size dataSize, AEDesc *result) {
    if (verbose) puts("STUB: AECreateDesc called");
    return noErr;
}

OSErr AEDisposeDesc(AEDesc *theAEDesc) {
    if (verbose) puts("STUB: AEDisposeDesc called");
    return noErr;
}

OSErr AEInstallEventHandler(
  AEEventClass theAEEventClass,
  AEEventID theAEEventID,
  AEEventHandlerUPP handler,
  SRefCon handlerRefcon,
  Boolean isSysHandler
) {
    if (verbose) puts("STUB: AEInstallEventHandler called");
    return noErr;
}

OSStatus AECreateAppleEvent(
  AEEventClass theAEEventClass,
  AEEventID theAEEventID,
  const AEAddressDesc *target,
  AEReturnID returnID,
  AETransactionID transactionID,
  AppleEvent *result
) {
    if (verbose) puts("STUB: AECreateAppleEvent called");
    return errAECoercionFail;
}

OSErr AEGetParamDesc(const AppleEvent *theAppleEvent, AEKeyword theAEKeyword, DescType desiredType, AEDesc *result) {
    if (verbose) puts("STUB: AEGetParamDesc called");
    return errAEDescNotFound;
}

OSErr AEPutParamDesc(AppleEvent *theAppleEvent, AEKeyword theAEKeyword, const AEDesc *theAEDesc) {
    if (verbose) puts("STUB: AEPutParamDesc called");
    return noErr;
}

OSErr AERemoveEventHandler(
  AEEventClass theAEEventClass,
  AEEventID theAEEventID,
  AEEventHandlerUPP handler,
  Boolean isSysHandler
) {
    if (verbose) puts("STUB: AERemoveEventHandler called");
    return noErr;
}

OSStatus AESendMessage(const AppleEvent *event, AppleEvent *reply, AESendMode sendMode, long timeOutInTicks) {
    if (verbose) puts("STUB: AESendMessage called");
    return errAETimeout;
}

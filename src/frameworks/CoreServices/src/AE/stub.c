#include <AE/AE.h>

static int verbose = 0;
__attribute__((constructor)) static void initme(void) {
    verbose = getenv("STUB_VERBOSE") != NULL;
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

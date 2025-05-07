#ifndef __AE_H__
#define __AE_H__

#include <AE/AEDataModel.h>
#include <AE/AppleEvents.h>

OSErr AECreateDesc(DescType typeCode, const void *dataPtr, Size dataSize, AEDesc *result);
OSErr AEDisposeDesc(AEDesc *theAEDesc);
OSErr AEInstallEventHandler(AEEventClass theAEEventClass, AEEventID theAEEventID, AEEventHandlerUPP handler, SRefCon handlerRefcon, Boolean isSysHandler);

#endif

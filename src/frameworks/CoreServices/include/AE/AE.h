#ifndef __AE_H__
#define __AE_H__

#include <AE/AEDataModel.h>

OSErr AECreateDesc(DescType typeCode, const void *dataPtr, Size dataSize, AEDesc *result);
OSErr AEDisposeDesc(AEDesc *theAEDesc);

enum : AEEventID {
kAEOpenApplication = 'oapp',
kAEReopenApplication = 'rapp',
kAEOpenDocuments = 'odoc',
kAEPrintDocuments = 'pdoc',
kAEOpenContents = 'ocon',
kAEQuitApplication = 'quit',
kAEAnswer = 'ansr',
kAEApplicationDied = 'obit',
kAEShowPreferences = 'pref',
};

enum : DescType {
    kCoreEventClass = 'aevt',
};

#endif

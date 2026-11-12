#ifndef __AppleEvents_H__
#define __AppleEvents_H__


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
    kInternetEventClass = 'GURL',
};

enum : AEEventID {
    kAEGetURL = 'GURL',
};

OSErr AEInstallEventHandler(AEEventClass theAEEventClass, AEEventID theAEEventID, AEEventHandlerUPP handler, SRefCon handlerRefcon, Boolean isSysHandler);
OSErr AERemoveEventHandler(AEEventClass theAEEventClass, AEEventID theAEEventID, AEEventHandlerUPP handler, Boolean isSysHandler);
OSErr AEGetEventHandler(AEEventClass theAEEventClass, AEEventID theAEEventID, AEEventHandlerUPP *handler, SRefCon *handlerRefcon, Boolean isSysHandler);

#endif

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
};

#endif

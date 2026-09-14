#ifndef __AE_DATAMODEL_H__
#define __AE_DATAMODEL_H__

#ifndef __CARBONCORE__
#include <CarbonCore/CarbonCore.h>
#endif

#include <Availability.h>

typedef SInt32 AESendMode;

typedef ResType DescType;
typedef FourCharCode                    AEKeyword;

#if OPAQUE_TOOLBOX_STRUCTS
typedef struct OpaqueAEDataStorageType* AEDataStorageType;
#else
typedef Ptr AEDataStorageType;
#endif

typedef AEDataStorageType* AEDataStorage;

struct AEDesc {
	DescType            descriptorType;
	AEDataStorage       dataHandle;
};

typedef struct AEDesc                   AEDesc;
typedef AEDesc *                        AEDescPtr;

typedef AEDesc AEAddressDesc;

struct AEKeyDesc {
	AEKeyword           descKey;
	AEDesc              descContent;
};
typedef struct AEKeyDesc                AEKeyDesc;

typedef AEDesc                          AEDescList;
typedef AEDescList                      AERecord;
typedef AEDesc                          AEAddressDesc;
typedef AERecord                        AppleEvent;
typedef AppleEvent *                    AppleEventPtr;
typedef SInt16                          AEReturnID;
typedef SInt32                          AETransactionID;
typedef FourCharCode                    AEEventClass;
typedef FourCharCode                    AEEventID;
typedef SInt8                           AEArrayType;

enum : DescType {
	typeBoolean = 'bool',
	typeChar = 'TEXT',
	typeSInt16 = 'shor',
	typeSInt32 = 'long',
	typeUInt32 = 'magn',
	typeSInt64 = 'comp',
	typeIEEE32BitFloatingPoint = 'sing',
	typeIEEE64BitFloatingPoint = 'doub',
	typeUTF8Text = 'utf8',
	typeUnicodeText = 'utxt',
	typeType = 'type',
	typeEnumerated = 'enum',
	typeFileURL = 'furl',
	typeApplicationBundleID = 'bund',
	typeKernelProcessID = 'kpid',
	typeAEList = 'list',
	typeAERecord = 'reco',
	typeAppleEvent = 'aevt',
	typeTrue = 'true',
	typeFalse = 'fals',
	typeNull = 'null',
	typeWildCard = '****',
};

enum : AEKeyword {
	keyTransactionIDAttr = 'tran',
	keyReturnIDAttr = 'rtid',
	keyEventClassAttr = 'evcl',
	keyEventIDAttr = 'evid',
	keyAddressAttr = 'addr',
	keyOptionalKeywordAttr = 'optk',
	keyTimeoutAttr = 'timo',
	keyInteractLevelAttr = 'inte',
	keyEventSourceAttr = 'esrc',
	keyMissedKeywordAttr = 'miss',
	keyOriginalAddressAttr = 'from',
	keyDirectObject = '----',
	keyErrorNumber = 'errn',
	keyErrorString = 'errs',
};

typedef OSErr (*AEEventHandlerProcPtr)(
	const AppleEvent* theAppleEvent,
	AppleEvent* reply,
	SRefCon handlerRefcon
);
typedef AEEventHandlerProcPtr           AEEventHandlerUPP;

#endif

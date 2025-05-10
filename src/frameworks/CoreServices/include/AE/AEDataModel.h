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

typedef OSErr (*AEEventHandlerProcPtr)(
	const AppleEvent* theAppleEvent,
	AppleEvent* reply,
	SRefCon handlerRefcon
);
typedef AEEventHandlerProcPtr           AEEventHandlerUPP;

#endif

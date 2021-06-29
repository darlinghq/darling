#include <CoreServices/CoreServices.h>
#include <QD/QD.h>

#include <HIToolbox/HIObject.h>

typedef Point Cell;
typedef Rect ListBounds;
typedef char DataArray[32001];
typedef char* DataPtr;
typedef DataPtr* DataHandle;
typedef CALLBACK_API(short, ListSearchProcPtr)(Ptr aPtr, Ptr bPtr, short aLen, short bLen);
typedef CALLBACK_API( Boolean , ListClickLoopProcPtr )(void);
typedef STACK_UPP_TYPE(ListSearchProcPtr) ListSearchUPP;
typedef STACK_UPP_TYPE(ListClickLoopProcPtr) ListClickLoopUPP;

struct ListRec {
	Rect rView;
	GrafPtr port;
	Point indent;
	Point cellSize;
	ListBounds visible;
	ControlRef vScroll;
	ControlRef hScroll;
	SInt8 selFlags;
	Boolean lActive;
	SInt8 lReserved;
	SInt8 listFlags;
	long clikTime;
	Point clikLoc;
	Point mouseLoc;
	ListClickLoopUPP lClickLoop;
	Cell lastClick;
	long refCon;
	Handle listDefProc;
	Handle userHandle;
	ListBounds dataBounds;
	DataHandle cells;
	short maxIndex;
	short cellArray[1];
};
typedef struct ListRec ListRec;
typedef ListRec* ListPtr;
typedef ListPtr* ListHandle;

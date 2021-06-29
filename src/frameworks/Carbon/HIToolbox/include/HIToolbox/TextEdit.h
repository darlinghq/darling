#ifndef _HITOOLBOX_TEXTEDIT_H_
#define _HITOOLBOX_TEXTEDIT_H_

#include <CoreServices/CoreServices.h>
#include <QD/QD.h>

typedef struct TERec TERec;
typedef TERec* TEPtr;
typedef TEPtr* TEHandle;

typedef CALLBACK_API(void, HighHookProcPtr)(const Rect* r, TEPtr pTE);
typedef CALLBACK_API(Boolean, EOLHookProcPtr)(char theChar, TEPtr pTE, TEHandle hTE);
typedef CALLBACK_API(void, CaretHookProcPtr)(const Rect* r, TEPtr pTE);
typedef CALLBACK_API(unsigned short, WidthHookProcPtr)(unsigned short textLen, unsigned short textOffset, void* textBufferPtr, TEPtr pTE, TEHandle hTE);
typedef CALLBACK_API(unsigned short, TextWidthHookProcPtr)(unsigned short textLen, unsigned short textOffset, void* textBufferPtr, TEPtr pTE, TEHandle hTE);
typedef CALLBACK_API(unsigned short, NWidthHookProcPtr)(unsigned short styleRunLen, unsigned short styleRunOffset, short slop, short direction, void *textBufferPtr, short* lineStart, TEPtr pTE, TEHandle hTE);
typedef CALLBACK_API(void, DrawHookProcPtr)(unsigned short textOffset, unsigned short drawLen, void* textBufferPtr, TEPtr pTE, TEHandle hTE);
typedef CALLBACK_API(Boolean, HitTestHookProcPtr)(unsigned short styleRunLen, unsigned short styleRunOffset, unsigned short slop, void* textBufferPtr, TEPtr pTE, TEHandle hTE, unsigned short *pixelWidth, unsigned short* charOffset, Boolean* pixelInChar);
typedef CALLBACK_API(void, TEFindWordProcPtr)(unsigned short currentPos, short caller, TEPtr pTE, TEHandle hTE, unsigned short* wordStart, unsigned short* wordEnd);
typedef CALLBACK_API(void, TERecalcProcPtr)(TEPtr pTE, unsigned short changeLength, unsigned short* lineStart, unsigned short* firstChar, unsigned short* lastChar);
typedef CALLBACK_API(void, TEDoTextProcPtr)(TEPtr pTE, unsigned short firstChar, unsigned short lastChar, short selector, GrafPtr* currentGrafPort, short* charPosition);
typedef CALLBACK_API(Boolean, TEClickLoopProcPtr)(TEPtr pTE);
typedef CALLBACK_API(Boolean, WordBreakProcPtr)(Ptr text, short charPos);

typedef STACK_UPP_TYPE(HighHookProcPtr) HighHookUPP;
typedef STACK_UPP_TYPE(EOLHookProcPtr) EOLHookUPP;
typedef STACK_UPP_TYPE(CaretHookProcPtr) CaretHookUPP;
typedef STACK_UPP_TYPE(WidthHookProcPtr) WidthHookUPP;
typedef STACK_UPP_TYPE(TextWidthHookProcPtr) TextWidthHookUPP;
typedef STACK_UPP_TYPE(NWidthHookProcPtr) NWidthHookUPP;
typedef STACK_UPP_TYPE(DrawHookProcPtr) DrawHookUPP;
typedef STACK_UPP_TYPE(HitTestHookProcPtr) HitTestHookUPP;
typedef STACK_UPP_TYPE(TEFindWordProcPtr) TEFindWordUPP;
typedef STACK_UPP_TYPE(TERecalcProcPtr) TERecalcUPP;
typedef STACK_UPP_TYPE(TEDoTextProcPtr) TEDoTextUPP;
typedef STACK_UPP_TYPE(TEClickLoopProcPtr) TEClickLoopUPP;
typedef STACK_UPP_TYPE(WordBreakProcPtr) WordBreakUPP;

struct TERec {
	Rect destRect;
	Rect viewRect;
	Rect selRect;
	short lineHeight;
	short fontAscent;
	Point selPoint;
	short selStart;
	short selEnd;
	short active;
	WordBreakUPP wordBreak;
	TEClickLoopUPP clickLoop;
	long clickTime;
	short clickLoc;
	long caretTime;
	short caretState;
	short just;
	short teLength;
	Handle hText;
	long hDispatchRec;
	short clikStuff;
	short crOnly;
	short txFont;
	StyleField txFace;
	short txMode;
	short txSize;
	GrafPtr inPort;
	HighHookUPP highHook;
	CaretHookUPP caretHook;
	short nLines;
	short lineStarts[16001];
};

#endif // _HITOOLBOX_TEXTEDIT_H_

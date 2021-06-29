
#pragma GCC visibility push(hidden)

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include <Carbon/Carbon.h>
#include "MacTypes.h"

#define FW_EXPORT __attribute__ ((visibility ("default")))

typedef UInt32 DisplayIDType;

typedef void* DSpContextReference;
typedef const void* DSpContextReferenceConst;
typedef void* DSpContextAttributesPtr;

typedef uint8_t DSpBufferKind;
typedef uint8_t DSpContextState;

typedef void* DSpCallbackUPP;

static int verbose = 0;

__attribute__((constructor))
static void initme(void) {
    verbose = getenv("STUB_VERBOSE") != NULL;
}

FW_EXPORT OSStatus DSpContext_Dispose(DSpContextReference ref)
{
   if (verbose) puts("STUB: DSpContext_Dispose called");
   return 0;
}

FW_EXPORT OSStatus DSpContext_FadeGamma(DSpContextReference ref, SInt32 a, RGBColor *b)
{
   if (verbose) puts("STUB: DSpContext_FadeGamma called");
   return 0;
}

FW_EXPORT OSStatus DSpContext_FadeGammaIn(DSpContextReference ref, RGBColor *a)
{
   if (verbose) puts("STUB: DSpContext_FadeGammaIn called");
   return 0;
}

FW_EXPORT OSStatus DSpContext_FadeGammaOut(DSpContextReference ref, RGBColor *a)
{
   if (verbose) puts("STUB: DSpContext_FadeGammaOut called");
   return 0;
}

FW_EXPORT OSStatus DSpContext_GetAttributes(DSpContextReferenceConst ref, DSpContextAttributesPtr a)
{
   if (verbose) puts("STUB: DSpContext_GetAttributes called");
   return 0;
}

FW_EXPORT OSStatus DSpContext_GetBackBuffer(DSpContextReference ref, DSpBufferKind a, CGrafPtr *b)
{
   if (verbose) puts("STUB: DSpContext_GetBackBuffer called");
   return 0;
}

FW_EXPORT OSStatus DSpContext_GetCLUTEntries(DSpContextReferenceConst ref, ColorSpec *a, UInt16 b, UInt16 c)
{
   if (verbose) puts("STUB: DSpContext_GetCLUTEntries called");
   return 0;
}

FW_EXPORT OSStatus DSpContext_GetDisplayID(DSpContextReferenceConst ref, DisplayIDType *a)
{
   if (verbose) puts("STUB: DSpContext_GetDisplayID called");
   return 0;
}

FW_EXPORT OSStatus DSpContext_GetFrontBuffer(DSpContextReferenceConst ref, CGrafPtr *a)
{
   if (verbose) puts("STUB: DSpContext_GetFrontBuffer called");
   return 0;
}

FW_EXPORT OSStatus DSpContext_GetMonitorFrequency(DSpContextReferenceConst ref, Fixed *a)
{
   if (verbose) puts("STUB: DSpContext_GetMonitorFrequency called");
   return 0;
}

FW_EXPORT OSStatus DSpContext_GetState(DSpContextReferenceConst ref, DSpContextState *a)
{
   if (verbose) puts("STUB: DSpContext_GetState called");
   return 0;
}

FW_EXPORT OSStatus DSpContext_GlobalToLocal(DSpContextReferenceConst ref, Point *a)
{
   if (verbose) puts("STUB: DSpContext_GlobalToLocal called");
   return 0;
}

FW_EXPORT OSStatus DSpContext_IsBusy(DSpContextReferenceConst ref, Boolean *a)
{
   if (verbose) puts("STUB: DSpContext_IsBusy called");
   return 0;
}

FW_EXPORT OSStatus DSpContext_LocalToGlobal(DSpContextReferenceConst ref, Point *a)
{
   if (verbose) puts("STUB: DSpContext_LocalToGlobal called");
   return 0;
}

FW_EXPORT OSStatus DSpContext_Queue(DSpContextReference ref, DSpContextReference ref2, DSpContextAttributesPtr a)
{
   if (verbose) puts("STUB: DSpContext_Queue called");
   return 0;
}

FW_EXPORT OSStatus DSpContext_Release(DSpContextReference ref)
{
   if (verbose) puts("STUB: DSpContext_Release called");
   return 0;
}

FW_EXPORT OSStatus DSpContext_Reserve(DSpContextReference ref, DSpContextAttributesPtr a)
{
   if (verbose) puts("STUB: DSpContext_Reserve called");
   return 0;
}
const
FW_EXPORT OSStatus DSpContext_SetCLUTEntries(DSpContextReference ref, const ColorSpec *a, UInt16 b, UInt16 c)
{
   if (verbose) puts("STUB: DSpContext_SetCLUTEntries called");
   return 0;
}

FW_EXPORT OSStatus DSpContext_SetState(DSpContextReference ref, DSpContextState a)
{
   if (verbose) puts("STUB: DSpContext_SetState called");
   return 0;
}

FW_EXPORT OSStatus DSpContext_SwapBuffers(DSpContextReference ref, DSpCallbackUPP a, void *b)
{
   if (verbose) puts("STUB: DSpContext_SwapBuffers called");
   return 0;
}

FW_EXPORT OSStatus DSpContext_Switch(DSpContextReference ref, DSpContextReference ref2)
{
   if (verbose) puts("STUB: DSpContext_Switch called");
   return 0;
}

FW_EXPORT OSStatus DSpFindBestContext(DSpContextAttributesPtr a, DSpContextReference *ref)
{
   if (verbose) puts("STUB: DSpFindBestContext called");
   return 0;
}

FW_EXPORT OSStatus DSpFindBestContextOnDisplayID(DSpContextAttributesPtr a, DSpContextReference *b, DisplayIDType c)
{
   if (verbose) puts("STUB: DSpFindBestContextOnDisplayID called");
   return 0;
}

FW_EXPORT OSStatus DSpFindContextFromPoint(Point a, DSpContextReference *ref)
{
   if (verbose) puts("STUB: DSpFindContextFromPoint called");
   return 0;
}

FW_EXPORT OSStatus DSpGetCurrentContext(DisplayIDType typeID, DSpContextReference *ref)
{
   if (verbose) puts("STUB: DSpGetCurrentContext called");
   return 0;
}

FW_EXPORT OSStatus DSpGetFirstContext(DisplayIDType typeID, DSpContextReference *ref)
{
   if (verbose) puts("STUB: DSpGetFirstContext called");
   return 0;
}

FW_EXPORT OSStatus DSpGetMouse(Point *a)
{
   if (verbose) puts("STUB: DSpGetMouse called");
   return 0;
}

FW_EXPORT OSStatus DSpGetNextContext(DSpContextReference ref, DSpContextReference *ref2)
{
   if (verbose) puts("STUB: DSpGetNextContext called");
   return 0;
}

FW_EXPORT NumVersion DSpGetVersion(void)
{
   if (verbose) puts("STUB: DSpGetVersion called");
   NumVersion ret;
   return ret;
}

FW_EXPORT OSStatus DSpProcessEvent(EventRecord *a, Boolean *b)
{
   if (verbose) puts("STUB: DSpProcessEvent called");
   return 0;
}

FW_EXPORT OSStatus DSpSetBlankingColor(const RGBColor * a)
{
   if (verbose) puts("STUB: DSpSetBlankingColor called");
   return 0;
}

FW_EXPORT OSStatus DSpSetDebugMode(Boolean a)
{
   if (verbose) puts("STUB: DSpSetDebugMode called");
   return 0;
}

FW_EXPORT OSStatus DSpShutdown(void)
{
   if (verbose) puts("STUB: DSpShutdown called");
   return 0;
}

FW_EXPORT OSStatus DSpStartup(void)
{
   if (verbose) puts("STUB: DSpStartup called");
   return 0;
}


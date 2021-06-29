#ifndef _Carbon_Dialogs_H_
#define _Carbon_Dialogs_H_

#ifdef __cplusplus
extern "C" {
#endif

typedef int16_t AlertType;
typedef void* DialogRef;
typedef DialogRef DialogPtr;
typedef uint32_t AlertStdCFStringAlertParamRec;
typedef void* AlertStdCFStringAlertParamPtr;

typedef int16_t DialogItemIndex;
typedef void* ModalFilterUPP;

OSStatus CreateStandardAlert(AlertType a, CFStringRef b, CFStringRef c, const AlertStdCFStringAlertParamRec * d, DialogRef * e);
OSStatus GetStandardAlertDefaultParams(AlertStdCFStringAlertParamPtr a, UInt32 b);

OSStatus RunStandardAlert(DialogRef a, ModalFilterUPP b, DialogItemIndex * c);


#ifdef __cplusplus
}
#endif

#endif

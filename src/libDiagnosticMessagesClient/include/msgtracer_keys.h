#ifndef __MSGTRACER_KEYS_H__
#define __MSGTRACER_KEYS_H__

// from:
//   `Security/OSX/libsecurity_ssl/Security/sslTransport.c`
//   `Security/securityd/src/token.cpp`
//   `IOKitUser/hidsystem.subproj/IOHIDLib.c`
// plus a little bit of common sense ;)
//
// once again, i am amazed at Apple's failure to properly hide things
#define kMsgTracerKeySignature "com.apple.message.signature"
#define kMsgTracerKeySummarize "com.apple.message.summarize"
#define kMsgTracerKeySignature2 "com.apple.message.signature2"

// these are good guesses
#define kMsgTracerKeyValue "com.apple.message.value"
#define kMsgTracerKeyValue2 "com.apple.message.value2"

#endif // __MSGTRACER_KEYS_H__

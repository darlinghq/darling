#ifndef _ACMDEFS_H_
#define _ACMDEFS_H_
#include <CoreFoundation/CFString.h>

// No one knows what these are for

// Someone please reverse-engineer this,
// these are guessed and not verified

#define kACMPolicyDeviceOwnerAuthentication "ACMPolicyDeviceOwnerAuthentication"
#define kACMKeyAclConstraintPolicy "ACMKeyAclConstraintPolicy"

#define kAKSKeyOpSync CFSTR("AKSKeyOpSync")
//#define kAKSKeyOpSign CFSTR("AKSKeyOpSign")
#define kAKSKeyOpDelete CFSTR("AKSKeyOpDelete")
//#define kAKSKeyOpDecrypt CFSTR("AKSKeyOpDecrypt")
#define kAKSKeyOpEncrypt CFSTR("AKSKeyOpEncrypt")
#define kAKSKeyOpDelete CFSTR("AKSKeyOpDelete")

extern const int kAKSKeyOpSign;

extern const CFStringRef kAKSKeyOpDecrypt;

extern const int kAKSKeyOpComputeKey;

extern const CFStringRef kAKSKeyOpAttest;


#endif

#ifndef _ACMACLDEFS_H_
#define _ACMACLDEFS_H_

#include <CoreFoundation/CoreFoundation.h>

// Someone please reverse-engineer this,
// these are guessed and not verified
#define kAKSKeyAcl CFSTR("AKSKeyAcl")
#define kAKSKeyOpDefaultAcl CFSTR("AKSKeyOpDefaultAcl")
#define kACMKeyAclConstraintUserPasscode "ACMKeyAclConstraintUserPasscode"
#define kACMKeyAclParamBioCatacombUUID "ACMKeyAclParamBioCatacombUUID"
#define kACMKeyAclConstraintBio "ACMKeyAclConstraintBio"
#define kACMKeyAclParamBioCatacombUUID "ACMKeyAclParamBioCatacombUUID"
#define kACMKeyAclParamBioDatabaseHash "ACMKeyAclParamBioDatabaseHash"
#define kACMKeyAclParamKofN "ACMKeyAclParamKofN"
#define kACMKeyAclConstraintKofN "ACMKeyAclConstraintKofN"
#define kAKSKeyAclParamRequirePasscode CFSTR("AKSKeyAclParamRequirePasscode")

#endif

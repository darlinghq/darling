#ifndef LocalAuthentication_LAPublicDefines_h
#define LocalAuthentication_LAPublicDefines_h

#define kLAPolicyDeviceOwnerAuthenticationWithBiometrics    1
#define kLAPolicyDeviceOwnerAuthentication                  2

#define kLAOptionUserFallback                               1
#define kLAOptionAuthenticationReason                       2

#define kLACredentialTypePasscode                          -1
#define kLACredentialTypePassphrase                        -2
#define kLACredentialCTKPIN                                -3

#define kLAErrorAuthenticationFailed                       -1
#define kLAErrorUserCancel                                 -2
#define kLAErrorUserFallback                               -3
#define kLAErrorSystemCancel                               -4
#define kLAErrorPasscodeNotSet                             -5
#define kLAErrorTouchIDNotAvailable                        -6
#define kLAErrorTouchIDNotEnrolled                         -7
#define kLAErrorTouchIDLockout                             -8
#define kLAErrorAppCancel                                  -9
#define kLAErrorInvalidContext                            -10

#define kLAErrorDomain        "com.apple.LocalAuthentication"

#endif

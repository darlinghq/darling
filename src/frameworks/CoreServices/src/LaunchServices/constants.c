#include <LaunchServices/LaunchServices.h>
#include <LaunchServices/LaunchServicesPriv.h>

const CFStringRef _kLSArchitecturePowerPCValue = CFSTR("ppc");
const CFStringRef _kLSArchitecturePowerPC64Value = CFSTR("ppc64");
const CFStringRef _kLSArchitecturei386Value = CFSTR("i386");
const CFStringRef _kLSArchitecturex86_64Value = CFSTR("x86_64");
const CFStringRef _kLSArchitectureUnknownValue = CFSTR("unknown");

const CFStringRef _kLSPIDKey = CFSTR("LSPID");
const CFStringRef _kLSExecutablePathKey = CFSTR("LSExecutablePath");
const CFStringRef _kLSCheckInTimeKey = CFSTR("LSCheckInTime");
const CFStringRef _kLSDisplayNameKey = CFSTR("LSDisplayName");
const CFStringRef _kLSArchitectureKey = CFSTR("LSArchitecture");
const CFStringRef _kLSBundlePathKey = CFSTR("LSBundlePath");

const CFStringRef _kLSApplicationTypeKey = CFSTR("ApplicationType");
const CFStringRef _kLSApplicationForegroundTypeKey = CFSTR("Foreground");
const CFStringRef _kLSApplicationBackgroundOnlyTypeKey = CFSTR("BackgroundOnly");

const CFStringRef _kLSExecutableFormatKey = CFSTR("LSExecutableFormat");
const CFStringRef _kLSExecutableFormatMachOKey = CFSTR("LSExecutableMachOFormat");
const CFStringRef _kLSExecutableFormatCFMKey = CFSTR("LSExecutableCFMFormat");

const CFStringRef _kLSASNKey = CFSTR("LSASN");

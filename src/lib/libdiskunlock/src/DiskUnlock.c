/*
	libDiskUnlock

	Unlocks a volume, which is a privileged operation the installer and setup paths
	hand off to. Boot Camp Assistant names this library in a load command and
	calls into it.

	DUUnlockVolume is declared with an untyped object parameter rather than an
	NSString, because the caller passes a bridged NSString but this is a C
	translation unit with no Foundation headers available to name the type. The
	parameter is a single object pointer either way, so the ABI is unaffected.

	There is no disk to unlock here, and unlocking is a privileged operation that
	needs a real volume to act on, so this does nothing. The caller discards the
	result, which is consistent with a request whose outcome it does not report.
*/

typedef int libdiskunlock_translation_unit_not_empty;

void DUUnlockVolume(void* volumeName)
{
	(void)volumeName;
}

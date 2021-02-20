#include <CarbonCore/BackupCore.h>

// these are technically stubs
// although, since Darling doesn't do backups, these are basically implemented
// the return values are *technically* correct: the items being queried *are* excluded from backups...
// ...it's just that *all* items are excluded from backups :)

Boolean CSBackupIsItemExcluded(CFURLRef item, Boolean* excludeByPath) {
	// stub
	if (excludeByPath) {
		*excludeByPath = false;
	}
	return true;
};

OSStatus CSBackupSetItemExcluded(CFURLRef item, Boolean exclude, Boolean excludeByPath) {
	// stub
	return noErr;
};

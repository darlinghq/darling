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

// Private SPI used by the Time Machine app to talk to backupd. Darling has no backup server,
// so report that there is none.
void* _CSBackupGetSharedServerProxy(void) {
	return NULL;
};

Boolean _CSBackupServerIsActive(void) {
	return false;
};

void _CSBackupServerProxyCancelBackup(void* proxy) {
	// no backup can be running
};

project(SyncServices)

remove_sdk_framework(SyncServices)

set(DYLIB_COMPAT_VERSION "48.0.0")
set(DYLIB_CURRENT_VERSION "727.0.0")

set(FRAMEWORK_VERSION "A")

generate_sdk_framework(SyncServices
    VERSION ${FRAMEWORK_VERSION}
    HEADER "include/SyncServices"
)

add_framework(SyncServices
	FAT
	CURRENT_VERSION
	VERSION ${FRAMEWORK_VERSION}

	SOURCES
		src/SyncServices.m
		src/ISDKeyedArchiverHelperToRemoveSyncStates.m
		src/ISDAdminDatabase.m
		src/DataMgrBasedNameNumberProvider.m
		src/ISDDataManager.m
		src/ISDSchema.m
		src/ISDSchemaParser.m
		src/ISDSyncPlan.m
		src/ISDSyncParticipant.m
		src/ISDEntityComponent.m
		src/ISDException.m
		src/ISDSyncManager.m
		src/ISDSyncJob.m
		src/ServerBasedNameNumberProvider.m
		src/ISDServerFileLockerDelegate.m
		src/ISDServer.m
		src/ISDDisabledServer.m
		src/SyncClientNameNumberProvider.m
		src/ISyncConcreteManager.m
		src/ISyncManager.m
		src/RelationshipInformation.m
		src/ISDChangeEnumerator.m
		src/ISDFiltratorThingy.m
		src/ISyncConcreteSession.m
		src/ISDInstrumentedEnumerator.m
		src/ISyncInstrumentedSession.m
		src/ISyncSession.m
		src/ISyncConcreteChange.m
		src/ISyncChange.m
		src/ISyncConjunctionFilter.m
		src/ISyncFilter.m
		src/ISyncSimplyIgnoreLocalsFilter.m
		src/ISyncAlwaysAcceptFilter.m
		src/ISyncTCCAccessOperation.m
		src/ISDSaveSyncServicesDirectoryCopyHandler.m
		src/ISyncConcreteClient.m
		src/ISyncClient.m
		src/ISyncConcreteRecordSnapshot.m
		src/ISyncRecordSnapshot.m
		src/ISDDataDatabase.m
		src/ISDDatabase.m
		src/ISDChange.m
		src/ISDPropertyChange.m
		src/ISDChangeStore.m
		src/ISDRecord.m
		src/ISDRecordStore.m
		src/ISDChangeBuilder.m
		src/ISDChangePusher.m
		src/ISDChangePuller.m
		src/ISDStructuredDelta.m
		src/ISDPropertyValue.m
		src/ISDConflict.m
		src/ISDConflictChangeEnumerator.m
		src/ISDConflictAddEnumerator.m
		src/ISDConflictManager.m
		src/ISDRecordIdMap.m
		src/ISDRecordIdMapper.m
		src/ISDDataObject.m
		src/ISDSyncState.m
		src/ISDClientState.m
		src/ISDFileReference.m
		src/ISDClient.m
		src/ISDEntity.m
		src/ISDProperty.m
		src/ISDRelationship.m
		src/ISDDataClass.m
		src/ISyncPlan.m
		src/ISDCallHistory.m
		src/ISDPropertyConflict.m
		src/ISyncFileLocker.m
		src/ISDObjectGraphWrapper.m
		src/ISDQuickDirtyCoder.m
		src/ISyncSessionDriver.m
		src/ISyncConcreteSessionDriver.m
		src/ISyncConcreteRecordReference.m
		src/ISyncRecordReference.m
		src/ISDRecordIdMapDatabase.m
		src/ISDPreNameNumberMapIdMapDatabase.m
		src/ISDSqliteRecordIdMapper.m
		src/IDMapping.m
		src/ISDIdMapDatabase.m
		src/ISyncRecordView.m
		src/ISyncConcreteRecordView.m
		src/ISyncRecordViewWithConflicts.m
		src/ISyncRecordGraphNode.m
		src/ISyncConcreteRecordGraphNode.m
		src/ISyncRecordWithConflictGraphNode.m
		src/ISDDataWrapper.m
		src/ISDDataReference.m
		src/ISDDigestMaker.m
		src/ISDPersistentStoreSyncer.m
		src/ISDFastCDSyncHelper.m
		src/ISDSqliteChangeStoreDatabase.m
		src/ISDSqliteChangeStore.m
		src/PresetNameNumberProvider.m
		src/ISDNameNumberProvider.m
		src/DefaultNameNumberProvider.m
		src/ISDNameNumberMapping.m
		src/ISDDataDirectoryVersion.m
		src/ISDGlobalId.m

	DEPENDENCIES
		system
		objc
		Foundation
)

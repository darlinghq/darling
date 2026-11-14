// Copyright (C) 2026 Lubos Dolezel
//
// Typecheck of what the Swift CloudKit overlay (operation conveniences, subscription and operation ID
// aliases, zone change configuration, container application permissions) needs from the CloudKit Clang
// module: the operations with their refined (double underscore) members, CKContainer, CKDatabase and
// the nested names CKFetchRecordZoneChangesOperation.ZoneConfiguration, CKQueryOperation.Cursor,
// CKDatabase.Scope and CKModifyRecordsOperation.RecordSavePolicy.
// Build: swiftc -typecheck cloudkit_operations_import.swift -target x86_64-apple-macosx26.0 -sdk <SDK>

import CloudKit
import Foundation

extension CKContainer {
    enum Application {
        typealias Permissions = CKContainer_Application_Permissions
        typealias PermissionStatus = CKContainer_Application_PermissionStatus
        typealias PermissionBlock = CKContainer_Application_PermissionBlock
    }
    func fetchAllLongLivedOperationIDs(completionHandler: @escaping ([String]?, Error?) -> Void) {
        self.__fetchAllLongLivedOperationIDs(completionHandler: completionHandler)
    }
    func fetchLongLivedOperation(withID operationID: String, completionHandler: @escaping (CKOperation?, Error?) -> Void) {
        self.__fetchLongLivedOperation(withID: operationID, completionHandler: completionHandler)
    }
}
extension CKOperation {
    typealias ID = String
    var operationID: CKOperation.ID { return self.__operationID as CKOperation.ID }
}
extension CKModifySubscriptionsOperation {
    convenience init(subscriptionsToSave: [CKSubscription]? = nil, subscriptionIDsToDelete: [String]? = nil) {
        self.init()
        self.subscriptionsToSave = subscriptionsToSave
        self.__subscriptionIDsToDelete = subscriptionIDsToDelete
    }
}
extension CKFetchSubscriptionsOperation {
    convenience init(subscriptionIDs: [String]) { self.init(__subscriptionIDs: subscriptionIDs) }
    var subscriptionIDs: [String]? {
        get { return self.__subscriptionIDs }
        set { self.__subscriptionIDs = newValue }
    }
}
extension CKDatabase {
    func fetch(withSubscriptionID subscriptionID: String, completionHandler: @escaping (CKSubscription?, Error?) -> Void) {
        self.__fetch(withSubscriptionID: subscriptionID, completionHandler: completionHandler)
    }
    func delete(withSubscriptionID subscriptionID: String, completionHandler: @escaping (String?, Error?) -> Void) {
        self.__delete(withSubscriptionID: subscriptionID, completionHandler: completionHandler)
    }
}
extension CKFetchRecordZoneChangesOperation {
    var recordWithIDWasDeletedBlock: ((CKRecord.ID, String) -> Void)? {
        get { return self.__recordWithIDWasDeletedBlock }
        set { self.__recordWithIDWasDeletedBlock = newValue }
    }
    convenience init(recordZoneIDs: [CKRecordZone.ID]? = nil, configurationsByRecordZoneID: [CKRecordZone.ID: ZoneConfiguration]? = nil) {
        self.init()
        self.recordZoneIDs = recordZoneIDs
        self.configurationsByRecordZoneID = configurationsByRecordZoneID
    }
}
extension CKFetchRecordZoneChangesOperation.ZoneConfiguration {
    var desiredKeys: [String]? {
        get { return self.__desiredKeys }
        set { self.__desiredKeys = newValue }
    }
    convenience init(previousServerChangeToken: CKServerChangeToken? = nil, resultsLimit: Int? = nil, desiredKeys: [String]? = nil) {
        self.init()
        if let previousServerChangeToken = previousServerChangeToken { self.previousServerChangeToken = previousServerChangeToken }
        if let resultsLimit = resultsLimit { self.resultsLimit = resultsLimit }
        if let desiredKeys = desiredKeys { self.desiredKeys = desiredKeys }
    }
}
extension CKModifyRecordZonesOperation {
    convenience init(recordZonesToSave: [CKRecordZone]? = nil, recordZoneIDsToDelete: [CKRecordZone.ID]? = nil) {
        self.init()
        self.recordZonesToSave = recordZonesToSave
        self.recordZoneIDsToDelete = recordZoneIDsToDelete
    }
}
extension CKModifyRecordsOperation {
    convenience init(recordsToSave: [CKRecord]? = nil, recordIDsToDelete: [CKRecord.ID]? = nil) {
        self.init()
        self.recordsToSave = recordsToSave
        self.recordIDsToDelete = recordIDsToDelete
    }
}
extension CKFetchShareMetadataOperation {
    var rootRecordDesiredKeys: [String]? {
        get { return self.__rootRecordDesiredKeys }
        set { self.__rootRecordDesiredKeys = newValue }
    }
}
extension CKFetchRecordsOperation {
    var desiredKeys: [String]? {
        get { return self.__desiredKeys }
        set { self.__desiredKeys = newValue }
    }
}
extension CKQueryOperation {
    var desiredKeys: [String]? {
        get { return self.__desiredKeys }
        set { self.__desiredKeys = newValue }
    }
}

// The operations and their completion blocks.
func useOperations(_ container: CKContainer, _ database: CKDatabase, _ record: CKRecord, _ query: CKQuery) {
    let fetch = CKFetchRecordsOperation(recordIDs: [record.recordID])
    fetch.desiredKeys = ["a"]
    fetch.fetchRecordsCompletionBlock = { records, error in _ = (records?[record.recordID], error) }
    fetch.perRecordCompletionBlock = { record, recordID, error in _ = (record, recordID, error) }
    database.add(fetch)

    let modify = CKModifyRecordsOperation(recordsToSave: [record], recordIDsToDelete: nil)
    modify.savePolicy = .changedKeys
    let policy: CKModifyRecordsOperation.RecordSavePolicy = .allKeys
    _ = policy
    modify.isAtomic = true
    modify.modifyRecordsCompletionBlock = { saved, deleted, error in _ = (saved, deleted, error) }

    let queryOp = CKQueryOperation(query: query)
    queryOp.resultsLimit = CKQueryOperation.maximumResults
    queryOp.recordFetchedBlock = { _ in }
    queryOp.queryCompletionBlock = { (cursor: CKQueryOperation.Cursor?, error: Error?) in _ = (cursor, error) }
    let more = CKQueryOperation(cursor: CKQueryOperation.Cursor?.none!)
    _ = more

    let zoneID = CKRecordZone.ID(__zoneName: "Z", ownerName: "o")
    let changes = CKFetchRecordZoneChangesOperation(recordZoneIDs: [zoneID],
        configurationsByRecordZoneID: [zoneID: CKFetchRecordZoneChangesOperation.ZoneConfiguration(resultsLimit: 10, desiredKeys: ["a"])])
    changes.fetchAllChanges = true
    changes.recordChangedBlock = { _ in }
    changes.recordWithIDWasDeletedBlock = { id, type in _ = (id, type) }
    changes.recordZoneFetchCompletionBlock = { zone, token, data, more, error in _ = (zone, token, data, more, error) }
    changes.fetchRecordZoneChangesCompletionBlock = { error in _ = error }

    let databaseChanges = CKFetchDatabaseChangesOperation(previousServerChangeToken: nil)
    databaseChanges.fetchDatabaseChangesCompletionBlock = { token, more, error in _ = (token, more, error) }

    let zones = CKFetchRecordZonesOperation.fetchAllRecordZonesOperation()
    zones.fetchRecordZonesCompletionBlock = { zones, error in _ = (zones, error) }
    _ = CKModifyRecordZonesOperation(recordZonesToSave: [CKRecordZone(zoneName: "Z")])

    let subscription = CKDatabaseSubscription(__subscriptionID: "s")
    let modifySubs = CKModifySubscriptionsOperation(subscriptionsToSave: [subscription], subscriptionIDsToDelete: ["old"])
    modifySubs.__modifySubscriptionsCompletionBlock = { saved, deleted, error in _ = (saved, deleted, error) }
    let fetchSubs = CKFetchSubscriptionsOperation(subscriptionIDs: ["s"])
    fetchSubs.__fetchSubscriptionCompletionBlock = { subs, error in _ = (subs, error) }

    let metadata = CKFetchShareMetadataOperation()
    metadata.shouldFetchRootRecord = true
    metadata.rootRecordDesiredKeys = ["a"]
    metadata.perShareMetadataBlock = { url, metadata, error in _ = (url, metadata, error) }
    let accept = CKAcceptSharesOperation(shareMetadatas: [])
    accept.perShareCompletionBlock = { metadata, share, error in _ = (metadata, share, error) }
    let participants = CKFetchShareParticipantsOperation(userIdentityLookupInfos: [CKUserIdentity.LookupInfo(emailAddress: "a@b.c")])
    participants.shareParticipantFetchedBlock = { _ in }
    let discover = CKDiscoverUserIdentitiesOperation(userIdentityLookupInfos: [])
    discover.userIdentityDiscoveredBlock = { identity, info in _ = (identity, info) }
    _ = CKDiscoverAllUserIdentitiesOperation()

    // Configuration, groups and the operation ID.
    let configuration = CKOperation.Configuration()
    configuration.allowsCellularAccess = false
    configuration.qualityOfService = .utility
    configuration.timeoutIntervalForRequest = 30
    fetch.configuration = configuration
    let group = CKOperationGroup()
    group.name = "sync"
    group.expectedSendSize = .kilobytes
    fetch.group = group
    let id: CKOperation.ID = fetch.operationID
    _ = id

    // The containers and their databases.
    _ = container.publicCloudDatabase
    _ = container.database(with: .shared)
    container.accountStatus { status, error in
        switch status {
        case .available, .noAccount, .restricted, .couldNotDetermine, .temporarilyUnavailable: break
        @unknown default: break
        }
        _ = error
    }
    container.status(forApplicationPermission: .userDiscoverability) { status, error in
        _ = (status, error)
    }
    container.fetchUserRecordID { recordID, error in _ = (recordID, error) }
    container.add(fetch)
    _ = CKContainer.default()
    _ = CKContainer(identifier: "iCloud.com.example.app")
}

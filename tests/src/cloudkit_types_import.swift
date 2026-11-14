// Copyright (C) 2026 Lubos Dolezel
//
// Typecheck of what the Swift CloudKit overlay (record IDs, zone IDs, references, shares, subscriptions,
// record values) needs from the CloudKit Clang module: the nested Swift names of the data types, the
// refined (double underscore) accessors and the record-value protocol.
// Build: swiftc -typecheck cloudkit_types_import.swift -target x86_64-apple-macosx26.0 -sdk <SDK>

import CloudKit
import Foundation

// Typealiases and defaults the overlay adds.
extension CKRecord.Reference {
    typealias Action = CKRecord_Reference_Action
}
extension CKShare.Participant {
    typealias AcceptanceStatus = CKShare_Participant_AcceptanceStatus
    typealias Permission = CKShare_Participant_Permission
    typealias Role = CKShare_Participant_Role
}
extension CKRecordZone.ID {
    convenience init(zoneName: String = __CKRecordZoneDefaultName, ownerName: String = "__defaultOwner__") {
        self.init(__zoneName: zoneName, ownerName: ownerName)
    }
    static let defaultZoneName = __CKRecordZoneDefaultName
}
extension CKRecord.ID {
    convenience init(recordName: String = UUID().uuidString, zoneID: CKRecordZone.ID = CKRecordZone.ID()) {
        self.init(__recordName: recordName, zoneID: zoneID)
    }
}
extension CKRecord {
    typealias RecordType = String
    typealias FieldKey = String
    enum SystemType {
        static let userRecord: CKRecord.RecordType = __CKRecordTypeUserRecord
        static let share: CKRecord.RecordType = __CKRecordTypeShare
    }
    enum SystemFieldKey {
        static let parent: CKRecord.FieldKey = __CKRecordParentKey
        static let share: CKRecord.FieldKey = __CKRecordShareKey
    }
    convenience init(recordType: CKRecord.RecordType, recordID: CKRecord.ID = CKRecord.ID()) {
        self.init(__recordType: recordType, recordID: recordID)
    }
    var recordType: CKRecord.RecordType { return self.__recordType }
    func object(forKey key: CKRecord.FieldKey) -> __CKRecordObjCValue? { return self.__object(forKey: key) }
    func setObject(_ object: __CKRecordObjCValue?, forKey key: CKRecord.FieldKey) { self.__setObject(object, forKey: key) }
    subscript(key: CKRecord.FieldKey) -> __CKRecordObjCValue? {
        get { return self.object(forKey: key) }
        set { self.setObject(newValue, forKey: key) }
    }
    func allKeys() -> [CKRecord.FieldKey] { return self.__allKeys() }
    func changedKeys() -> [CKRecord.FieldKey] { return self.__changedKeys() }
}
extension CKShare {
    enum SystemFieldKey {
        static let title: CKRecord.FieldKey = __CKShareTitleKey
        static let thumbnailImageData: CKRecord.FieldKey = __CKShareThumbnailImageDataKey
        static let shareType: CKRecord.FieldKey = __CKShareTypeKey
    }
}
extension CKSubscription {
    typealias ID = String
    var subscriptionID: CKSubscription.ID { return self.__subscriptionID }
}
extension CKQuerySubscription {
    convenience init(recordType: CKRecord.RecordType, predicate: NSPredicate, subscriptionID: CKSubscription.ID = UUID().uuidString, options: CKQuerySubscription.Options = []) {
        self.init(__recordType: recordType, predicate: predicate, subscriptionID: subscriptionID, options: options)
    }
    var recordType: CKRecord.RecordType? { return self.__recordType }
}
extension CKRecordZoneSubscription {
    convenience init(zoneID: CKRecordZone.ID, subscriptionID: CKSubscription.ID) {
        self.init(__zoneID: zoneID, subscriptionID: subscriptionID)
    }
    var recordType: CKRecord.RecordType? {
        get { return self.__recordType }
        set { self.__recordType = newValue }
    }
}
extension CKDatabaseSubscription {
    convenience init(subscriptionID: CKSubscription.ID) { self.init(__subscriptionID: subscriptionID) }
}
extension CKQuery {
    convenience init(recordType: CKRecord.RecordType, predicate: NSPredicate) {
        self.init(__recordType: recordType, predicate: predicate)
    }
    var recordType: CKRecord.RecordType { return self.__recordType }
}
extension CKSubscription.NotificationInfo {
    var alertLocalizationArgs: [CKRecord.FieldKey]? {
        get { return self.__alertLocalizationArgs }
        set { self.__alertLocalizationArgs = newValue }
    }
    var titleLocalizationArgs: [CKRecord.FieldKey]? {
        get { return self.__titleLocalizationArgs }
        set { self.__titleLocalizationArgs = newValue }
    }
    var subtitleLocalizationArgs: [CKRecord.FieldKey]? {
        get { return self.__subtitleLocalizationArgs }
        set { self.__subtitleLocalizationArgs = newValue }
    }
    var desiredKeys: [CKRecord.FieldKey]? {
        get { return self.__desiredKeys }
        set { self.__desiredKeys = newValue }
    }
}
extension CKNotification {
    var subscriptionID: CKSubscription.ID? { return self.__subscriptionID }
}

// Record values: the Objective-C protocol is exposed as __CKRecordObjCValue.
func setValues(_ record: CKRecord) {
    record["name"] = "text" as NSString
    record["count"] = NSNumber(value: 3)
    record["when"] = Date() as NSDate
    record["blob"] = Data([1, 2]) as NSData
    record["list"] = ["a", "b"] as NSArray
    record["ref"] = CKRecord.Reference(recordID: record.recordID, action: .deleteSelf)
    record["file"] = CKAsset(fileURL: URL(fileURLWithPath: "/tmp/x"))
    let keys: [CKRecord.FieldKey] = record.allKeys() + record.changedKeys()
    _ = keys
}

// The nested names.
func useNames() {
    let zoneID = CKRecordZone.ID(zoneName: "Z")
    let recordID = CKRecord.ID(recordName: "r", zoneID: zoneID)
    let record = CKRecord(recordType: "Foo", recordID: recordID)
    _ = record.recordType
    let zone = CKRecordZone(zoneName: "Z")
    let caps: CKRecordZone.Capabilities = [.fetchChanges, .atomic, .sharing, .zoneWideSharing]
    _ = (zone.zoneID, caps.contains(.atomic))
    let reference: CKRecord.Reference = CKRecord.Reference(record: record, action: .none)
    let action: CKRecord.Reference.Action = reference.action
    _ = action
    let share = CKShare(rootRecord: record)
    let owner: CKShare.Participant = share.owner
    let status: CKShare.Participant.AcceptanceStatus = owner.acceptanceStatus
    let permission: CKShare.Participant.Permission = owner.permission
    let role: CKShare.Participant.Role = owner.role
    _ = (status, permission, role)
    share.publicPermission = .readOnly
    let metadata: CKShare.Metadata? = nil
    _ = metadata?.share
    let info = CKSubscription.NotificationInfo()
    info.desiredKeys = ["a"]
    info.shouldBadge = true
    let sub = CKQuerySubscription(recordType: "Foo", predicate: NSPredicate(value: true), options: [.firesOnRecordCreation, .firesOnRecordUpdate])
    sub.notificationInfo = info
    _ = (sub.subscriptionID, sub.recordType)
    switch sub.subscriptionType {
    case .query, .recordZone, .database: break
    @unknown default: break
    }
    let zoneSub = CKRecordZoneSubscription(zoneID: zoneID, subscriptionID: "s")
    zoneSub.recordType = "Foo"
    _ = CKDatabaseSubscription(subscriptionID: "d")
    let scope: CKDatabase.Scope = .private
    _ = scope
    let type: CKNotification.NotificationType = .query
    let reason: CKQueryNotification.Reason = .recordCreated
    _ = (type, reason)
    let lookup = CKUserIdentity.LookupInfo(emailAddress: "a@b.c")
    _ = (lookup.emailAddress, CKUserIdentity.LookupInfo.lookupInfos(withEmails: ["a@b.c"]))
    let query = CKQuery(recordType: "Foo", predicate: NSPredicate(value: true))
    query.sortDescriptors = [NSSortDescriptor(key: "a", ascending: true)]
    let cursor: CKQueryOperation.Cursor? = nil
    _ = (query.recordType, cursor)
    let token: CKServerChangeToken? = nil
    _ = token
}

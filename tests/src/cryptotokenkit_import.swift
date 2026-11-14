// Copyright (C) 2026 Lubos Dolezel
//
// Typecheck of what the Swift CryptoTokenKit overlay (apple/swift 5.2.2, stdlib/public/Darwin/CryptoTokenKit/TKSmartCard.swift)
// needs from the CryptoTokenKit Clang module: the Swift-refined __sendIns (two forms) and __inSession on TKSmartCard.
// Build: swiftc -typecheck cryptotokenkit_import.swift -target x86_64-apple-macosx26.0 -sdk <SDK>

import CryptoTokenKit
import Foundation

// The overlay's extension, spelled out.
extension TKSmartCard {
    func sendTyped(ins: UInt8, p1: UInt8, p2: UInt8, data: Data? = nil, le: Int? = nil,
                   reply: @escaping (Data?, UInt16, Error?) -> Void) {
        self.__sendIns(ins, p1: p1, p2: p2, data: data, le: le.map { NSNumber(value: $0) }, reply: reply)
    }

    func sendTyped(ins: UInt8, p1: UInt8, p2: UInt8, data: Data? = nil, le: Int? = nil) throws -> (sw: UInt16, response: Data) {
        var sw: UInt16 = 0
        let response = try self.__sendIns(ins, p1: p1, p2: p2, data: data, le: le.map { NSNumber(value: $0) }, sw: &sw)
        return (sw: sw, response: response)
    }

    func withSessionTyped<T>(_ body: @escaping () throws -> T) throws -> T {
        var result: T?
        try self.__inSession(executeBlock: { (errorPointer: NSErrorPointer) -> Bool in
            do {
                result = try body()
                return true
            } catch let error as NSError {
                errorPointer?.pointee = error
                return false
            }
        })
        return result!
    }
}

func use(_ slot: TKSmartCardSlot, _ card: TKSmartCard, _ record: TKTLVRecord) {
    let protocols: TKSmartCardProtocol = [.t0, .t1, .t15, .any]
    let none: TKSmartCardProtocol = []
    let state: TKSmartCardSlot.State = .missing
    _ = [TKSmartCardSlot.State.empty, .probing, .muteCard, .validCard]
    let atr: TKSmartCardATR? = slot.atr
    card.allowedProtocols = protocols
    let group = atr?.interfaceGroup(at: 1)
    let byProtocol = atr?.interfaceGroup(for: .t1)
    _ = (none, state, atr?.bytes, group?.ta, group?.tb, group?.tc, group?.protocol, byProtocol)
    _ = (record.tag, record.value, TKTokenOperation.signData)
}

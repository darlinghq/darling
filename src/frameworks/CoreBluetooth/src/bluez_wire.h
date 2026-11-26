/*
 * This file is part of Darling.
 *
 * Copyright (C) 2026 Darling contributors
 *
 * Darling is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Darling is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 */

#ifndef CB_BLUEZ_WIRE_H
#define CB_BLUEZ_WIRE_H

#include <dbus/dbus.h>

#include <cstdint>
#include <map>
#include <string>
#include <vector>

struct darling_dbus;

namespace cbbluez
{
struct Value
{
    enum class Type
    {
        Empty,
        String,
        UInt16,
        Int16,
        UInt32,
        Int32,
        Boolean,
        Bytes,
        Strings,
        ByteDict,
        StringBytesDict
    };

    Type type = Type::Empty;
    std::string stringValue;
    uint16_t uint16Value = 0;
    int16_t int16Value = 0;
    uint32_t uint32Value = 0;
    int32_t int32Value = 0;
    bool boolValue = false;
    std::vector<uint8_t> bytes;
    std::vector<std::string> strings;
    std::map<std::string, std::vector<uint8_t>> byteDict;
    std::map<std::string, std::vector<uint8_t>> stringBytesDict;
};

struct DeviceInfo
{
    std::string path;
    std::string address;
    std::string addressType;
    std::string alias;
    bool hasRSSI = false;
    int16_t RSSI = 0;
    bool hasTxPower = false;
    int16_t txPower = 0;
    bool hasConnectable = false;
    bool connectable = false;
    std::vector<std::string> UUIDs;
    std::map<std::string, std::vector<uint8_t>> manufacturerData;
    std::map<std::string, std::vector<uint8_t>> serviceData;
};

struct ServiceInfo
{
    std::string path;
    std::string UUID;
    uint16_t handle = 0;
    bool primary = true;
    std::vector<std::string> includes;
};

struct CharacteristicInfo
{
    std::string path;
    std::string servicePath;
    std::string UUID;
    uint16_t handle = 0;
    uint32_t properties = 0;
    bool notifying = false;
    std::vector<uint8_t> value;
};

struct DescriptorInfo
{
    std::string path;
    std::string characteristicPath;
    std::string UUID;
    uint16_t handle = 0;
    std::vector<uint8_t> value;
};

struct ObjectRecord
{
    std::string path;
    std::map<std::string, std::map<std::string, Value>> interfaces;
};

// All D-Bus operations are made through the shared wrapper function table.
bool ReadValue(const struct darling_dbus *api, DBusMessageIter *iter, Value *out);
bool ReadProperties(const struct darling_dbus *api, DBusMessageIter *iter,
                    std::map<std::string, Value> *out);
bool ReadManagedObjects(const struct darling_dbus *api, DBusMessageIter *iter,
                        std::vector<ObjectRecord> *out);

bool DeviceFromRecord(const ObjectRecord &record, DeviceInfo *out);
bool ServiceFromRecord(const ObjectRecord &record, ServiceInfo *out);
bool CharacteristicFromRecord(const ObjectRecord &record, CharacteristicInfo *out);
bool DescriptorFromRecord(const ObjectRecord &record, DescriptorInfo *out);

// Converts a 16-bit or 32-bit Bluetooth UUID (or a full UUID) to its
// canonical upper-case 128-bit representation. Returns an empty string when
// the input is not a UUID.
std::string NormalizeUUID(const std::string &input);

// A stable CBPeripheral identifier derived from the BlueZ address. The
// address type occupies a separate UUID group so public and random addresses
// with equal 48-bit values do not collide.
std::string PeripheralIdentifier(const std::string &address, const std::string &addressType);

// Maps the BlueZ GATT characteristic flags to the public CBCharacteristic
// properties bit field.
uint32_t CharacteristicProperties(const std::vector<std::string> &flags);

bool PathHasPrefix(const std::string &path, const std::string &prefix);

// Maps a BlueZ D-Bus error name to the CBError value used by CoreBluetooth.
int ErrorCodeForDBusName(const std::string &name);
} // namespace cbbluez

#endif

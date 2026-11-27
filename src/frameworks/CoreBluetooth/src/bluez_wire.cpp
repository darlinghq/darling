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

#include "bluez_wire.h"

#include "darling_dbus.h"

#include <algorithm>
#include <cctype>

namespace cbbluez
{
namespace
{
bool ReadBasic(const struct darling_dbus *api, DBusMessageIter *iter, Value *out)
{
    const int type = api->message_iter_get_arg_type(iter);
    switch (type)
    {
    case DBUS_TYPE_STRING:
    {
        const char *text = nullptr;
        api->message_iter_get_basic(iter, &text);
        out->type = Value::Type::String;
        out->stringValue = text != nullptr ? text : "";
        return true;
    }
    case DBUS_TYPE_OBJECT_PATH:
    {
        // Device1.Adapter, GattService1.Device, GattCharacteristic1.Service and
        // GattDescriptor1.Characteristic are object paths; they read like strings.
        const char *text = nullptr;
        api->message_iter_get_basic(iter, &text);
        out->type = Value::Type::String;
        out->stringValue = text != nullptr ? text : "";
        return true;
    }
    case DBUS_TYPE_UINT16:
        api->message_iter_get_basic(iter, &out->uint16Value);
        out->type = Value::Type::UInt16;
        return true;
    case DBUS_TYPE_INT16:
        api->message_iter_get_basic(iter, &out->int16Value);
        out->type = Value::Type::Int16;
        return true;
    case DBUS_TYPE_UINT32:
        api->message_iter_get_basic(iter, &out->uint32Value);
        out->type = Value::Type::UInt32;
        return true;
    case DBUS_TYPE_INT32:
        api->message_iter_get_basic(iter, &out->int32Value);
        out->type = Value::Type::Int32;
        return true;
    case DBUS_TYPE_BOOLEAN:
    {
        dbus_bool_t value = false;
        api->message_iter_get_basic(iter, &value);
        out->type = Value::Type::Boolean;
        out->boolValue = value != false;
        return true;
    }
    case DBUS_TYPE_BYTE:
    {
        unsigned char value = 0;
        api->message_iter_get_basic(iter, &value);
        out->type = Value::Type::UInt16;
        out->uint16Value = value;
        return true;
    }
    default:
        return false;
    }
}

bool ReadStringKey(const struct darling_dbus *api, DBusMessageIter *iter, std::string *out)
{
    const int type = api->message_iter_get_arg_type(iter);
    if (type == DBUS_TYPE_STRING)
    {
        const char *text = nullptr;
        api->message_iter_get_basic(iter, &text);
        *out = text != nullptr ? text : "";
        return true;
    }
    if (type == DBUS_TYPE_UINT16)
    {
        uint16_t value = 0;
        api->message_iter_get_basic(iter, &value);
        *out = std::to_string(value);
        return true;
    }
    if (type == DBUS_TYPE_BYTE)
    {
        unsigned char value = 0;
        api->message_iter_get_basic(iter, &value);
        *out = std::to_string(value);
        return true;
    }
    return false;
}

bool IsHex(char c)
{
    return (c >= '0' && c <= '9') || (c >= 'a' && c <= 'f') || (c >= 'A' && c <= 'F');
}
} // namespace

bool ReadValue(const struct darling_dbus *api, DBusMessageIter *iter, Value *out)
{
    if (api == nullptr || iter == nullptr || out == nullptr)
        return false;

    *out = Value();
    if (api->message_iter_get_arg_type(iter) == DBUS_TYPE_VARIANT)
    {
        DBusMessageIter inner;
        api->message_iter_recurse(iter, &inner);
        return ReadValue(api, &inner, out);
    }

    const int type = api->message_iter_get_arg_type(iter);
    if (type == DBUS_TYPE_ARRAY)
    {
        DBusMessageIter items;
        api->message_iter_recurse(iter, &items);
        const int itemType = api->message_iter_get_arg_type(&items);
        if (itemType == DBUS_TYPE_BYTE)
        {
            out->type = Value::Type::Bytes;
            while (api->message_iter_get_arg_type(&items) == DBUS_TYPE_BYTE)
            {
                unsigned char byte = 0;
                api->message_iter_get_basic(&items, &byte);
                out->bytes.push_back(byte);
                api->message_iter_next(&items);
            }
            return true;
        }
        if (itemType == DBUS_TYPE_STRING || itemType == DBUS_TYPE_OBJECT_PATH)
        {
            out->type = Value::Type::Strings;
            while (api->message_iter_get_arg_type(&items) == itemType)
            {
                const char *text = nullptr;
                api->message_iter_get_basic(&items, &text);
                out->strings.emplace_back(text != nullptr ? text : "");
                api->message_iter_next(&items);
            }
            return true;
        }
        if (itemType == DBUS_TYPE_DICT_ENTRY)
        {
            out->type = Value::Type::StringBytesDict;
            while (api->message_iter_get_arg_type(&items) == DBUS_TYPE_DICT_ENTRY)
            {
                DBusMessageIter entry;
                api->message_iter_recurse(&items, &entry);
                std::string key;
                Value value;
                if (!ReadStringKey(api, &entry, &key))
                    return false;
                api->message_iter_next(&entry);
                if (!ReadValue(api, &entry, &value) || value.type != Value::Type::Bytes)
                    return false;
                out->stringBytesDict[key] = value.bytes;
                api->message_iter_next(&items);
            }
            return true;
        }
        return false;
    }
    return ReadBasic(api, iter, out);
}

bool ReadProperties(const struct darling_dbus *api, DBusMessageIter *iter,
                    std::map<std::string, Value> *out)
{
    if (api == nullptr || iter == nullptr || out == nullptr ||
        api->message_iter_get_arg_type(iter) != DBUS_TYPE_ARRAY)
        return false;

    DBusMessageIter entries;
    api->message_iter_recurse(iter, &entries);
    while (api->message_iter_get_arg_type(&entries) == DBUS_TYPE_DICT_ENTRY)
    {
        DBusMessageIter entry;
        api->message_iter_recurse(&entries, &entry);
        const char *name = nullptr;
        if (api->message_iter_get_arg_type(&entry) != DBUS_TYPE_STRING)
            return false;
        api->message_iter_get_basic(&entry, &name);
        api->message_iter_next(&entry);

        // BlueZ publishes properties of types that are not modelled here (for
        // example Device1.Sets, a dictionary of dictionaries). Leaving one out
        // must not make the whole object, or the whole reply, unreadable.
        Value value;
        if (ReadValue(api, &entry, &value))
            (*out)[name != nullptr ? name : ""] = value;
        api->message_iter_next(&entries);
    }
    return true;
}

bool ReadManagedObjects(const struct darling_dbus *api, DBusMessageIter *iter,
                        std::vector<ObjectRecord> *out)
{
    if (api == nullptr || iter == nullptr || out == nullptr ||
        api->message_iter_get_arg_type(iter) != DBUS_TYPE_ARRAY)
        return false;

    DBusMessageIter entries;
    api->message_iter_recurse(iter, &entries);
    while (api->message_iter_get_arg_type(&entries) == DBUS_TYPE_DICT_ENTRY)
    {
        DBusMessageIter entry;
        api->message_iter_recurse(&entries, &entry);
        const char *path = nullptr;
        if (api->message_iter_get_arg_type(&entry) != DBUS_TYPE_OBJECT_PATH)
            return false;
        api->message_iter_get_basic(&entry, &path);
        api->message_iter_next(&entry);

        if (api->message_iter_get_arg_type(&entry) != DBUS_TYPE_ARRAY)
            return false;
        DBusMessageIter interfacesIter;
        api->message_iter_recurse(&entry, &interfacesIter);
        ObjectRecord record;
        record.path = path != nullptr ? path : "";
        while (api->message_iter_get_arg_type(&interfacesIter) == DBUS_TYPE_DICT_ENTRY)
        {
            DBusMessageIter interfaceEntry;
            api->message_iter_recurse(&interfacesIter, &interfaceEntry);
            const char *interface = nullptr;
            if (api->message_iter_get_arg_type(&interfaceEntry) != DBUS_TYPE_STRING)
                return false;
            api->message_iter_get_basic(&interfaceEntry, &interface);
            api->message_iter_next(&interfaceEntry);

            std::map<std::string, Value> properties;
            if (!ReadProperties(api, &interfaceEntry, &properties))
                return false;
            if (interface != nullptr)
                record.interfaces[interface] = properties;
            api->message_iter_next(&interfacesIter);
        }
        out->push_back(record);
        api->message_iter_next(&entries);
    }
    return true;
}

bool DeviceFromRecord(const ObjectRecord &record, DeviceInfo *out)
{
    if (out == nullptr)
        return false;
    const auto it = record.interfaces.find("org.bluez.Device1");
    if (it == record.interfaces.end())
        return false;

    DeviceInfo info;
    info.path = record.path;
    for (const auto &property : it->second)
    {
        const std::string &name = property.first;
        const Value &value = property.second;
        if (name == "Address" && value.type == Value::Type::String)
            info.address = value.stringValue;
        else if (name == "AddressType" && value.type == Value::Type::String)
            info.addressType = value.stringValue;
        else if ((name == "Alias" || name == "Name") && value.type == Value::Type::String)
        {
            if (!info.alias.empty() && name == "Name")
                continue;
            info.alias = value.stringValue;
        }
        else if (name == "RSSI" && value.type == Value::Type::Int16)
        {
            info.RSSI = value.int16Value;
            info.hasRSSI = true;
        }
        else if (name == "TxPower" && value.type == Value::Type::Int16)
        {
            info.txPower = value.int16Value;
            info.hasTxPower = true;
        }
        else if (name == "Connectable" && value.type == Value::Type::Boolean)
        {
            info.connectable = value.boolValue;
            info.hasConnectable = true;
        }
        else if (name == "UUIDs" && value.type == Value::Type::Strings)
        {
            for (const std::string &uuid : value.strings)
            {
                const std::string normalized = NormalizeUUID(uuid);
                if (!normalized.empty())
                    info.UUIDs.push_back(normalized);
            }
        }
        else if (name == "ManufacturerData" && value.type == Value::Type::StringBytesDict)
            info.manufacturerData = value.stringBytesDict;
        else if (name == "ServiceData" && value.type == Value::Type::StringBytesDict)
        {
            // BlueZ uses the short form (for example "180f") as the key;
            // CoreBluetooth exposes CBUUID objects, so canonicalize them.
            for (const auto &entry : value.stringBytesDict)
            {
                const std::string normalized = NormalizeUUID(entry.first);
                info.serviceData[normalized.empty() ? entry.first : normalized] = entry.second;
            }
        }
    }
    if (info.address.empty())
        return false;
    *out = info;
    return true;
}

bool ServiceFromRecord(const ObjectRecord &record, ServiceInfo *out)
{
    if (out == nullptr)
        return false;
    const auto it = record.interfaces.find("org.bluez.GattService1");
    if (it == record.interfaces.end())
        return false;

    ServiceInfo info;
    info.path = record.path;
    for (const auto &property : it->second)
    {
        const std::string &name = property.first;
        const Value &value = property.second;
        if (name == "UUID" && value.type == Value::Type::String)
            info.UUID = NormalizeUUID(value.stringValue);
        else if (name == "Primary" && value.type == Value::Type::Boolean)
            info.primary = value.boolValue;
        else if (name == "Handle" && value.type == Value::Type::UInt16)
            info.handle = value.uint16Value;
        else if (name == "Includes" && value.type == Value::Type::Strings)
            info.includes = value.strings;
    }
    if (info.UUID.empty())
        return false;
    *out = info;
    return true;
}

bool CharacteristicFromRecord(const ObjectRecord &record, CharacteristicInfo *out)
{
    if (out == nullptr)
        return false;
    const auto it = record.interfaces.find("org.bluez.GattCharacteristic1");
    if (it == record.interfaces.end())
        return false;

    CharacteristicInfo info;
    info.path = record.path;
    std::vector<std::string> flags;
    for (const auto &property : it->second)
    {
        const std::string &name = property.first;
        const Value &value = property.second;
        if (name == "UUID" && value.type == Value::Type::String)
            info.UUID = NormalizeUUID(value.stringValue);
        else if (name == "Service" && value.type == Value::Type::String)
            info.servicePath = value.stringValue;
        else if (name == "Handle" && value.type == Value::Type::UInt16)
            info.handle = value.uint16Value;
        else if (name == "Value" && value.type == Value::Type::Bytes)
            info.value = value.bytes;
        else if (name == "Notifying" && value.type == Value::Type::Boolean)
            info.notifying = value.boolValue;
        else if (name == "Flags" && value.type == Value::Type::Strings)
            flags = value.strings;
    }
    if (info.UUID.empty())
        return false;
    info.properties = CharacteristicProperties(flags);
    *out = info;
    return true;
}

bool DescriptorFromRecord(const ObjectRecord &record, DescriptorInfo *out)
{
    if (out == nullptr)
        return false;
    const auto it = record.interfaces.find("org.bluez.GattDescriptor1");
    if (it == record.interfaces.end())
        return false;

    DescriptorInfo info;
    info.path = record.path;
    for (const auto &property : it->second)
    {
        const std::string &name = property.first;
        const Value &value = property.second;
        if (name == "UUID" && value.type == Value::Type::String)
            info.UUID = NormalizeUUID(value.stringValue);
        else if (name == "Characteristic" && value.type == Value::Type::String)
            info.characteristicPath = value.stringValue;
        else if (name == "Handle" && value.type == Value::Type::UInt16)
            info.handle = value.uint16Value;
        else if (name == "Value" && value.type == Value::Type::Bytes)
            info.value = value.bytes;
    }
    if (info.UUID.empty())
        return false;
    *out = info;
    return true;
}

std::string NormalizeUUID(const std::string &input)
{
    std::string hex;
    for (char c : input)
    {
        if (c == '-')
            continue;
        if (!IsHex(c))
            return std::string();
        hex.push_back(static_cast<char>(std::toupper(static_cast<unsigned char>(c))));
    }
    if (hex.size() == 4 || hex.size() == 8)
    {
        hex.insert(0, 8 - hex.size(), '0');
        return hex + "-0000-1000-8000-00805F9B34FB";
    }
    if (hex.size() != 32)
        return std::string();
    return hex.substr(0, 8) + "-" + hex.substr(8, 4) + "-" + hex.substr(12, 4) + "-" +
           hex.substr(16, 4) + "-" + hex.substr(20);
}

std::string PeripheralIdentifier(const std::string &address, const std::string &addressType)
{
    std::string hex;
    for (char c : address)
    {
        if (c == ':' || c == '-')
            continue;
        if (!IsHex(c))
            return std::string();
        hex.push_back(static_cast<char>(std::toupper(static_cast<unsigned char>(c))));
    }
    if (hex.size() != 12)
        return std::string();
    const std::string variant = addressType == "random" ? "0001" : "0000";
    return "00000000-0000-0000-" + variant + "-" + hex;
}

uint32_t CharacteristicProperties(const std::vector<std::string> &flags)
{
    struct FlagMap
    {
        const char *name;
        uint32_t bit;
    };
    static const FlagMap map[] = {
        {"broadcast", 0x001},
        {"read", 0x002},
        {"write-without-response", 0x004},
        {"write", 0x008},
        {"notify", 0x010},
        {"indicate", 0x020},
        {"authenticated-signed-writes", 0x040},
        {"extended-properties", 0x080},
        {"encrypt-notify", 0x100},
        {"encrypt-indicate", 0x200},
    };

    uint32_t properties = 0;
    for (const std::string &flag : flags)
    {
        for (const FlagMap &entry : map)
        {
            if (flag == entry.name)
            {
                properties |= entry.bit;
                break;
            }
        }
    }
    return properties;
}

bool PathHasPrefix(const std::string &path, const std::string &prefix)
{
    if (prefix.empty())
        return true;
    if (path.size() < prefix.size() || path.compare(0, prefix.size(), prefix) != 0)
        return false;
    return prefix.back() == '/' || (path.size() > prefix.size() && path[prefix.size()] == '/');
}

int ErrorCodeForDBusName(const std::string &name)
{
    if (name == "org.bluez.Error.NotConnected" || name == "org.bluez.Error.DoesNotExist")
        return 2; // CBErrorNotConnected
    if (name == "org.bluez.Error.InvalidArguments" || name == "org.bluez.Error.InvalidValueLength")
        return 1; // CBErrorInvalidParameters
    if (name == "org.bluez.Error.AuthenticationCanceled")
        return 7; // CBErrorConnectionCancelled
    if (name == "org.bluez.Error.AuthenticationRejected")
        return 6; // CBErrorConnectionRejected
    if (name == "org.bluez.Error.AuthenticationTimeout" || name == "org.bluez.Error.Timeout")
        return 4; // CBErrorTimeout
    if (name == "org.bluez.Error.NotPermitted" || name == "org.bluez.Error.NotAuthorized" ||
        name == "org.bluez.Error.NotSupported" || name == "org.bluez.Error.NotReady" ||
        name == "org.bluez.Error.InProgress")
        return 9; // CBErrorOperationNotSupported
    return 5; // CBErrorConnectionFailed
}
} // namespace cbbluez

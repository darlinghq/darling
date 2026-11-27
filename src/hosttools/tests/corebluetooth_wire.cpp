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

// The checks below are asserts: keep them in every build type.
#undef NDEBUG

#include <darling_dbus.h>

#include <cassert>
#include <string>
#include <vector>

using namespace cbbluez;

namespace
{
// Bluetooth Core Specification, Vol 3, Part B: short UUIDs expand against the
// Bluetooth base UUID. Apple documents CBUUID as accepting the same forms.
void checkUUIDCanonicalization()
{
    const std::string base = "0000-1000-8000-00805F9B34FB";
    assert(NormalizeUUID("180f") == "0000180F-" + base);
    assert(NormalizeUUID("180F") == "0000180F-" + base);
    assert(NormalizeUUID("0000180f") == "0000180F-" + base);
    assert(NormalizeUUID("0000180f-" + base) == "0000180F-" + base);
    assert(NormalizeUUID("0000180F00001000800000805F9B34FB") == "0000180F-" + base);
    assert(NormalizeUUID("0000180F-0000-1000-8000-00805F9B34F").empty());
    assert(NormalizeUUID("001800F").empty());
    assert(NormalizeUUID("not-a-uuid").empty());
    assert(NormalizeUUID("").empty());
}

// BlueZ publishes the Core Properties flags by name; CoreBluetooth exposes
// them as the documented CBCharacteristicProperties bit field.
void checkCharacteristicProperties()
{
    assert(CharacteristicProperties({}) == 0);
    assert(CharacteristicProperties({"unknown-flag"}) == 0);
    assert(CharacteristicProperties({"broadcast"}) == 0x001);
    assert(CharacteristicProperties({"read"}) == 0x002);
    assert(CharacteristicProperties({"write-without-response"}) == 0x004);
    assert(CharacteristicProperties({"write"}) == 0x008);
    assert(CharacteristicProperties({"notify"}) == 0x010);
    assert(CharacteristicProperties({"indicate"}) == 0x020);
    assert(CharacteristicProperties({"authenticated-signed-writes"}) == 0x040);
    assert(CharacteristicProperties({"extended-properties"}) == 0x080);
    assert(CharacteristicProperties({"encrypt-notify"}) == 0x100);
    assert(CharacteristicProperties({"encrypt-indicate"}) == 0x200);
    assert(CharacteristicProperties({"read", "write", "notify"}) == 0x01a);
    // Server-only and extended-property flags are not Core Properties bits.
    assert(CharacteristicProperties({"secure-read", "reliable-write", "authorize"}) == 0);
}

void checkPathPrefix()
{
    assert(PathHasPrefix("/org/bluez/hci0/dev_AA_BB/service1", "/org/bluez/hci0/dev_AA_BB"));
    assert(!PathHasPrefix("/org/bluez/hci0/dev_AA_BB2/service1", "/org/bluez/hci0/dev_AA_BB"));
    assert(!PathHasPrefix("/org/bluez/hci0/dev_AA_BB", "/org/bluez/hci0/dev_AA_BB/service1"));
    assert(PathHasPrefix("/any/path", ""));
}

void checkPeripheralIdentifier()
{
    const std::string first = PeripheralIdentifier("AA:BB:CC:DD:EE:FF", "public");
    assert(!first.empty());
    assert(first == PeripheralIdentifier("aa:bb:cc:dd:ee:ff", "public"));
    assert(first != PeripheralIdentifier("AA:BB:CC:DD:EE:FF", "random"));
    assert(first != PeripheralIdentifier("AA:BB:CC:DD:EE:00", "public"));
    assert(PeripheralIdentifier("not-an-address", "public").empty());
}

void checkErrorMapping()
{
    assert(ErrorCodeForDBusName("org.bluez.Error.InvalidArguments") == 1);
    assert(ErrorCodeForDBusName("org.bluez.Error.NotConnected") == 2);
    assert(ErrorCodeForDBusName("org.bluez.Error.Timeout") == 4);
    assert(ErrorCodeForDBusName("org.bluez.Error.Failed") == 5);
    assert(ErrorCodeForDBusName("org.bluez.Error.AuthenticationRejected") == 6);
    assert(ErrorCodeForDBusName("org.bluez.Error.AuthenticationCanceled") == 7);
    assert(ErrorCodeForDBusName("org.bluez.Error.NotSupported") == 9);
}

// A GetManagedObjects reply as BlueZ sends it: a device (with its Adapter object
// path, a property of a type the wire helpers do not model, and ordinary
// properties) and a GATT service (with its Device object path). Reading it must
// succeed, and the object paths must read as strings.
void checkManagedObjectsReply()
{
    const struct darling_dbus *api = darling_dbus_get(DARLING_DBUS_HOST_FALLBACK);
    assert(api != nullptr);

    DBusMessage *reply = dbus_message_new(DBUS_MESSAGE_TYPE_METHOD_RETURN);
    DBusMessageIter args, objects, object, interfaces, interface, properties, property, variant, array, entry;
    dbus_message_iter_init_append(reply, &args);
    dbus_message_iter_open_container(&args, DBUS_TYPE_ARRAY, "{oa{sa{sv}}}", &objects);

    const char *const adapter = "/org/bluez/hci0";
    const char *const devicePath = "/org/bluez/hci0/dev_AA_BB_CC_DD_EE_01";
    const auto openProperty = [&](const char *key, const char *signature) {
        dbus_message_iter_open_container(&properties, DBUS_TYPE_DICT_ENTRY, nullptr, &property);
        dbus_message_iter_append_basic(&property, DBUS_TYPE_STRING, &key);
        dbus_message_iter_open_container(&property, DBUS_TYPE_VARIANT, signature, &variant);
    };
    const auto closeProperty = [&]() {
        dbus_message_iter_close_container(&property, &variant);
        dbus_message_iter_close_container(&properties, &property);
    };

    // the device
    const char *const deviceInterface = "org.bluez.Device1";
    dbus_message_iter_open_container(&objects, DBUS_TYPE_DICT_ENTRY, nullptr, &object);
    dbus_message_iter_append_basic(&object, DBUS_TYPE_OBJECT_PATH, &devicePath);
    dbus_message_iter_open_container(&object, DBUS_TYPE_ARRAY, "{sa{sv}}", &interfaces);
    dbus_message_iter_open_container(&interfaces, DBUS_TYPE_DICT_ENTRY, nullptr, &interface);
    dbus_message_iter_append_basic(&interface, DBUS_TYPE_STRING, &deviceInterface);
    dbus_message_iter_open_container(&interface, DBUS_TYPE_ARRAY, "{sv}", &properties);
    openProperty("Adapter", "o");
    dbus_message_iter_append_basic(&variant, DBUS_TYPE_OBJECT_PATH, &adapter);
    closeProperty();
    openProperty("Sets", "a{oa{sv}}");
    dbus_message_iter_open_container(&variant, DBUS_TYPE_ARRAY, "{oa{sv}}", &array);
    dbus_message_iter_open_container(&array, DBUS_TYPE_DICT_ENTRY, nullptr, &entry);
    dbus_message_iter_append_basic(&entry, DBUS_TYPE_OBJECT_PATH, &adapter);
    DBusMessageIter inner;
    dbus_message_iter_open_container(&entry, DBUS_TYPE_ARRAY, "{sv}", &inner);
    dbus_message_iter_close_container(&entry, &inner);
    dbus_message_iter_close_container(&array, &entry);
    dbus_message_iter_close_container(&variant, &array);
    closeProperty();
    openProperty("Address", "s");
    const char *const address = "AA:BB:CC:DD:EE:01";
    dbus_message_iter_append_basic(&variant, DBUS_TYPE_STRING, &address);
    closeProperty();
    openProperty("RSSI", "n");
    const dbus_int16_t rssi = -52;
    dbus_message_iter_append_basic(&variant, DBUS_TYPE_INT16, &rssi);
    closeProperty();
    dbus_message_iter_close_container(&interface, &properties);
    dbus_message_iter_close_container(&interfaces, &interface);
    dbus_message_iter_close_container(&object, &interfaces);
    dbus_message_iter_close_container(&objects, &object);

    // a GATT service of the device
    const char *const servicePath = "/org/bluez/hci0/dev_AA_BB_CC_DD_EE_01/service0010";
    const char *const serviceInterface = "org.bluez.GattService1";
    dbus_message_iter_open_container(&objects, DBUS_TYPE_DICT_ENTRY, nullptr, &object);
    dbus_message_iter_append_basic(&object, DBUS_TYPE_OBJECT_PATH, &servicePath);
    dbus_message_iter_open_container(&object, DBUS_TYPE_ARRAY, "{sa{sv}}", &interfaces);
    dbus_message_iter_open_container(&interfaces, DBUS_TYPE_DICT_ENTRY, nullptr, &interface);
    dbus_message_iter_append_basic(&interface, DBUS_TYPE_STRING, &serviceInterface);
    dbus_message_iter_open_container(&interface, DBUS_TYPE_ARRAY, "{sv}", &properties);
    openProperty("Device", "o");
    dbus_message_iter_append_basic(&variant, DBUS_TYPE_OBJECT_PATH, &devicePath);
    closeProperty();
    dbus_message_iter_close_container(&interface, &properties);
    dbus_message_iter_close_container(&interfaces, &interface);
    dbus_message_iter_close_container(&object, &interfaces);
    dbus_message_iter_close_container(&objects, &object);
    dbus_message_iter_close_container(&args, &objects);

    DBusMessageIter read;
    assert(dbus_message_iter_init(reply, &read));
    std::vector<ObjectRecord> records;
    assert(ReadManagedObjects(api, &read, &records));
    assert(records.size() == 2);
    const auto &device = records[0].interfaces.at("org.bluez.Device1");
    assert(device.at("Adapter").type == Value::Type::String);
    assert(device.at("Adapter").stringValue == adapter);
    assert(device.at("Address").stringValue == address);
    assert(device.at("RSSI").int16Value == -52);
    assert(device.count("Sets") == 0);
    const auto &service = records[1].interfaces.at("org.bluez.GattService1");
    assert(service.at("Device").stringValue == devicePath);
    dbus_message_unref(reply);
}
} // namespace

int main()
{
    checkUUIDCanonicalization();
    checkCharacteristicProperties();
    checkPathPrefix();
    checkPeripheralIdentifier();
    checkErrorMapping();
    checkManagedObjectsReply();
    return 0;
}

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
} // namespace

int main()
{
    checkUUIDCanonicalization();
    checkCharacteristicProperties();
    checkPathPrefix();
    checkPeripheralIdentifier();
    checkErrorMapping();
    return 0;
}

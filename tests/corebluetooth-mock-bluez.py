#!/usr/bin/env python3
# Copyright (C) 2026 Lubos Dolezel
#
# A mock of the org.bluez D-Bus service (Adapter1, Device1, GattService1,
# GattCharacteristic1, GattDescriptor1 and the ObjectManager root) for the
# CoreBluetooth central-role test. It owns org.bluez on the bus given by
# --address and appends one line per received call to --log.
#
#   mock-bluez.py --address ADDRESS --log FILE [--adapter off|unpowered|on]
#
# The fake world: one adapter hci0; a heart-rate style peripheral AA:BB:CC:DD:EE:01
# (appears when discovery starts) with one service, a characteristic that can be
# read, written, written without response and notified, and one descriptor; and
# a second peripheral AA:BB:CC:DD:EE:02 whose Connect always fails.
import argparse
import sys

import dbus
import dbus.mainloop.glib
import dbus.service
from gi.repository import GLib

parser = argparse.ArgumentParser()
parser.add_argument("--address", required=True)
parser.add_argument("--log", required=True)
parser.add_argument("--adapter", default="on", choices=["on", "unpowered", "off"])
args = parser.parse_args()

dbus.mainloop.glib.DBusGMainLoop(set_as_default=True)
bus = dbus.bus.BusConnection(args.address)
logf = open(args.log, "a", buffering=1)


def log(line):
    logf.write(line + "\n")


ADAPTER = "/org/bluez/hci0"
DEV1 = ADAPTER + "/dev_AA_BB_CC_DD_EE_01"
DEV2 = ADAPTER + "/dev_AA_BB_CC_DD_EE_02"
SVC = DEV1 + "/service0010"
CHR = SVC + "/char0011"
DSC = CHR + "/desc0013"
BASE = "-0000-1000-8000-00805f9b34fb"


def ay(data):
    return dbus.Array([dbus.Byte(b) for b in data], signature="y")


class Node(dbus.service.Object):
    def __init__(self, path, ifaces):
        super().__init__(bus, path)
        self.ifaces = ifaces  # interface -> {property: value}

    def snapshot(self):
        return {i: dict(p) for i, p in self.ifaces.items()}

    @dbus.service.method("org.freedesktop.DBus.Properties", in_signature="ss", out_signature="v")
    def Get(self, iface, name):
        try:
            return self.ifaces[iface][name]
        except KeyError:
            raise dbus.exceptions.DBusException("no such property", name="org.freedesktop.DBus.Error.InvalidArgs")

    @dbus.service.method("org.freedesktop.DBus.Properties", in_signature="s", out_signature="a{sv}")
    def GetAll(self, iface):
        return dict(self.ifaces.get(iface, {}))

    @dbus.service.signal("org.freedesktop.DBus.Properties", signature="sa{sv}as")
    def PropertiesChanged(self, iface, changed, invalidated):
        pass

    def set(self, iface, name, value):
        self.ifaces[iface][name] = value
        self.PropertiesChanged(iface, {name: value}, [])


class Root(dbus.service.Object):
    def __init__(self):
        super().__init__(bus, "/")
        self.nodes = {}

    @dbus.service.method("org.freedesktop.DBus.ObjectManager", out_signature="a{oa{sa{sv}}}")
    def GetManagedObjects(self):
        return {dbus.ObjectPath(p): n.snapshot() for p, n in self.nodes.items()}

    @dbus.service.signal("org.freedesktop.DBus.ObjectManager", signature="oa{sa{sv}}")
    def InterfacesAdded(self, path, ifaces):
        pass

    def add(self, node, announce=True):
        self.nodes[str(node.__dbus_object_path__)] = node
        if announce:
            self.InterfacesAdded(dbus.ObjectPath(node.__dbus_object_path__), node.snapshot())


root = Root()


class Adapter(Node):
    def __init__(self):
        super().__init__(ADAPTER, {"org.bluez.Adapter1": {
            "Address": "00:11:22:33:44:55", "Powered": dbus.Boolean(args.adapter == "on"),
            "Discovering": dbus.Boolean(False)}})

    @dbus.service.method("org.bluez.Adapter1", in_signature="a{sv}")
    def SetDiscoveryFilter(self, f):
        uuids = sorted(str(u) for u in f.get("UUIDs", []))
        log("SetDiscoveryFilter uuids=%s transport=%s duplicate=%s" % (
            ",".join(uuids), f.get("Transport", ""), bool(f.get("DuplicateData", False))))

    @dbus.service.method("org.bluez.Adapter1")
    def StartDiscovery(self):
        log("StartDiscovery")
        self.set("org.bluez.Adapter1", "Discovering", dbus.Boolean(True))
        GLib.timeout_add(150, self.reveal)

    @dbus.service.method("org.bluez.Adapter1")
    def StopDiscovery(self):
        log("StopDiscovery")
        self.set("org.bluez.Adapter1", "Discovering", dbus.Boolean(False))

    def reveal(self):
        for d in (dev1, dev2):
            if str(d.__dbus_object_path__) not in root.nodes:
                root.add(d)
        return False


class Device(Node):
    def __init__(self, path, address, name, rssi, uuids, fail, extra=None):
        props = {"Address": address, "AddressType": "public", "Name": name, "Alias": name,
                 "RSSI": dbus.Int16(rssi), "UUIDs": dbus.Array(uuids, signature="s"),
                 "Connected": dbus.Boolean(False), "ServicesResolved": dbus.Boolean(False),
                 "Paired": dbus.Boolean(False), "Adapter": dbus.ObjectPath(ADAPTER)}
        props.update(extra or {})
        super().__init__(path, {"org.bluez.Device1": props})
        self.fail = fail

    @dbus.service.method("org.bluez.Device1")
    def Connect(self):
        log("Connect %s" % self.__dbus_object_path__)
        if self.fail:
            raise dbus.exceptions.DBusException("connection refused", name="org.bluez.Error.Failed")
        self.set("org.bluez.Device1", "Connected", dbus.Boolean(True))
        GLib.timeout_add(100, lambda: self.set("org.bluez.Device1", "ServicesResolved", dbus.Boolean(True)) and False)

    @dbus.service.method("org.bluez.Device1")
    def Disconnect(self):
        log("Disconnect %s" % self.__dbus_object_path__)
        self.set("org.bluez.Device1", "ServicesResolved", dbus.Boolean(False))
        self.set("org.bluez.Device1", "Connected", dbus.Boolean(False))


class Characteristic(Node):
    def __init__(self):
        super().__init__(CHR, {"org.bluez.GattCharacteristic1": {
            "UUID": "00002a19" + BASE, "Service": dbus.ObjectPath(SVC),
            "Value": ay(b"\x55"), "Notifying": dbus.Boolean(False),
            "Flags": dbus.Array(["read", "write", "write-without-response", "notify"], signature="s")}})
        self.value = b"\x55"
        self.tick = None

    @dbus.service.method("org.bluez.GattCharacteristic1", in_signature="a{sv}", out_signature="ay")
    def ReadValue(self, opts):
        log("ReadValue %s" % self.__dbus_object_path__)
        return ay(self.value)

    @dbus.service.method("org.bluez.GattCharacteristic1", in_signature="aya{sv}")
    def WriteValue(self, value, opts):
        data = bytes(bytearray(value))
        log("WriteValue %s data=%s type=%s" % (self.__dbus_object_path__, data.hex(), opts.get("type", "")))
        if data == b"\xde\xad":
            raise dbus.exceptions.DBusException("not permitted", name="org.bluez.Error.NotPermitted")
        # BlueZ does not publish a written value as the characteristic's Value
        self.value = data

    @dbus.service.method("org.bluez.GattCharacteristic1")
    def StartNotify(self):
        log("StartNotify %s" % self.__dbus_object_path__)
        self.set("org.bluez.GattCharacteristic1", "Notifying", dbus.Boolean(True))
        GLib.timeout_add(200, self.notify_once)

    @dbus.service.method("org.bluez.GattCharacteristic1")
    def StopNotify(self):
        log("StopNotify %s" % self.__dbus_object_path__)
        self.set("org.bluez.GattCharacteristic1", "Notifying", dbus.Boolean(False))

    def notify_once(self):
        self.value = b"\x07\x08"
        self.set("org.bluez.GattCharacteristic1", "Value", ay(self.value))
        return False


class Descriptor(Node):
    def __init__(self):
        super().__init__(DSC, {"org.bluez.GattDescriptor1": {
            "UUID": "00002901" + BASE, "Characteristic": dbus.ObjectPath(CHR),
            "Value": ay(b"Battery"), "Flags": dbus.Array(["read", "write"], signature="s")}})
        self.value = b"Battery"

    @dbus.service.method("org.bluez.GattDescriptor1", in_signature="a{sv}", out_signature="ay")
    def ReadValue(self, opts):
        log("ReadDescriptor %s" % self.__dbus_object_path__)
        return ay(self.value)

    @dbus.service.method("org.bluez.GattDescriptor1", in_signature="aya{sv}")
    def WriteValue(self, value, opts):
        log("WriteDescriptor %s data=%s type=%s" % (
            self.__dbus_object_path__, bytes(bytearray(value)).hex(), opts.get("type", "")))
        self.value = bytes(bytearray(value))


class Service(Node):
    def __init__(self):
        super().__init__(SVC, {"org.bluez.GattService1": {
            "UUID": "0000180f" + BASE, "Primary": dbus.Boolean(True), "Device": dbus.ObjectPath(DEV1)}})


adapter = Adapter()
if args.adapter != "off":
    root.add(adapter, announce=False)
dev1 = Device(DEV1, "AA:BB:CC:DD:EE:01", "FakeSensor", -52, ["0000180f" + BASE], False, {
    "ManufacturerData": dbus.Dictionary({dbus.UInt16(0x1234): dbus.Array([dbus.Byte(1), dbus.Byte(2), dbus.Byte(3)],
                                                                          signature="y", variant_level=1)},
                                        signature="qv"),
    "ServiceData": dbus.Dictionary({"0000180f" + BASE: dbus.Array([dbus.Byte(0x64)], signature="y", variant_level=1)},
                                   signature="sv")})
dev2 = Device(DEV2, "AA:BB:CC:DD:EE:02", "Refuser", -80, ["0000181c" + BASE], True)
root.add(Service(), announce=False)
root.add(Characteristic(), announce=False)
root.add(Descriptor(), announce=False)
bus.request_name("org.bluez")
print("READY", flush=True)
GLib.MainLoop().run()

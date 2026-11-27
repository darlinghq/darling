# Bluetooth support in Darling

CoreBluetooth's **central role** is implemented on top of the host's BlueZ
daemon. It does not emulate a Bluetooth controller: the adapter managed by the host's
`bluetoothd` is the source of devices and GATT data.

## Architecture

```
guest app (CoreBluetooth)
  CBCentralManager / CBPeripheral
             │
             v
      CBHostSession (ObjC bridge)
             │   delegate callbacks on the manager's dispatch queue
             v
      CBBlueZTransport (C++/ObjC++)
             │   org.freedesktop.DBus / BlueZ D-Bus via darling_dbus
             v
      host bluetoothd ── host Bluetooth adapter
```

All D-Bus traffic goes through Darling's shared `darling_dbus_loader` wrapper, the same
lower-level layer used by other frameworks. No extra host bridge process is required. The
BlueZ connection and its signal-reading thread are owned by `CBBlueZTransport`;
`CBHostSession` translates BlueZ objects into CoreBluetooth objects and dispatches delegate
callbacks to the queue supplied to `initWithDelegate:queue:`.

The framework is built FAT. The BlueZ-backed implementations, which rely on the modern
Objective-C runtime's property synthesis, are compiled into the x86_64 slice. The i386 slice
keeps the generic stub-forwarding behavior used by the rest of the unimplemented framework,
because the legacy Objective-C runtime of that slice cannot synthesize the properties the
implementation uses. The bridge classes and the C++ helpers are
internal: they are not exported from the framework.

## Host setup

BlueZ must be running and the adapter powered on. The connection uses the system bus address
from `DBUS_SYSTEM_BUS_ADDRESS`, falling back to `unix:path=/var/run/dbus/system_bus_socket`.

With a real adapter, confirm that BlueZ sees it before launching a guest app:

```sh
bluetoothctl show
```

`CBCentralManager.state` reflects the BlueZ adapter state:

| BlueZ state | `CBManagerState` |
| --- | --- |
| No adapter or no D-Bus connection | `CBManagerStateUnsupported` |
| Adapter present but `Powered == false` | `CBManagerStatePoweredOff` |
| Adapter `Powered == true` | `CBManagerStatePoweredOn` |

`CBCentralManager.authorization` always reports `CBManagerAuthorizationAllowedAlways`;
Darling does not implement macOS' per-bundle Bluetooth privacy prompt. Access is governed by
the host's D-Bus policy.

## Supported CoreBluetooth APIs

Central role:

- `initWithDelegate:queue:` and `initWithDelegate:queue:options:`
- `delegate`, `state`, `isScanning`
- `scanForPeripheralsWithServices:options:` (including
  `CBCentralManagerScanOptionAllowDuplicatesKey`)
- `stopScan`
- `connectPeripheral:options:`, `cancelPeripheralConnection:`
- `retrievePeripheralsWithIdentifiers:`, `retrieveConnectedPeripheralsWithServices:`

Peripheral operations:

- `readRSSI`
- `discoverServices:`, `discoverIncludedServices:forService:`
- `discoverCharacteristics:forService:`, `discoverDescriptorsForCharacteristic:`
- `readValueForCharacteristic:`, `readValueForDescriptor:`
- `writeValue:forCharacteristic:type:`, `writeValue:forDescriptor:`
- `setNotifyValue:forCharacteristic:`
- `maximumWriteValueLengthForType:`

Delegate callbacks are delivered on the manager's queue for central events and through the
peripheral's `delegate` for peripheral events. Errors use `CBErrorDomain` with the documented
`CBError` codes.

## Deliberate scope limits

The following remain stubs; this change does not implement them:

- CoreBluetooth peripheral/advertiser role (`CBPeripheralManager`)
- `IOBluetooth` and `IOBluetoothUI` (classic Bluetooth)
- L2CAP channels, pairing UI, and `CBATTRequest`-based server behavior

`CBPeripheral.identifier` is derived deterministically from the BlueZ address and address type,
so it is stable across rescans within a Darling installation. It is not the same value macOS
would generate for the same physical device.

## Tests

`corebluetooth_wire` is a host-side unit test for the pure conversion helpers. It covers UUID
canonicalization, BlueZ flag to `CBCharacteristicProperties` mapping, object-path matching,
peripheral identifier derivation, and BlueZ error name to `CBError` mapping. Run it with:

```sh
ctest --test-dir build -R corebluetooth_wire --output-on-failure
```

`tests/corebluetooth-bluez-central.py` is the integration test. It starts a private D-Bus daemon
that plays the system bus, `tests/corebluetooth-mock-bluez.py` as `org.bluez` on it (an adapter, two
devices, a GATT service with a characteristic and a descriptor), and runs the guest program
`tests/src/corebluetooth_central.m` with `DBUS_SYSTEM_BUS_ADDRESS` set to the private bus. It covers
the manager state with a powered, an unpowered and a missing adapter and with no bus, a filtered
scan, discovery, connecting, GATT discovery, reads, writes with and without response, notifications,
RSSI, a failing connection and disconnecting, and checks the calls the mock received. It needs
dbus-python and PyGObject on the host and does not touch the host's Bluetooth.

## Provenance

Sources used: Apple's public CoreBluetooth API documentation and public header contracts;
BlueZ's published D-Bus API documentation (`org.bluez.Adapter`, `org.bluez.Device`,
`org.bluez.GattCharacteristic`, `org.bluez.GattDescriptor`); and the Bluetooth Core
Specification's short-UUID expansion rule. No restricted Apple source, disassembly, or
application-specific behavior was used. No new third-party dependencies were added; the test
uses the system `dbus-1` headers and library already required by the framework build.

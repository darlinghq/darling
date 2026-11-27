// CFLAGS: -framework Foundation -framework CoreBluetooth
// Copyright (C) 2026 Lubos Dolezel
//
// CoreBluetooth central role against the host's BlueZ, which the host driver
// corebluetooth-bluez-central.py replaces with a mock on a private bus
// (DBUS_SYSTEM_BUS_ADDRESS). The program follows the documented central-role
// flow: manager state, a service-filtered scan, connecting, discovering
// services, characteristics and descriptors, reading, writing with and without
// response, notifications, RSSI and cancelling the connection, then a failing
// connection. Every delegate callback must arrive on the queue given to
// -initWithDelegate:queue:.
//
//   corebluetooth_central [powered|unpowered|noadapter]
//
// "powered" (the default) runs the whole flow, the other modes only check the
// manager state that the documented CBManagerState values describe.
#import <CoreBluetooth/CoreBluetooth.h>
#import <Foundation/Foundation.h>
#include <objc/runtime.h>
#include <stdio.h>
#include <string.h>

static int failures;

static void expect(BOOL ok, NSString *what) {
    printf("%s %s\n", ok ? "PASS" : "FAIL", [what UTF8String]);
    fflush(stdout);
    if (!ok)
        failures++;
}

#if __LP64__

static const void *kQueueKey = &kQueueKey;

@interface Harness : NSObject <CBCentralManagerDelegate, CBPeripheralDelegate> {
    NSCondition *_cond;
    NSMutableDictionary *_got;
    BOOL _offQueue;
}
@property (nonatomic, retain) CBPeripheral *sensor;
@property (nonatomic, retain) CBPeripheral *refuser;
@property (nonatomic, retain) NSMutableArray *discovered;
@property (nonatomic, assign) BOOL discoveredRefuserFiltered;
- (id)waitFor: (NSString *) name;
- (void)forget: (NSString *) name;
- (BOOL)has: (NSString *) name;
- (BOOL)waitUntil: (BOOL (^)(void)) condition;
- (BOOL)callbacksStayedOnQueue;
@end

@implementation Harness
- (instancetype)init {
    self = [super init];
    _cond = [NSCondition new];
    _got = [NSMutableDictionary new];
    _discovered = [NSMutableArray new];
    return self;
}
- (void)post: (NSString *) name value: (id) value {
    if (dispatch_get_specific(kQueueKey) == NULL)
        _offQueue = YES;
    [_cond lock];
    _got[name] = value ?: [NSNull null];
    [_cond broadcast];
    [_cond unlock];
}
- (BOOL)has: (NSString *) name {
    [_cond lock];
    BOOL result = _got[name] != nil;
    [_cond unlock];
    return result;
}
- (void)forget: (NSString *) name {
    [_cond lock];
    [_got removeObjectForKey: name];
    [_cond unlock];
}
- (id)waitFor: (NSString *) name {
    NSDate *deadline = [NSDate dateWithTimeIntervalSinceNow: 10];
    [_cond lock];
    while (_got[name] == nil && [deadline timeIntervalSinceNow] > 0)
        [_cond waitUntilDate: deadline];
    id value = [[_got[name] retain] autorelease];
    [_got removeObjectForKey: name];
    [_cond unlock];
    return value;
}
// The calls that talk to the host return at once and finish on a private queue, so a state they change is polled.
- (BOOL)waitUntil: (BOOL (^)(void)) condition {
    NSDate *deadline = [NSDate dateWithTimeIntervalSinceNow: 5];
    while (!condition() && [deadline timeIntervalSinceNow] > 0)
        [NSThread sleepForTimeInterval: 0.05];
    return condition();
}
- (BOOL)callbacksStayedOnQueue {
    return !_offQueue;
}

- (void)centralManagerDidUpdateState: (CBCentralManager *) central {
    [self post: @"state" value: @(central.state)];
}
- (void)centralManager: (CBCentralManager *) central didDiscoverPeripheral: (CBPeripheral *) peripheral
     advertisementData: (NSDictionary *) advertisementData RSSI: (NSNumber *) RSSI {
    NSString *name = peripheral.name;
    @synchronized(self) {
        [_discovered addObject: name ?: @"?"];
    }
    if ([name isEqualToString: @"FakeSensor"]) {
        self.sensor = peripheral;
        [self post: @"discover-sensor" value: @{@"adv": advertisementData, @"rssi": RSSI}];
    } else if ([name isEqualToString: @"Refuser"]) {
        self.refuser = peripheral;
        [self post: @"discover-refuser" value: nil];
    }
}
- (void)centralManager: (CBCentralManager *) central didConnectPeripheral: (CBPeripheral *) peripheral {
    [self post: @"connect" value: peripheral.name];
}
- (void)centralManager: (CBCentralManager *) central didFailToConnectPeripheral: (CBPeripheral *) peripheral
                 error: (NSError *) error {
    [self post: @"connect-failed" value: error];
}
- (void)centralManager: (CBCentralManager *) central didDisconnectPeripheral: (CBPeripheral *) peripheral
                 error: (NSError *) error {
    [self post: @"disconnect" value: error];
}

- (void)peripheral: (CBPeripheral *) peripheral didDiscoverServices: (NSError *) error {
    [self post: @"services" value: error];
}
- (void)peripheral: (CBPeripheral *) peripheral didDiscoverCharacteristicsForService: (CBService *) service
             error: (NSError *) error {
    [self post: @"characteristics" value: error];
}
- (void)peripheral: (CBPeripheral *) peripheral didDiscoverDescriptorsForCharacteristic: (CBCharacteristic *) c
             error: (NSError *) error {
    [self post: @"descriptors" value: error];
}
- (void)peripheral: (CBPeripheral *) peripheral didUpdateValueForCharacteristic: (CBCharacteristic *) c
             error: (NSError *) error {
    [self post: @"value" value: c.value];
}
- (void)peripheral: (CBPeripheral *) peripheral didUpdateValueForDescriptor: (CBDescriptor *) d
             error: (NSError *) error {
    [self post: @"descriptor-value" value: d.value];
}
- (void)peripheral: (CBPeripheral *) peripheral didWriteValueForCharacteristic: (CBCharacteristic *) c
             error: (NSError *) error {
    [self post: @"write" value: error];
}
- (void)peripheral: (CBPeripheral *) peripheral didWriteValueForDescriptor: (CBDescriptor *) d
             error: (NSError *) error {
    [self post: @"descriptor-write" value: error];
}
- (void)peripheral: (CBPeripheral *) peripheral didUpdateNotificationStateForCharacteristic: (CBCharacteristic *) c
             error: (NSError *) error {
    [self post: @"notify-state" value: @(c.isNotifying)];
}
- (void)peripheral: (CBPeripheral *) peripheral didReadRSSI: (NSNumber *) RSSI error: (NSError *) error {
    [self post: @"rssi" value: RSSI];
}
@end

static NSData *bytes(const char *b, NSUInteger n) {
    return [NSData dataWithBytes: b length: n];
}

int main(int argc, char **argv) {
    @autoreleasepool {
        NSString *mode = argc > 1 ? [NSString stringWithUTF8String: argv[1]] : @"powered";
        dispatch_queue_t queue = dispatch_queue_create("corebluetooth.test", DISPATCH_QUEUE_SERIAL);
        dispatch_queue_set_specific(queue, kQueueKey, (void *) 1, NULL);
        Harness *h = [Harness new];
        BOOL uuidOnly = [mode isEqualToString: @"uuid"];
        CBCentralManager *central = uuidOnly ? nil : [[CBCentralManager alloc] initWithDelegate: h queue: queue];
        NSNumber *state = uuidOnly ? nil : [h waitFor: @"state"];
        printf("state = %ld\n", (long) [state integerValue]);

        if ([mode isEqualToString: @"uuid"]) {
            // QUIRK checks, see the comment at DisplayUUIDString in CBUUID.m
            CBUUID *a = [CBUUID UUIDWithString: @"180f"];
            expect([a.UUIDString isEqual: @"180F"] && [a.data length] == 2, @"QUIRK: a 16-bit UUID keeps its width");
            CBUUID *m = [CBUUID UUIDWithString: @"0000180f"];
            expect([m.UUIDString isEqual: @"0000180F"] && [m.data length] == 4, @"QUIRK: a 32-bit UUID keeps its width");
            CBUUID *l = [CBUUID UUIDWithString: @"0000180f-0000-1000-8000-00805f9b34fb"];
            expect([l.UUIDString isEqual: @"0000180F-0000-1000-8000-00805F9B34FB"] && [l.data length] == 16,
                   @"QUIRK: a 128-bit UUID of the base range stays 128-bit");
            expect([a isEqual: l] && [a isEqual: m] && [a hash] == [l hash] && [a hash] == [m hash],
                   @"QUIRK: UUIDs of different widths with the same value are equal");
            unsigned char bytes[] = {0x18, 0x0f};
            expect([[CBUUID UUIDWithData: [NSData dataWithBytes: bytes length: 2]] isEqual: a], @"UUIDWithData: takes two bytes");
            expect([[CBUUID UUIDWithString: @"6e400001-b5a3-f393-e0a9-e50e24dcca9e"].UUIDString
                       isEqual: @"6E400001-B5A3-F393-E0A9-E50E24DCCA9E"], @"a vendor UUID is upper case");
            expect(![a isEqual: [CBUUID UUIDWithString: @"180E"]], @"different UUIDs differ");
        } else if ([mode isEqualToString: @"unpowered"]) {
            expect([state integerValue] == CBManagerStatePoweredOff, @"an unpowered adapter reports PoweredOff");
        } else if ([mode isEqualToString: @"noadapter"]) {
            expect([state integerValue] == CBManagerStateUnsupported, @"no adapter reports Unsupported");
        } else {
            expect([state integerValue] == CBManagerStatePoweredOn, @"a powered adapter reports PoweredOn");
            expect(central.state == CBManagerStatePoweredOn, @"-state agrees with the delegate callback");
            expect(central.isScanning == NO, @"the manager is not scanning at first");

            CBUUID *batteryService = [CBUUID UUIDWithString: @"180F"];
            [central scanForPeripheralsWithServices: @[batteryService] options: nil];
            expect([h waitUntil: ^BOOL { return central.isScanning; }], @"-isScanning is YES once the scan has started");
            NSDictionary *found = [h waitFor: @"discover-sensor"];
            expect(found != nil, @"the peripheral with the filtered service is discovered");
            NSDictionary *adv = found[@"adv"];
            expect([found[@"rssi"] intValue] == -52, @"the discovery reports the RSSI");
            expect([adv[CBAdvertisementDataLocalNameKey] isEqual: @"FakeSensor"], @"advertisement local name");
            expect([adv[CBAdvertisementDataServiceUUIDsKey] containsObject: batteryService],
                   @"advertisement service UUIDs hold the 16-bit UUID");
            expect([adv[CBAdvertisementDataManufacturerDataKey] isEqual: bytes("\x34\x12\x01\x02\x03", 5)],
                   @"manufacturer data is the company identifier (little endian) followed by the data");
            NSData *svcData = adv[CBAdvertisementDataServiceDataKey][batteryService];
            expect([svcData isEqual: bytes("\x64", 1)], @"service data is keyed by CBUUID");
            // The mock also announces a second peripheral that does not advertise the service.
            [NSThread sleepForTimeInterval: 0.6];
            @synchronized(h) {
                expect(![h.discovered containsObject: @"Refuser"],
                       @"a peripheral without the requested service is not reported by a filtered scan");
            }
            [central stopScan];
            expect([h waitUntil: ^BOOL { return !central.isScanning; }], @"-isScanning is NO once the scan has stopped");

            CBPeripheral *sensor = h.sensor;
            sensor.delegate = h;
            expect(sensor.state == CBPeripheralStateDisconnected, @"a discovered peripheral is disconnected");
            expect(sensor.identifier != nil, @"the peripheral has an identifier");
            expect([[central retrievePeripheralsWithIdentifiers: @[sensor.identifier]] count] == 1,
                   @"retrievePeripheralsWithIdentifiers returns the known peripheral");
            expect([[central retrievePeripheralsWithIdentifiers: @[[NSUUID UUID]]] count] == 0,
                   @"an unknown identifier retrieves nothing");

            [central connectPeripheral: sensor options: nil];
            expect([[h waitFor: @"connect"] isEqual: @"FakeSensor"], @"didConnectPeripheral is delivered");
            expect(sensor.state == CBPeripheralStateConnected, @"the peripheral is connected");
            expect([[central retrieveConnectedPeripheralsWithServices: @[batteryService]] count] == 1,
                   @"retrieveConnectedPeripheralsWithServices returns the connected peripheral");
            expect([[central retrieveConnectedPeripheralsWithServices: @[[CBUUID UUIDWithString: @"1234"]]] count] == 0,
                   @"a service the peripheral lacks retrieves nothing");

            [sensor discoverServices: nil];
            expect([h waitFor: @"services"] == [NSNull null], @"didDiscoverServices has no error");
            expect([sensor.services count] == 1 && [[[sensor.services[0] UUID] UUIDString] isEqual: @"180F"],
                   @"the battery service is discovered");
            CBService *service = sensor.services[0];
            expect(service.isPrimary, @"the service is primary");
            expect(service.peripheral == sensor, @"the service refers back to its peripheral");

            [sensor discoverCharacteristics: nil forService: service];
            expect([h waitFor: @"characteristics"] == [NSNull null], @"didDiscoverCharacteristics has no error");
            CBCharacteristic *level = [service.characteristics firstObject];
            expect(level != nil && [level.UUID.UUIDString isEqual: @"2A19"],
                   @"the battery level characteristic is discovered");
            CBCharacteristicProperties want = CBCharacteristicPropertyRead | CBCharacteristicPropertyWrite |
                CBCharacteristicPropertyWriteWithoutResponse | CBCharacteristicPropertyNotify;
            expect(level.properties == want, @"the characteristic properties follow the BlueZ flags");

            [sensor discoverDescriptorsForCharacteristic: level];
            expect([h waitFor: @"descriptors"] == [NSNull null], @"didDiscoverDescriptors has no error");
            CBDescriptor *description = [level.descriptors firstObject];
            expect(description != nil && [description.UUID isEqual: [CBUUID UUIDWithString: @"2901"]],
                   @"the user description descriptor is discovered");

            [sensor readValueForCharacteristic: level];
            expect([[h waitFor: @"value"] isEqual: bytes("\x55", 1)], @"reading a characteristic returns its value");
            expect([level.value isEqual: bytes("\x55", 1)], @"the characteristic's value property is updated");

            [sensor readValueForDescriptor: description];
            id descriptorValue = [h waitFor: @"descriptor-value"];
            expect([descriptorValue isEqual: @"Battery"], @"the user description descriptor reads as a string");

            [sensor writeValue: bytes("\x0a\x0b", 2) forCharacteristic: level type: CBCharacteristicWriteWithResponse];
            expect([h waitFor: @"write"] == [NSNull null], @"a write with response is confirmed without error");
            [sensor writeValue: bytes("\x0c", 1) forCharacteristic: level type: CBCharacteristicWriteWithoutResponse];
            [sensor writeValue: bytes("\x01", 1) forDescriptor: description];
            expect([h waitFor: @"descriptor-write"] == [NSNull null], @"a descriptor write is confirmed");
            [NSThread sleepForTimeInterval: 0.5];
            expect(![h has: @"write"], @"a write without response is not confirmed to the delegate");
            expect([sensor maximumWriteValueLengthForType: CBCharacteristicWriteWithResponse] > 0,
                   @"maximumWriteValueLengthForType is positive");

            [sensor writeValue: bytes("\xde\xad", 2) forCharacteristic: level type: CBCharacteristicWriteWithResponse];
            NSError *refused = [h waitFor: @"write"];
            expect([refused isKindOfClass: [NSError class]] && [refused.domain isEqual: CBErrorDomain] &&
                       refused.code == CBErrorOperationNotSupported,
                   @"a write the peripheral rejects is reported as a CBError");

            [h forget: @"value"];
            [sensor setNotifyValue: YES forCharacteristic: level];
            expect([[h waitFor: @"notify-state"] boolValue] == YES, @"notifications are switched on");
            expect(level.isNotifying, @"-isNotifying is YES");
            expect([[h waitFor: @"value"] isEqual: bytes("\x07\x08", 2)], @"a notification delivers the new value");
            [sensor setNotifyValue: NO forCharacteristic: level];
            id off = [h waitFor: @"notify-state"];
            expect([off boolValue] == NO && !level.isNotifying, @"notifications are switched off");

            [sensor readRSSI];
            expect([[h waitFor: @"rssi"] intValue] == -52, @"readRSSI reports the RSSI");

            [central cancelPeripheralConnection: sensor];
            id disconnected = [h waitFor: @"disconnect"];
            expect(disconnected == [NSNull null], @"a requested disconnect has no error");
            expect(sensor.state == CBPeripheralStateDisconnected, @"the peripheral is disconnected again");

            [central scanForPeripheralsWithServices: nil options: nil];
            expect([h waitFor: @"discover-refuser"] != nil, @"an unfiltered scan finds every peripheral");
            [central stopScan];
            [central connectPeripheral: h.refuser options: nil];
            NSError *failed = [h waitFor: @"connect-failed"];
            expect([failed isKindOfClass: [NSError class]] && [failed.domain isEqual: CBErrorDomain] &&
                       failed.code == CBErrorConnectionFailed,
                   @"a failing connection reports didFailToConnectPeripheral with CBErrorConnectionFailed");
            expect(h.refuser.state == CBPeripheralStateDisconnected, @"the failed peripheral is disconnected");
        }
        if (!uuidOnly)
            expect([h callbacksStayedOnQueue], @"every delegate callback ran on the manager's queue");
        [central release];
    }
    printf(failures ? "FAILED %d\n" : "ALL PASSED\n", failures);
    return failures != 0;
}

#else

// The framework's 32-bit slice keeps the stub forwarding that answers unknown selectors (its legacy Objective-C
// runtime cannot synthesize the properties of the implementation): loading it, creating a manager and calling it must still not crash, and there is no powered adapter
// and no scan.
int main(int argc, char **argv) {
    @autoreleasepool {
        CBCentralManager *central = [[CBCentralManager alloc] initWithDelegate: nil queue: nil];
        // the stub answers the unimplemented selectors without doing anything (the creation may even return nil)
        [central scanForPeripheralsWithServices: nil options: nil];
        [central stopScan];
        expect(central.state != CBManagerStatePoweredOn, @"the 32-bit slice reports no powered adapter");
        expect(central.isScanning == NO, @"the 32-bit slice does not scan");
        [central release];
    }
    printf(failures ? "FAILED %d\n" : "ALL PASSED\n", failures);
    return failures != 0;
}

#endif

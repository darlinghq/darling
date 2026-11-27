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

#import "CBHostSession.h"
#import "CBBlueZTransport.h"
#import "CBInternal.h"

#include <algorithm>
#include <cstdlib>

// The BlueZ bridge is only built into the 64-bit slice: its Objective-C++
// implementation relies on the modern runtime's property synthesis. The 32-bit
// slice keeps the generic CoreBluetooth stubs.
#if __LP64__

namespace
{
NSString *const kEventType = @"type";
NSString *const kEventPath = @"path";
NSString *const kEventDevice = @"device";
NSString *const kEventUUID = @"uuid";
NSString *const kEventValue = @"value";
NSString *const kEventServices = @"services";
NSString *const kEventCharacteristics = @"characteristics";
NSString *const kEventDescriptors = @"descriptors";
NSString *const kEventErrorCode = @"error";
NSString *const kEventFlag = @"flag";
NSString *const kEventRSSI = @"rssi";

NSString *const kDevicePath = @"path";
NSString *const kDeviceIdentifier = @"identifier";
NSString *const kDeviceName = @"name";
NSString *const kDeviceRSSI = @"rssi";
NSString *const kDeviceUUIDs = @"uuids";
NSString *const kDeviceManufacturerData = @"manufacturer";
NSString *const kDeviceServiceData = @"serviceData";
NSString *const kDeviceTxPower = @"txPower";
NSString *const kDeviceConnectable = @"connectable";

NSString *const kAttributePath = @"path";
NSString *const kAttributeUUID = @"uuid";
NSString *const kAttributeHandle = @"handle";
NSString *const kAttributePrimary = @"primary";
NSString *const kAttributeProperties = @"properties";
NSString *const kAttributeNotifying = @"notifying";
NSString *const kAttributeValue = @"value";
NSString *const kAttributeIncludes = @"includes";

NSDictionary *DeviceDictionary(const cbbluez::DeviceInfo &device)
{
    NSMutableDictionary *result = [NSMutableDictionary dictionary];
    [result setObject:[NSString stringWithUTF8String:device.path.c_str()] forKey:kDevicePath];
    const std::string identifier = cbbluez::PeripheralIdentifier(device.address, device.addressType);
    [result setObject:[NSString stringWithUTF8String:identifier.c_str()] forKey:kDeviceIdentifier];
    if (!device.alias.empty())
        [result setObject:[NSString stringWithUTF8String:device.alias.c_str()] forKey:kDeviceName];
    if (device.hasRSSI)
        [result setObject:[NSNumber numberWithInt:device.RSSI] forKey:kDeviceRSSI];
    if (device.hasTxPower)
        [result setObject:[NSNumber numberWithInt:device.txPower] forKey:kDeviceTxPower];
    if (device.hasConnectable)
        [result setObject:[NSNumber numberWithBool:device.connectable] forKey:kDeviceConnectable];

    NSMutableArray *UUIDs = [NSMutableArray array];
    for (const std::string &UUID : device.UUIDs)
        [UUIDs addObject:[NSString stringWithUTF8String:UUID.c_str()]];
    [result setObject:UUIDs forKey:kDeviceUUIDs];

    // CoreBluetooth represents manufacturer data as one NSData value whose
    // first two bytes are the little-endian company identifier. BlueZ provides
    // a company-ID-keyed dictionary; use the lowest ID for a deterministic
    // mapping when an advertisement carries more than one entry.
    if (!device.manufacturerData.empty())
    {
        const auto &entry = *device.manufacturerData.begin();
        const uint16_t company = static_cast<uint16_t>(std::strtoul(entry.first.c_str(), nullptr, 10));
        NSMutableData *manufacturer = [NSMutableData dataWithCapacity:2 + entry.second.size()];
        const unsigned char identifier[2] = {
            static_cast<unsigned char>(company & 0xff),
            static_cast<unsigned char>(company >> 8),
        };
        [manufacturer appendBytes:identifier length:2];
        [manufacturer appendBytes:entry.second.data() length:entry.second.size()];
        [result setObject:manufacturer forKey:kDeviceManufacturerData];
    }

    NSMutableDictionary *serviceData = [NSMutableDictionary dictionary];
    for (const auto &entry : device.serviceData)
    {
        NSString *key = [NSString stringWithUTF8String:entry.first.c_str()];
        NSData *data = [NSData dataWithBytes:entry.second.data() length:entry.second.size()];
        [serviceData setObject:data forKey:key];
    }
    [result setObject:serviceData forKey:kDeviceServiceData];
    return result;
}

NSArray *ServiceDictionaries(const std::vector<cbbluez::ServiceInfo> &services)
{
    NSMutableArray *result = [NSMutableArray array];
    for (const cbbluez::ServiceInfo &service : services)
    {
        NSMutableDictionary *entry = [NSMutableDictionary dictionary];
        [entry setObject:[NSString stringWithUTF8String:service.path.c_str()] forKey:kAttributePath];
        [entry setObject:[NSString stringWithUTF8String:service.UUID.c_str()] forKey:kAttributeUUID];
        [entry setObject:[NSNumber numberWithUnsignedInt:service.handle] forKey:kAttributeHandle];
        [entry setObject:[NSNumber numberWithBool:service.primary] forKey:kAttributePrimary];
        NSMutableArray *includes = [NSMutableArray array];
        for (const std::string &included : service.includes)
            [includes addObject:[NSString stringWithUTF8String:included.c_str()]];
        [entry setObject:includes forKey:kAttributeIncludes];
        [result addObject:entry];
    }
    return result;
}

NSArray *CharacteristicDictionaries(const std::vector<cbbluez::CharacteristicInfo> &characteristics)
{
    NSMutableArray *result = [NSMutableArray array];
    for (const cbbluez::CharacteristicInfo &characteristic : characteristics)
    {
        NSMutableDictionary *entry = [NSMutableDictionary dictionary];
        [entry setObject:[NSString stringWithUTF8String:characteristic.path.c_str()] forKey:kAttributePath];
        [entry setObject:[NSString stringWithUTF8String:characteristic.UUID.c_str()] forKey:kAttributeUUID];
        [entry setObject:[NSNumber numberWithUnsignedInt:characteristic.handle] forKey:kAttributeHandle];
        [entry setObject:[NSNumber numberWithUnsignedInt:characteristic.properties] forKey:kAttributeProperties];
        [entry setObject:[NSNumber numberWithBool:characteristic.notifying] forKey:kAttributeNotifying];
        [entry setObject:[NSData dataWithBytes:characteristic.value.data() length:characteristic.value.size()]
                  forKey:kAttributeValue];
        [result addObject:entry];
    }
    return result;
}

NSArray *DescriptorDictionaries(const std::vector<cbbluez::DescriptorInfo> &descriptors)
{
    NSMutableArray *result = [NSMutableArray array];
    for (const cbbluez::DescriptorInfo &descriptor : descriptors)
    {
        NSMutableDictionary *entry = [NSMutableDictionary dictionary];
        [entry setObject:[NSString stringWithUTF8String:descriptor.path.c_str()] forKey:kAttributePath];
        [entry setObject:[NSString stringWithUTF8String:descriptor.UUID.c_str()] forKey:kAttributeUUID];
        [entry setObject:[NSNumber numberWithUnsignedInt:descriptor.handle] forKey:kAttributeHandle];
        [entry setObject:[NSData dataWithBytes:descriptor.value.data() length:descriptor.value.size()]
                  forKey:kAttributeValue];
        [result addObject:entry];
    }
    return result;
}

NSDictionary *EventDictionary(const cbbluez::Event &event)
{
    NSMutableDictionary *result = [NSMutableDictionary dictionary];
    [result setObject:[NSNumber numberWithInt:static_cast<int>(event.type)] forKey:kEventType];
    [result setObject:[NSNumber numberWithInt:event.state] forKey:@"state"];
    [result setObject:[NSNumber numberWithInt:event.errorCode] forKey:kEventErrorCode];
    [result setObject:[NSNumber numberWithBool:event.flag] forKey:kEventFlag];
    [result setObject:[NSNumber numberWithInt:event.rssi] forKey:kEventRSSI];
    if (!event.path.empty())
        [result setObject:[NSString stringWithUTF8String:event.path.c_str()] forKey:kEventPath];
    if (!event.uuid.empty())
        [result setObject:[NSString stringWithUTF8String:event.uuid.c_str()] forKey:kEventUUID];
    if (!event.value.empty())
        [result setObject:[NSData dataWithBytes:event.value.data() length:event.value.size()] forKey:kEventValue];
    if (!event.device.path.empty())
        [result setObject:DeviceDictionary(event.device) forKey:kEventDevice];
    if (!event.services.empty())
        [result setObject:ServiceDictionaries(event.services) forKey:kEventServices];
    if (!event.characteristics.empty())
        [result setObject:CharacteristicDictionaries(event.characteristics) forKey:kEventCharacteristics];
    if (!event.descriptors.empty())
        [result setObject:DescriptorDictionaries(event.descriptors) forKey:kEventDescriptors];
    return result;
}

NSError *CBErrorWithCode(NSInteger code)
{
    NSString *description = @"Bluetooth operation failed";
    switch (code)
    {
    case CBErrorInvalidParameters: description = @"Invalid Bluetooth parameter"; break;
    case CBErrorNotConnected: description = @"Peripheral is not connected"; break;
    case CBErrorTimeout: description = @"Bluetooth operation timed out"; break;
    case CBErrorConnectionFailed: description = @"Bluetooth connection failed"; break;
    case CBErrorConnectionRejected: description = @"Bluetooth connection was rejected"; break;
    case CBErrorConnectionCancelled: description = @"Bluetooth connection was cancelled"; break;
    case CBErrorOperationNotSupported: description = @"Bluetooth operation is not supported"; break;
    default: break;
    }
    return [NSError errorWithDomain:CBErrorDomain
                              code:code
                          userInfo:[NSDictionary dictionaryWithObject:description
                                                               forKey:NSLocalizedDescriptionKey]];
}

NSError *EventError(NSDictionary *event)
{
    const NSInteger code = [[event objectForKey:kEventErrorCode] integerValue];
    return code == 0 ? nil : CBErrorWithCode(code);
}

NSArray<NSString *> *UUIDStrings(NSArray<CBUUID *> *UUIDs)
{
    NSMutableArray *result = [NSMutableArray array];
    for (CBUUID *UUID in UUIDs)
    {
        if (UUID.UUIDString != nil)
            [result addObject:UUID.UUIDString];
    }
    return result;
}

// The transport compares UUIDs in the 128-bit form BlueZ uses, whatever width the CBUUID has.
std::vector<std::string> UUIDVector(NSArray<NSString *> *strings)
{
    std::vector<std::string> result;
    for (NSString *value in strings)
    {
        const std::string normalized = cbbluez::NormalizeUUID([value UTF8String]);
        result.push_back(normalized.empty() ? std::string([value UTF8String]) : normalized);
    }
    return result;
}

// BlueZ and the wire helpers use the 128-bit form; the CBUUID of an attribute that is derived
// from the Bluetooth base UUID has the short form, as for the original framework.
CBUUID *UUIDFromWireString(NSString *wire)
{
    NSString *text = [wire uppercaseString];
    if ([text length] == 36 && [text hasSuffix:@"-0000-1000-8000-00805F9B34FB"])
    {
        NSString *head = [text substringToIndex:8];
        text = [head hasPrefix:@"0000"] ? [head substringFromIndex:4] : head;
    }
    return [CBUUID UUIDWithString:text];
}

std::vector<uint8_t> BytesFromData(NSData *data)
{
    const uint8_t *bytes = static_cast<const uint8_t *>([data bytes]);
    return std::vector<uint8_t>(bytes, bytes + [data length]);
}

// The value types the CBUUID descriptor constants document: the extended properties and the
// client and server configuration are numbers, the user description is a string, everything
// else stays the raw data.
id DescriptorValueFromData(CBUUID *UUID, NSData *data)
{
    NSString *name = UUID.UUIDString;
    if ([name isEqualToString:@"2901"])
    {
        NSString *text = [[[NSString alloc] initWithData:data encoding:NSUTF8StringEncoding] autorelease];
        return text != nil ? text : data;
    }
    if (([name isEqualToString:@"2900"] || [name isEqualToString:@"2902"] ||
         [name isEqualToString:@"2903"]) && [data length] >= 1 && [data length] <= 2)
    {
        // ATT values are little endian
        const uint8_t *bytes = static_cast<const uint8_t *>([data bytes]);
        unsigned value = bytes[0];
        if ([data length] == 2)
            value |= static_cast<unsigned>(bytes[1]) << 8;
        return [NSNumber numberWithUnsignedInt:value];
    }
    return data;
}

struct SessionState
{
    cbbluez::Transport *transport = nullptr;
    CBCentralManager *manager = nil;
    dispatch_queue_t queue = nullptr;
    dispatch_queue_t workQueue = nullptr;
    NSMutableDictionary *peripheralsByPath = nil;
    NSMutableDictionary *objectsByPath = nil;
    NSMutableDictionary *advertisements = nil;
    NSRecursiveLock *lock = nil;
};
} // namespace

@interface CBHostSession ()
- (void)handleEvent:(NSDictionary *)event;
- (CBPeripheral *)peripheralForDevice:(NSDictionary *)device;
- (id)objectForPath:(NSString *)path;
@end

@implementation CBHostSession
{
    SessionState *_sessionState;
}

- (instancetype)initWithManager:(CBCentralManager *)manager queue:(dispatch_queue_t)queue
{
    self = [super init];
    if (self == nil)
        return nil;

    _sessionState = new SessionState();
    _sessionState->manager = manager;
    _sessionState->queue = queue != nil ? queue : dispatch_get_main_queue();
    dispatch_retain(_sessionState->queue);
    _sessionState->workQueue = dispatch_queue_create("com.darling.corebluetooth.bluez", DISPATCH_QUEUE_SERIAL);
    _sessionState->peripheralsByPath = [[NSMutableDictionary alloc] init];
    _sessionState->objectsByPath = [[NSMutableDictionary alloc] init];
    _sessionState->advertisements = [[NSMutableDictionary alloc] init];
    _sessionState->lock = [[NSRecursiveLock alloc] init];

    __unsafe_unretained CBHostSession *session = self;
    _sessionState->transport = new cbbluez::Transport([session](const cbbluez::Event &event) {
        @autoreleasepool {
            NSDictionary *dictionary = EventDictionary(event);
            CBHostSession *strongSession = [session retain];
            dispatch_async(session->_sessionState->queue, ^{
                [strongSession handleEvent:dictionary];
                [strongSession release];
            });
        }
    });
    _sessionState->transport->open();
    return self;
}

- (void)dealloc
{
    [self close];
    if (_sessionState != nullptr)
    {
        if (_sessionState->workQueue != nullptr)
            dispatch_release(_sessionState->workQueue);
        if (_sessionState->queue != nullptr)
            dispatch_release(_sessionState->queue);
        [_sessionState->peripheralsByPath release];
        [_sessionState->objectsByPath release];
        [_sessionState->advertisements release];
        [_sessionState->lock release];
        delete _sessionState;
        _sessionState = nullptr;
    }
    [super dealloc];
}

- (void)close
{
    if (_sessionState == nullptr)
        return;
    // Let queued D-Bus operations finish before destroying the connection
    // they are using.
    if (_sessionState->workQueue != nullptr)
        dispatch_barrier_sync(_sessionState->workQueue, ^{});
    // Peripherals outlive their manager in well-behaved apps; drop the
    // back-pointer so later calls on them become safe no-ops.
    @synchronized (_sessionState->lock) {
        for (CBPeripheral *peripheral in [_sessionState->peripheralsByPath allValues])
            peripheral.hostSession = nil;
    }
    if (_sessionState->transport != nullptr)
    {
        delete _sessionState->transport;
        _sessionState->transport = nullptr;
    }
}

- (int)state
{
    return _sessionState != nullptr && _sessionState->transport != nullptr ? _sessionState->transport->state()
                                                             : cbbluez::kStateUnsupported;
}

- (BOOL)isScanning
{
    return _sessionState != nullptr && _sessionState->transport != nullptr && _sessionState->transport->isScanning() ? YES : NO;
}

- (CBPeripheral *)peripheralForDevice:(NSDictionary *)device
{
    NSString *path = [device objectForKey:kDevicePath];
    CBPeripheral *peripheral = [_sessionState->peripheralsByPath objectForKey:path];
    if (peripheral == nil)
    {
        peripheral = [[CBPeripheral alloc] init];
        peripheral.bluezPath = path;
        NSString *identifierString = [device objectForKey:kDeviceIdentifier];
        NSUUID *identifier = identifierString != nil
            ? [[[NSUUID alloc] initWithUUIDString:identifierString] autorelease]
            : [NSUUID UUID];
        peripheral.identifier = identifier;
        peripheral.state = CBPeripheralStateDisconnected;
        peripheral.hostSession = self;
        [_sessionState->peripheralsByPath setObject:peripheral forKey:path];
        [_sessionState->objectsByPath setObject:peripheral forKey:path];
        [peripheral release];
    }

    NSString *name = [device objectForKey:kDeviceName];
    if (name != nil && (peripheral.name == nil || ![peripheral.name isEqualToString:name]))
        peripheral.name = name;
    NSNumber *RSSI = [device objectForKey:kDeviceRSSI];
    if (RSSI != nil)
        peripheral.RSSI = RSSI;

    NSMutableDictionary *advertisement = [NSMutableDictionary dictionary];
    if (name != nil)
        [advertisement setObject:name forKey:CBAdvertisementDataLocalNameKey];
    NSArray *UUIDs = [device objectForKey:kDeviceUUIDs];
    if (UUIDs != nil)
    {
        NSMutableArray *objects = [NSMutableArray array];
        for (NSString *value in UUIDs)
        {
            CBUUID *UUID = UUIDFromWireString(value);
            if (UUID != nil)
                [objects addObject:UUID];
        }
        [advertisement setObject:objects forKey:CBAdvertisementDataServiceUUIDsKey];
    }
    NSDictionary *manufacturer = [device objectForKey:kDeviceManufacturerData];
    if (manufacturer != nil)
        [advertisement setObject:manufacturer forKey:CBAdvertisementDataManufacturerDataKey];
    NSMutableDictionary *serviceData = [NSMutableDictionary dictionary];
    for (NSString *value in [device objectForKey:kDeviceServiceData])
    {
        CBUUID *UUID = UUIDFromWireString(value);
        if (UUID != nil)
            [serviceData setObject:[[device objectForKey:kDeviceServiceData] objectForKey:value]
                            forKey:UUID];
    }
    if ([serviceData count] != 0)
        [advertisement setObject:serviceData forKey:CBAdvertisementDataServiceDataKey];
    NSNumber *txPower = [device objectForKey:kDeviceTxPower];
    if (txPower != nil)
        [advertisement setObject:txPower forKey:CBAdvertisementDataTxPowerLevelKey];
    NSNumber *connectable = [device objectForKey:kDeviceConnectable];
    if (connectable != nil)
        [advertisement setObject:connectable forKey:CBAdvertisementDataIsConnectable];
    [_sessionState->advertisements setObject:advertisement forKey:[peripheral.identifier UUIDString]];
    peripheral.advertisementData = advertisement;
    return peripheral;
}

- (id)objectForPath:(NSString *)path
{
    return [_sessionState->objectsByPath objectForKey:path];
}

- (void)handleEvent:(NSDictionary *)event
{
 @synchronized (_sessionState->lock) {
    CBCentralManager *manager = _sessionState->manager;
    if (manager == nil || _sessionState->transport == nullptr)
        return;
    id<CBCentralManagerDelegate> centralDelegate = manager.delegate;
    const cbbluez::EventType type = static_cast<cbbluez::EventType>(
        [[event objectForKey:kEventType] intValue]);
    NSString *path = [event objectForKey:kEventPath];
    if (path != nil && type != cbbluez::EventType::DeviceFound &&
        type != cbbluez::EventType::StateChanged && [self objectForPath:path] == nil)
        return;

    switch (type)
    {
    case cbbluez::EventType::StateChanged:
        manager.state = static_cast<CBManagerState>([[event objectForKey:@"state"] integerValue]);
        if ([centralDelegate respondsToSelector:@selector(centralManagerDidUpdateState:)])
            [centralDelegate centralManagerDidUpdateState:manager];
        break;

    case cbbluez::EventType::DeviceFound:
    {
        NSDictionary *device = [event objectForKey:kEventDevice];
        CBPeripheral *peripheral = [self peripheralForDevice:device];
        if ([centralDelegate respondsToSelector:@selector(centralManager:didDiscoverPeripheral:advertisementData:RSSI:)])
        {
            NSNumber *RSSI = [device objectForKey:kDeviceRSSI];
            [centralDelegate centralManager:manager
                       didDiscoverPeripheral:peripheral
                           advertisementData:peripheral.advertisementData
                                        RSSI:RSSI != nil ? RSSI : [NSNumber numberWithInt:0]];
        }
        break;
    }

    case cbbluez::EventType::Connected:
    {
        CBPeripheral *peripheral = [self objectForPath:path];
        peripheral.state = CBPeripheralStateConnected;
        if ([centralDelegate respondsToSelector:@selector(centralManager:didConnectPeripheral:)])
            [centralDelegate centralManager:manager didConnectPeripheral:peripheral];
        break;
    }

    case cbbluez::EventType::ConnectFailed:
    {
        CBPeripheral *peripheral = [self objectForPath:path];
        peripheral.state = CBPeripheralStateDisconnected;
        if ([centralDelegate respondsToSelector:@selector(centralManager:didFailToConnectPeripheral:error:)])
            [centralDelegate centralManager:manager
                   didFailToConnectPeripheral:peripheral
                                        error:EventError(event)];
        break;
    }

    case cbbluez::EventType::Disconnected:
    {
        CBPeripheral *peripheral = [self objectForPath:path];
        peripheral.state = CBPeripheralStateDisconnected;
        peripheral.services = @[];
        if ([centralDelegate respondsToSelector:@selector(centralManager:didDisconnectPeripheral:error:)])
            [centralDelegate centralManager:manager
                    didDisconnectPeripheral:peripheral
                                      error:EventError(event)];
        break;
    }

    case cbbluez::EventType::ServicesDiscovered:
    {
        CBPeripheral *peripheral = [self objectForPath:path];
        NSMutableArray *services = [NSMutableArray array];
        for (NSDictionary *entry in [event objectForKey:kEventServices])
        {
            NSString *servicePath = [entry objectForKey:kAttributePath];
            CBService *service = [_sessionState->objectsByPath objectForKey:servicePath];
            if (service == nil)
            {
                service = [[CBService alloc] init];
                service.bluezPath = servicePath;
                service.UUID = UUIDFromWireString([entry objectForKey:kAttributeUUID]);
                service.isPrimary = [[entry objectForKey:kAttributePrimary] boolValue];
                service.includedPaths = [entry objectForKey:kAttributeIncludes];
                service.characteristics = @[];
                service.includedServices = @[];
                [_sessionState->objectsByPath setObject:service forKey:servicePath];
                [service release];
            }
            service.peripheral = peripheral;
            [services addObject:service];
        }
        peripheral.services = services;
        id<CBPeripheralDelegate> delegate = peripheral.delegate;
        if ([delegate respondsToSelector:@selector(peripheral:didDiscoverServices:)])
            [delegate peripheral:peripheral didDiscoverServices:EventError(event)];
        break;
    }

    case cbbluez::EventType::IncludedServicesDiscovered:
    {
        CBService *service = [self objectForPath:path];
        NSMutableArray *included = [NSMutableArray array];
        for (NSDictionary *entry in [event objectForKey:kEventServices])
        {
            NSString *includedPath = [entry objectForKey:kAttributePath];
            CBService *includedService = [_sessionState->objectsByPath objectForKey:includedPath];
            if (includedService == nil)
            {
                includedService = [[CBService alloc] init];
                includedService.bluezPath = includedPath;
                includedService.UUID = UUIDFromWireString([entry objectForKey:kAttributeUUID]);
                includedService.isPrimary = [[entry objectForKey:kAttributePrimary] boolValue];
                includedService.includedPaths = [entry objectForKey:kAttributeIncludes];
                includedService.characteristics = @[];
                includedService.includedServices = @[];
                [_sessionState->objectsByPath setObject:includedService forKey:includedPath];
                [includedService release];
            }
            includedService.peripheral = service.peripheral;
            [included addObject:includedService];
        }
        service.includedServices = included;
        id<CBPeripheralDelegate> delegate = service.peripheral.delegate;
        if ([delegate respondsToSelector:@selector(peripheral:didDiscoverIncludedServicesForService:error:)])
            [delegate peripheral:service.peripheral
                didDiscoverIncludedServicesForService:service
                                               error:EventError(event)];
        break;
    }

    case cbbluez::EventType::CharacteristicsDiscovered:
    {
        CBService *service = [self objectForPath:path];
        NSMutableArray *characteristics = [NSMutableArray array];
        for (NSDictionary *entry in [event objectForKey:kEventCharacteristics])
        {
            NSString *characteristicPath = [entry objectForKey:kAttributePath];
            CBCharacteristic *characteristic = [_sessionState->objectsByPath objectForKey:characteristicPath];
            if (characteristic == nil)
            {
                characteristic = [[CBCharacteristic alloc] init];
                characteristic.bluezPath = characteristicPath;
                characteristic.UUID = UUIDFromWireString([entry objectForKey:kAttributeUUID]);
                characteristic.descriptors = @[];
                [_sessionState->objectsByPath setObject:characteristic forKey:characteristicPath];
                [characteristic release];
            }
            characteristic.service = service;
            characteristic.properties = static_cast<CBCharacteristicProperties>(
                [[entry objectForKey:kAttributeProperties] unsignedIntegerValue]);
            characteristic.isNotifying = [[entry objectForKey:kAttributeNotifying] boolValue];
            NSData *value = [entry objectForKey:kAttributeValue];
            if (value != nil)
                characteristic.value = value;
            [characteristics addObject:characteristic];
        }
        service.characteristics = characteristics;
        id<CBPeripheralDelegate> delegate = service.peripheral.delegate;
        if ([delegate respondsToSelector:@selector(peripheral:didDiscoverCharacteristicsForService:error:)])
            [delegate peripheral:service.peripheral
                didDiscoverCharacteristicsForService:service
                                               error:EventError(event)];
        break;
    }

    case cbbluez::EventType::DescriptorsDiscovered:
    {
        CBCharacteristic *characteristic = [self objectForPath:path];
        NSMutableArray *descriptors = [NSMutableArray array];
        for (NSDictionary *entry in [event objectForKey:kEventDescriptors])
        {
            NSString *descriptorPath = [entry objectForKey:kAttributePath];
            CBDescriptor *descriptor = [_sessionState->objectsByPath objectForKey:descriptorPath];
            if (descriptor == nil)
            {
                descriptor = [[CBDescriptor alloc] init];
                descriptor.bluezPath = descriptorPath;
                descriptor.UUID = UUIDFromWireString([entry objectForKey:kAttributeUUID]);
                [_sessionState->objectsByPath setObject:descriptor forKey:descriptorPath];
                [descriptor release];
            }
            descriptor.characteristic = characteristic;
            descriptor.value = [entry objectForKey:kAttributeValue];
            [descriptors addObject:descriptor];
        }
        characteristic.descriptors = descriptors;
        id<CBPeripheralDelegate> delegate = characteristic.service.peripheral.delegate;
        if ([delegate respondsToSelector:@selector(peripheral:didDiscoverDescriptorsForCharacteristic:error:)])
            [delegate peripheral:characteristic.service.peripheral
                didDiscoverDescriptorsForCharacteristic:characteristic
                                                  error:EventError(event)];
        break;
    }

    case cbbluez::EventType::ValueUpdated:
    {
        id object = [self objectForPath:path];
        NSError *error = EventError(event);
        NSData *value = [event objectForKey:kEventValue];
        if ([object isKindOfClass:[CBCharacteristic class]])
        {
            CBCharacteristic *characteristic = object;
            if (value != nil)
                characteristic.value = value;
            id<CBPeripheralDelegate> delegate = characteristic.service.peripheral.delegate;
            if ([delegate respondsToSelector:@selector(peripheral:didUpdateValueForCharacteristic:error:)])
                [delegate peripheral:characteristic.service.peripheral
                  didUpdateValueForCharacteristic:characteristic
                                           error:error];
        }
        else if ([object isKindOfClass:[CBDescriptor class]])
        {
            CBDescriptor *descriptor = object;
            if (value != nil)
                descriptor.value = DescriptorValueFromData(descriptor.UUID, value);
            id<CBPeripheralDelegate> delegate = descriptor.characteristic.service.peripheral.delegate;
            if ([delegate respondsToSelector:@selector(peripheral:didUpdateValueForDescriptor:error:)])
                [delegate peripheral:descriptor.characteristic.service.peripheral
                  didUpdateValueForDescriptor:descriptor
                                       error:error];
        }
        break;
    }

    case cbbluez::EventType::WriteCompleted:
    {
        id object = [self objectForPath:path];
        NSError *error = EventError(event);
        if ([object isKindOfClass:[CBCharacteristic class]])
        {
            CBCharacteristic *characteristic = object;
            id<CBPeripheralDelegate> delegate = characteristic.service.peripheral.delegate;
            if ([delegate respondsToSelector:@selector(peripheral:didWriteValueForCharacteristic:error:)])
                [delegate peripheral:characteristic.service.peripheral
                  didWriteValueForCharacteristic:characteristic
                                           error:error];
        }
        else if ([object isKindOfClass:[CBDescriptor class]])
        {
            CBDescriptor *descriptor = object;
            id<CBPeripheralDelegate> delegate = descriptor.characteristic.service.peripheral.delegate;
            if ([delegate respondsToSelector:@selector(peripheral:didWriteValueForDescriptor:error:)])
                [delegate peripheral:descriptor.characteristic.service.peripheral
                  didWriteValueForDescriptor:descriptor
                                       error:error];
        }
        break;
    }

    case cbbluez::EventType::NotificationStateChanged:
    {
        CBCharacteristic *characteristic = [self objectForPath:path];
        characteristic.isNotifying = [[event objectForKey:kEventFlag] boolValue];
        id<CBPeripheralDelegate> delegate = characteristic.service.peripheral.delegate;
        if ([delegate respondsToSelector:@selector(peripheral:didUpdateNotificationStateForCharacteristic:error:)])
            [delegate peripheral:characteristic.service.peripheral
                didUpdateNotificationStateForCharacteristic:characteristic
                                                      error:EventError(event)];
        break;
    }

    case cbbluez::EventType::RSSIUpdated:
    {
        CBPeripheral *peripheral = [self objectForPath:path];
        peripheral.RSSI = [NSNumber numberWithInt:[[event objectForKey:kEventRSSI] intValue]];
        id<CBPeripheralDelegate> delegate = peripheral.delegate;
        if ([delegate respondsToSelector:@selector(peripheral:didReadRSSI:error:)])
            [delegate peripheral:peripheral
                        didReadRSSI:peripheral.RSSI
                              error:EventError(event)];
        break;
    }

    case cbbluez::EventType::NameUpdated:
    {
        CBPeripheral *peripheral = [self objectForPath:path];
        id<CBPeripheralDelegate> delegate = peripheral.delegate;
        if ([delegate respondsToSelector:@selector(peripheralDidUpdateName:)])
            [delegate peripheralDidUpdateName:peripheral];
        break;
    }
    }
 }
}

- (void)scanForServices:(NSArray<CBUUID *> *)services allowDuplicates:(BOOL)allowDuplicates
{
    NSArray<NSString *> *strings = UUIDStrings(services);
    dispatch_async(_sessionState->workQueue, ^{
        if (_sessionState->transport == nullptr)
            return;
        if (!_sessionState->transport->startScan(UUIDVector(strings), allowDuplicates == YES))
        {
            cbbluez::Event event;
            event.type = cbbluez::EventType::StateChanged;
            event.state = _sessionState->transport->state();
            dispatch_async(_sessionState->queue, ^{
                [self handleEvent:EventDictionary(event)];
            });
        }
    });
}

- (void)stopScan
{
    dispatch_async(_sessionState->workQueue, ^{
        if (_sessionState->transport != nullptr)
            _sessionState->transport->stopScan();
    });
}

- (void)connectPeripheral:(CBPeripheral *)peripheral
{
    peripheral.state = CBPeripheralStateConnecting;
    dispatch_async(_sessionState->workQueue, ^{
        if (_sessionState->transport == nullptr)
            return;
        std::string errorName;
        _sessionState->transport->connect([peripheral.bluezPath UTF8String], &errorName);
    });
}

- (void)cancelPeripheralConnection:(CBPeripheral *)peripheral
{
    peripheral.state = CBPeripheralStateDisconnecting;
    dispatch_async(_sessionState->workQueue, ^{
        if (_sessionState->transport != nullptr)
            _sessionState->transport->disconnect([peripheral.bluezPath UTF8String]);
    });
}

- (NSArray<CBPeripheral *> *)peripheralsWithIdentifiers:(NSArray<NSUUID *> *)identifiers
{
    NSMutableArray *result = [NSMutableArray array];
    @synchronized (_sessionState->lock) {
        for (NSUUID *identifier in identifiers)
        {
            for (CBPeripheral *peripheral in [_sessionState->peripheralsByPath allValues])
            {
                if ([peripheral.identifier isEqual:identifier])
                    [result addObject:peripheral];
            }
        }
    }
    return result;
}

- (NSArray<CBPeripheral *> *)connectedPeripheralsWithServices:(NSArray<CBUUID *> *)services
{
    std::vector<cbbluez::DeviceInfo> devices;
    @synchronized (_sessionState->lock) {
        if (_sessionState->transport == nullptr ||
            !_sessionState->transport->connectedDevices(UUIDVector(UUIDStrings(services)), &devices))
            return @[];

        NSMutableArray *result = [NSMutableArray array];
        for (const cbbluez::DeviceInfo &device : devices)
        {
            CBPeripheral *peripheral = [self peripheralForDevice:DeviceDictionary(device)];
            peripheral.state = CBPeripheralStateConnected;
            [result addObject:peripheral];
        }
        return result;
    }
}

- (void)peripheral:(CBPeripheral *)peripheral discoverServices:(NSArray<CBUUID *> *)services
{
    dispatch_async(_sessionState->workQueue, ^{
        if (_sessionState->transport == nullptr)
            return;
        _sessionState->transport->discoverServices([peripheral.bluezPath UTF8String],
                                                   UUIDVector(UUIDStrings(services)), nullptr);
    });
}

- (void)peripheral:(CBPeripheral *)peripheral
    discoverIncludedServices:(NSArray<CBUUID *> *)services
                  forService:(CBService *)service
{
    dispatch_async(_sessionState->workQueue, ^{
        if (_sessionState->transport == nullptr)
            return;
        _sessionState->transport->discoverIncludedServices([service.bluezPath UTF8String],
                                                           UUIDVector(UUIDStrings(services)), nullptr);
    });
}

- (void)peripheral:(CBPeripheral *)peripheral
    discoverCharacteristics:(NSArray<CBUUID *> *)characteristics
                 forService:(CBService *)service
{
    dispatch_async(_sessionState->workQueue, ^{
        if (_sessionState->transport == nullptr)
            return;
        _sessionState->transport->discoverCharacteristics([service.bluezPath UTF8String],
                                                          UUIDVector(UUIDStrings(characteristics)), nullptr);
    });
}

- (void)peripheral:(CBPeripheral *)peripheral
    discoverDescriptorsForCharacteristic:(CBCharacteristic *)characteristic
{
    dispatch_async(_sessionState->workQueue, ^{
        if (_sessionState->transport == nullptr)
            return;
        _sessionState->transport->discoverDescriptors([characteristic.bluezPath UTF8String], nullptr);
    });
}

- (void)readRSSIForPeripheral:(CBPeripheral *)peripheral
{
    dispatch_async(_sessionState->workQueue, ^{
        if (_sessionState->transport == nullptr)
            return;
        _sessionState->transport->readRSSI([peripheral.bluezPath UTF8String], nullptr);
    });
}

- (void)peripheral:(CBPeripheral *)peripheral readValueForCharacteristic:(CBCharacteristic *)characteristic
{
    dispatch_async(_sessionState->workQueue, ^{
        if (_sessionState->transport == nullptr)
            return;
        std::vector<uint8_t> value;
        std::string errorName;
        _sessionState->transport->readValue([characteristic.bluezPath UTF8String], &value, &errorName);
    });
}

- (void)peripheral:(CBPeripheral *)peripheral readValueForDescriptor:(CBDescriptor *)descriptor
{
    dispatch_async(_sessionState->workQueue, ^{
        if (_sessionState->transport == nullptr)
            return;
        std::vector<uint8_t> value;
        std::string errorName;
        _sessionState->transport->readDescriptorValue([descriptor.bluezPath UTF8String], &value, &errorName);
    });
}

- (void)peripheral:(CBPeripheral *)peripheral
       writeValue:(NSData *)data
forCharacteristic:(CBCharacteristic *)characteristic
              type:(CBCharacteristicWriteType)type
{
    std::vector<uint8_t> bytes = BytesFromData(data);
    dispatch_async(_sessionState->workQueue, ^{
        if (_sessionState->transport == nullptr)
            return;
        std::string errorName;
        _sessionState->transport->writeValue([characteristic.bluezPath UTF8String], bytes,
                                             type == CBCharacteristicWriteWithResponse, &errorName);
    });
}

- (void)peripheral:(CBPeripheral *)peripheral
       writeValue:(NSData *)data
      forDescriptor:(CBDescriptor *)descriptor
{
    std::vector<uint8_t> bytes = BytesFromData(data);
    dispatch_async(_sessionState->workQueue, ^{
        if (_sessionState->transport == nullptr)
            return;
        std::string errorName;
        _sessionState->transport->writeDescriptorValue([descriptor.bluezPath UTF8String], bytes, &errorName);
    });
}

- (void)peripheral:(CBPeripheral *)peripheral
      setNotifyValue:(BOOL)enabled
   forCharacteristic:(CBCharacteristic *)characteristic
{
    dispatch_async(_sessionState->workQueue, ^{
        if (_sessionState->transport == nullptr)
            return;
        std::string errorName;
        _sessionState->transport->setNotify([characteristic.bluezPath UTF8String], enabled == YES, &errorName);
    });
}

@end

#endif // __LP64__

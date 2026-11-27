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

#ifndef CB_BLUEZ_TRANSPORT_H
#define CB_BLUEZ_TRANSPORT_H

#include "bluez_wire.h"

#include <dbus/dbus.h>

#include <atomic>
#include <functional>
#include <map>
#include <mutex>
#include <set>
#include <string>
#include <thread>
#include <vector>

struct darling_dbus;

namespace cbbluez
{
// Mirrors CBManagerState. Kept here so the transport can report manager state
// without depending on Foundation.
enum ManagerState
{
    kStateUnknown = 0,
    kStatePoweredOff = 1,
    kStatePoweredOn = 2,
    kStateResetting = 3,
    kStateUnauthorized = 4,
    kStateUnsupported = 5
};

enum class EventType
{
    StateChanged,
    DeviceFound,
    Connected,
    ConnectFailed,
    Disconnected,
    ServicesDiscovered,
    IncludedServicesDiscovered,
    CharacteristicsDiscovered,
    DescriptorsDiscovered,
    ValueUpdated,
    WriteCompleted,
    NotificationStateChanged,
    RSSIUpdated,
    NameUpdated
};

struct Event
{
    EventType type = EventType::StateChanged;
    int state = 0;
    int errorCode = 0;
    std::string path;
    std::string uuid;
    std::vector<uint8_t> value;
    DeviceInfo device;
    std::vector<ServiceInfo> services;
    std::vector<CharacteristicInfo> characteristics;
    std::vector<DescriptorInfo> descriptors;
    int rssi = 0;
    bool flag = false;
};

class Transport
{
public:
    explicit Transport(const std::function<void(const Event &)> &callback);
    ~Transport();

    Transport(const Transport &) = delete;
    Transport &operator=(const Transport &) = delete;

    bool open();
    int state() const;
    bool isScanning() const;

    bool startScan(const std::vector<std::string> &UUIDs, bool allowDuplicates);
    void stopScan();

    bool connect(const std::string &path, std::string *errorName);
    bool disconnect(const std::string &path);

    bool discoverServices(const std::string &devicePath, const std::vector<std::string> &UUIDs,
                          std::vector<ServiceInfo> *services);
    bool discoverIncludedServices(const std::string &servicePath,
                                  const std::vector<std::string> &UUIDs,
                                  std::vector<ServiceInfo> *services);
    bool discoverCharacteristics(const std::string &servicePath,
                                 const std::vector<std::string> &UUIDs,
                                 std::vector<CharacteristicInfo> *characteristics);
    bool discoverDescriptors(const std::string &characteristicPath,
                             std::vector<DescriptorInfo> *descriptors);

    bool readValue(const std::string &path, std::vector<uint8_t> *value, std::string *errorName);
    bool readDescriptorValue(const std::string &path, std::vector<uint8_t> *value,
                             std::string *errorName);
    bool writeValue(const std::string &path, const std::vector<uint8_t> &value,
                    bool withResponse, std::string *errorName);
    bool writeDescriptorValue(const std::string &path, const std::vector<uint8_t> &value,
                              std::string *errorName);
    bool setNotify(const std::string &path, bool enabled, std::string *errorName);

    bool readRSSI(const std::string &devicePath, int *RSSI);
    bool connectedDevices(const std::vector<std::string> &UUIDs,
                          std::vector<DeviceInfo> *devices);

private:
    void run();
    void handleMessage(DBusMessage *message);
    void handlePropertiesChanged(const std::string &path, DBusMessageIter *iter);
    void handleInterfacesAdded(DBusMessageIter *iter);
    void emit(const Event &event);

    bool call(const std::string &path, const std::string &interface,
              const std::string &method,
              const std::function<void(DBusMessageIter *)> &appendArguments,
              const std::function<bool(DBusMessageIter *)> &readReply,
              std::string *errorName);
    bool getManagedObjects(std::vector<ObjectRecord> *objects);
    bool getProperty(const std::string &path, const std::string &interface,
                     const std::string &name, Value *value);
    bool readAttribute(const std::string &path, const char *interface,
                       std::vector<uint8_t> *value, std::string *errorName);
    // A non-null writeType adds the characteristic-only "type" option. BlueZ
    // rejects that option for GattDescriptor1.WriteValue.
    bool writeAttribute(const std::string &path, const char *interface,
                        const std::vector<uint8_t> &value, const char *writeType,
                        std::string *errorName);
    void reportDevice(const DeviceInfo &device, bool force);
    DeviceInfo deviceForPath(const std::string &path);
    void updateDevice(const DeviceInfo &device);
    void waitForServicesResolved(const std::string &path);

    const struct darling_dbus *api_ = nullptr;
    DBusConnection *connection_ = nullptr;
    std::string adapterPath_;
    std::function<void(const Event &)> callback_;
    std::thread thread_;
    std::atomic<bool> stopping_{false};
    std::atomic<int> state_{0};
    std::atomic<bool> scanning_{false};
    bool allowDuplicates_ = false;
    std::vector<std::string> scanUUIDs_;
    std::mutex mutex_;
    std::map<std::string, DeviceInfo> devices_;
    std::set<std::string> seenDevices_;
    std::map<std::string, bool> notifying_;
    std::map<std::string, bool> connected_;
};

} // namespace cbbluez

#endif

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

#include "CBBlueZTransport.h"
#include "darling_dbus.h"

#include <poll.h>
#include <unistd.h>

#include <algorithm>
#include <cerrno>
#include <cstdlib>
#include <cstring>

namespace cbbluez
{
namespace
{
constexpr int kCallTimeoutMs = 10000;

const char *const kBluezService = "org.bluez";
const char *const kRootPath = "/";
const char *const kObjectManager = "org.freedesktop.DBus.ObjectManager";
const char *const kProperties = "org.freedesktop.DBus.Properties";
const char *const kAdapter = "org.bluez.Adapter1";
const char *const kDevice = "org.bluez.Device1";
const char *const kService = "org.bluez.GattService1";
const char *const kCharacteristic = "org.bluez.GattCharacteristic1";
const char *const kDescriptor = "org.bluez.GattDescriptor1";

void AppendString(const struct darling_dbus *api, DBusMessageIter *iter, const char *value)
{
    api->message_iter_append_basic(iter, DBUS_TYPE_STRING, &value);
}

void AppendVariantArray(const struct darling_dbus *api, DBusMessageIter *dict,
                        const char *key, const std::vector<std::string> &values)
{
    DBusMessageIter entry, variant, array;
    if (!api->message_iter_open_container(dict, DBUS_TYPE_DICT_ENTRY, nullptr, &entry))
        return;
    AppendString(api, &entry, key);
    if (!api->message_iter_open_container(&entry, DBUS_TYPE_VARIANT, "as", &variant))
    {
        api->message_iter_close_container(dict, &entry);
        return;
    }
    if (!api->message_iter_open_container(&variant, DBUS_TYPE_ARRAY, "s", &array))
    {
        api->message_iter_close_container(&entry, &variant);
        api->message_iter_close_container(dict, &entry);
        return;
    }
    for (const std::string &value : values)
    {
        const char *text = value.c_str();
        api->message_iter_append_basic(&array, DBUS_TYPE_STRING, &text);
    }
    api->message_iter_close_container(&variant, &array);
    api->message_iter_close_container(&entry, &variant);
    api->message_iter_close_container(dict, &entry);
}

void AppendVariantBool(const struct darling_dbus *api, DBusMessageIter *dict,
                       const char *key, bool value)
{
    DBusMessageIter entry, variant;
    if (!api->message_iter_open_container(dict, DBUS_TYPE_DICT_ENTRY, nullptr, &entry))
        return;
    AppendString(api, &entry, key);
    dbus_bool_t dbusValue = value ? TRUE : FALSE;
    if (!api->message_iter_open_container(&entry, DBUS_TYPE_VARIANT, "b", &variant))
    {
        api->message_iter_close_container(dict, &entry);
        return;
    }
    api->message_iter_append_basic(&variant, DBUS_TYPE_BOOLEAN, &dbusValue);
    api->message_iter_close_container(&entry, &variant);
    api->message_iter_close_container(dict, &entry);
}

void AppendVariantString(const struct darling_dbus *api, DBusMessageIter *dict,
                         const char *key, const char *value)
{
    DBusMessageIter entry, variant;
    if (!api->message_iter_open_container(dict, DBUS_TYPE_DICT_ENTRY, nullptr, &entry))
        return;
    AppendString(api, &entry, key);
    if (!api->message_iter_open_container(&entry, DBUS_TYPE_VARIANT, "s", &variant))
    {
        api->message_iter_close_container(dict, &entry);
        return;
    }
    AppendString(api, &variant, value);
    api->message_iter_close_container(&entry, &variant);
    api->message_iter_close_container(dict, &entry);
}

void AppendByteArray(const struct darling_dbus *api, DBusMessageIter *iter,
                     const std::vector<uint8_t> &bytes)
{
    DBusMessageIter array;
    api->message_iter_open_container(iter, DBUS_TYPE_ARRAY, "y", &array);
    for (uint8_t byte : bytes)
    {
        unsigned char value = byte;
        api->message_iter_append_basic(&array, DBUS_TYPE_BYTE, &value);
    }
    api->message_iter_close_container(iter, &array);
}

bool ReadByteArray(const struct darling_dbus *api, DBusMessageIter *iter,
                   std::vector<uint8_t> *out)
{
    Value value;
    if (!ReadValue(api, iter, &value) || value.type != Value::Type::Bytes)
        return false;
    *out = value.bytes;
    return true;
}

bool HasUUID(const std::vector<std::string> &available, const std::vector<std::string> &wanted)
{
    if (wanted.empty())
        return true;
    for (const std::string &uuid : wanted)
    {
        if (std::find(available.begin(), available.end(), uuid) != available.end())
            return true;
    }
    return false;
}
} // namespace

Transport::Transport(const std::function<void(const Event &)> &callback)
    : callback_(callback)
{
}

Transport::~Transport()
{
    stopping_.store(true);
    if (thread_.joinable())
        thread_.join();
    if (connection_ != nullptr)
    {
        api_->connection_close(connection_);
        api_->connection_unref(connection_);
        connection_ = nullptr;
    }
}

bool Transport::open()
{
    api_ = darling_dbus_get(0);
    if (api_ == nullptr)
        return false;

    const auto reportState = [this]() {
        Event event;
        event.type = EventType::StateChanged;
        event.state = state_.load();
        emit(event);
    };

    const char *envAddress = std::getenv("DBUS_SYSTEM_BUS_ADDRESS");
    const std::string address = (envAddress != nullptr && envAddress[0] != '\0')
                                    ? envAddress
                                    : "unix:path=/var/run/dbus/system_bus_socket";

    DBusError error;
    api_->error_init(&error);
    connection_ = api_->connection_open_private(address.c_str(), &error);
    if (connection_ == nullptr || !api_->bus_register(connection_, &error))
    {
        api_->error_free(&error);
        if (connection_ != nullptr)
        {
            api_->connection_close(connection_);
            api_->connection_unref(connection_);
            connection_ = nullptr;
        }
        state_.store(kStateUnsupported);
        reportState();
        return false;
    }

    const char *const matches[] = {
        "type='signal',sender='org.bluez',interface='org.freedesktop.DBus.ObjectManager'",
        "type='signal',sender='org.bluez',interface='org.freedesktop.DBus.Properties'",
        "type='signal',sender='org.bluez',interface='org.bluez.Device1'",
    };
    for (const char *match : matches)
        api_->bus_add_match(connection_, match, &error);
    api_->connection_flush(connection_);
    api_->error_free(&error);

    std::vector<ObjectRecord> objects;
    if (!getManagedObjects(&objects))
    {
        state_.store(kStateUnsupported);
        reportState();
        return true;
    }
    bool haveAdapter = false;
    bool powered = false;
    for (const ObjectRecord &record : objects)
    {
        const auto adapter = record.interfaces.find(kAdapter);
        if (adapter == record.interfaces.end())
            continue;
        haveAdapter = true;
        const auto it = adapter->second.find("Powered");
        if (it != adapter->second.end() && it->second.type == Value::Type::Boolean &&
            it->second.boolValue)
        {
            adapterPath_ = record.path;
            powered = true;
        }
        else if (adapterPath_.empty())
            adapterPath_ = record.path;
    }
    if (!haveAdapter)
        state_.store(kStateUnsupported);
    else if (!powered)
        state_.store(kStatePoweredOff);
    else
        state_.store(kStatePoweredOn);
    reportState();

    thread_ = std::thread(&Transport::run, this);
    return true;
}

int Transport::state() const
{
    return state_.load();
}

bool Transport::isScanning() const
{
    return scanning_.load();
}

void Transport::emit(const Event &event)
{
    if (callback_)
        callback_(event);
}

void Transport::run()
{
    int fd = -1;
    if (!api_->connection_get_unix_fd(connection_, &fd))
        return;

    while (!stopping_.load())
    {
        struct pollfd descriptor;
        descriptor.fd = fd;
        descriptor.events = POLLIN;
        descriptor.revents = 0;
        const int ready = poll(&descriptor, 1, 100);
        if (ready < 0)
        {
            if (errno == EINTR)
                continue;
            break;
        }
        if (ready > 0)
        {
            std::lock_guard<std::mutex> lock(mutex_);
            api_->connection_read_write(connection_, 0);
        }
        std::vector<DBusMessage *> messages;
        {
            std::lock_guard<std::mutex> lock(mutex_);
            while (DBusMessage *message = api_->connection_pop_message(connection_))
                messages.push_back(message);
        }
        for (DBusMessage *message : messages)
        {
            handleMessage(message);
            api_->message_unref(message);
            if (stopping_.load())
                break;
        }
    }
}

void Transport::handleMessage(DBusMessage *message)
{
    if (api_->message_is_signal(message, kObjectManager, "InterfacesAdded"))
    {
        DBusMessageIter iter;
        if (api_->message_iter_init(message, &iter))
            handleInterfacesAdded(&iter);
        return;
    }
    if (api_->message_is_signal(message, kProperties, "PropertiesChanged"))
    {
        const char *path = api_->message_get_path(message);
        DBusMessageIter iter;
        if (path != nullptr && api_->message_iter_init(message, &iter))
            handlePropertiesChanged(path, &iter);
        return;
    }
    if (api_->message_is_signal(message, kDevice, "Disconnected"))
    {
        const char *path = api_->message_get_path(message);
        if (path == nullptr)
            return;
        Event event;
        event.type = EventType::Disconnected;
        event.path = path;
        emit(event);
    }
}

void Transport::handleInterfacesAdded(DBusMessageIter *iter)
{
    const char *path = nullptr;
    if (api_->message_iter_get_arg_type(iter) != DBUS_TYPE_OBJECT_PATH)
        return;
    api_->message_iter_get_basic(iter, &path);
    api_->message_iter_next(iter);

    // The second argument is the dictionary of interfaces (a{sa{sv}}) of the new object.
    if (api_->message_iter_get_arg_type(iter) != DBUS_TYPE_ARRAY)
        return;
    DBusMessageIter interfaces;
    api_->message_iter_recurse(iter, &interfaces);

    ObjectRecord record;
    record.path = path != nullptr ? path : "";
    while (api_->message_iter_get_arg_type(&interfaces) == DBUS_TYPE_DICT_ENTRY)
    {
        DBusMessageIter entry;
        api_->message_iter_recurse(&interfaces, &entry);
        const char *interface = nullptr;
        api_->message_iter_get_basic(&entry, &interface);
        api_->message_iter_next(&entry);
        std::map<std::string, Value> properties;
        if (interface != nullptr && ReadProperties(api_, &entry, &properties))
            record.interfaces[interface] = properties;
        api_->message_iter_next(&interfaces);
    }

    DeviceInfo device;
    if (DeviceFromRecord(record, &device) && scanning_.load())
        reportDevice(device, false);
}

void Transport::handlePropertiesChanged(const std::string &path, DBusMessageIter *iter)
{
    const char *interface = nullptr;
    api_->message_iter_get_basic(iter, &interface);
    api_->message_iter_next(iter);
    std::map<std::string, Value> changed;
    if (!ReadProperties(api_, iter, &changed))
        return;

    const std::string name = interface != nullptr ? interface : "";
    if (name == kAdapter && path == adapterPath_)
    {
        const auto powered = changed.find("Powered");
        if (powered != changed.end() && powered->second.type == Value::Type::Boolean)
        {
            state_.store(powered->second.boolValue ? kStatePoweredOn : kStatePoweredOff);
            Event event;
            event.type = EventType::StateChanged;
            event.state = state_.load();
            emit(event);
        }
        return;
    }

    if (name == kDevice)
    {
        DeviceInfo device = deviceForPath(path);
        if (device.path.empty())
            return;
        updateDevice(device);
        if (scanning_.load() &&
            (changed.count("RSSI") != 0 || changed.count("Name") != 0 ||
             changed.count("Alias") != 0 || changed.count("UUIDs") != 0 ||
             changed.count("ManufacturerData") != 0 || changed.count("ServiceData") != 0))
        {
            reportDevice(device, false);
        }
        const auto connected = changed.find("Connected");
        if (connected != changed.end() && connected->second.type == Value::Type::Boolean)
        {
            bool emitChange = false;
            {
                std::lock_guard<std::mutex> lock(mutex_);
                const auto previous = connected_.find(path);
                emitChange = previous == connected_.end() ||
                             previous->second != connected->second.boolValue;
                connected_[path] = connected->second.boolValue;
            }
            if (emitChange)
            {
                Event event;
                event.type = connected->second.boolValue ? EventType::Connected : EventType::Disconnected;
                event.path = path;
                emit(event);
            }
        }
        if (changed.count("Name") != 0 || changed.count("Alias") != 0)
        {
            Event event;
            event.type = EventType::NameUpdated;
            event.path = path;
            emit(event);
        }
        return;
    }

    if (name == kCharacteristic)
    {
        const auto value = changed.find("Value");
        if (value != changed.end() && value->second.type == Value::Type::Bytes)
        {
            bool notifying = false;
            {
                std::lock_guard<std::mutex> lock(mutex_);
                const auto it = notifying_.find(path);
                notifying = it != notifying_.end() && it->second;
            }
            if (notifying)
            {
                Event event;
                event.type = EventType::ValueUpdated;
                event.path = path;
                event.value = value->second.bytes;
                emit(event);
            }
        }
        const auto notifying = changed.find("Notifying");
        if (notifying != changed.end() && notifying->second.type == Value::Type::Boolean)
        {
            {
                std::lock_guard<std::mutex> lock(mutex_);
                notifying_[path] = notifying->second.boolValue;
            }
            Event event;
            event.type = EventType::NotificationStateChanged;
            event.path = path;
            event.flag = notifying->second.boolValue;
            emit(event);
        }
    }
}

bool Transport::call(const std::string &path, const std::string &interface,
                     const std::string &method,
                     const std::function<void(DBusMessageIter *)> &appendArguments,
                     const std::function<bool(DBusMessageIter *)> &readReply,
                     std::string *errorName)
{
    if (connection_ == nullptr || api_ == nullptr)
        return false;

    std::lock_guard<std::mutex> lock(mutex_);
    DBusMessage *message = api_->message_new_method_call(kBluezService, path.c_str(),
                                                          interface.c_str(), method.c_str());
    if (message == nullptr)
        return false;
    if (appendArguments)
    {
        DBusMessageIter args;
        api_->message_iter_init_append(message, &args);
        appendArguments(&args);
    }

    DBusError error;
    api_->error_init(&error);
    DBusMessage *reply = api_->connection_send_with_reply_and_block(
        connection_, message, kCallTimeoutMs, &error);
    api_->message_unref(message);
    if (reply == nullptr)
    {
        if (errorName != nullptr && error.name != nullptr)
            *errorName = error.name;
        api_->error_free(&error);
        return false;
    }
    api_->error_free(&error);

    bool result = true;
    if (readReply)
    {
        DBusMessageIter args;
        if (api_->message_iter_init(reply, &args))
            result = readReply(&args);
    }
    api_->message_unref(reply);
    return result;
}

bool Transport::getManagedObjects(std::vector<ObjectRecord> *objects)
{
    return call(kRootPath, kObjectManager, "GetManagedObjects", nullptr,
                [this, objects](DBusMessageIter *iter) {
                    return ReadManagedObjects(api_, iter, objects);
                },
                nullptr);
}

bool Transport::getProperty(const std::string &path, const std::string &interface,
                            const std::string &name, Value *value)
{
    return call(path, kProperties, "Get",
                [this, interface, name](DBusMessageIter *iter) {
                    AppendString(api_, iter, interface.c_str());
                    AppendString(api_, iter, name.c_str());
                },
                [this, value](DBusMessageIter *iter) {
                    return ReadValue(api_, iter, value);
                },
                nullptr);
}

DeviceInfo Transport::deviceForPath(const std::string &path)
{
    std::vector<ObjectRecord> objects;
    if (!getManagedObjects(&objects))
        return DeviceInfo();
    for (const ObjectRecord &record : objects)
    {
        if (record.path == path)
        {
            DeviceInfo device;
            DeviceFromRecord(record, &device);
            return device;
        }
    }
    return DeviceInfo();
}

void Transport::updateDevice(const DeviceInfo &device)
{
    std::lock_guard<std::mutex> lock(mutex_);
    devices_[device.path] = device;
}

void Transport::reportDevice(const DeviceInfo &device, bool force)
{
    {
        std::lock_guard<std::mutex> lock(mutex_);
        // BlueZ merges filters from all clients, so a signal can describe a
        // device that does not match ours. CoreBluetooth promises only
        // matching devices.
        if (!HasUUID(device.UUIDs, scanUUIDs_))
            return;
        if (!force && !allowDuplicates_ && !seenDevices_.insert(device.path).second)
            return;
    }
    updateDevice(device);
    Event event;
    event.type = EventType::DeviceFound;
    event.device = device;
    emit(event);
}

void Transport::waitForServicesResolved(const std::string &path)
{
    for (int attempt = 0; attempt < 30; ++attempt)
    {
        Value value;
        if (!getProperty(path, kDevice, "ServicesResolved", &value))
            return;
        if (value.type == Value::Type::Boolean && value.boolValue)
            return;
        usleep(100000);
    }
}

bool Transport::startScan(const std::vector<std::string> &UUIDs, bool allowDuplicates)
{
    if (adapterPath_.empty())
        return false;
    std::string errorName;
    const bool filtered = call(
        adapterPath_, kAdapter, "SetDiscoveryFilter",
        [this, &UUIDs, allowDuplicates](DBusMessageIter *iter) {
            DBusMessageIter dict;
            if (!api_->message_iter_open_container(iter, DBUS_TYPE_ARRAY, "{sv}", &dict))
                return;
            AppendVariantArray(api_, &dict, "UUIDs", UUIDs);
            AppendVariantString(api_, &dict, "Transport", "le");
            AppendVariantBool(api_, &dict, "DuplicateData", allowDuplicates);
            api_->message_iter_close_container(iter, &dict);
        },
        nullptr, &errorName);
    if (!filtered)
        return false;
    if (!call(adapterPath_, kAdapter, "StartDiscovery", nullptr, nullptr, &errorName))
        return false;
    {
        std::lock_guard<std::mutex> lock(mutex_);
        allowDuplicates_ = allowDuplicates;
        scanUUIDs_ = UUIDs;
        seenDevices_.clear();
    }
    scanning_.store(true);

    // Existing BlueZ device objects are not announced again by a fresh
    // discovery session, so report the currently known devices once.
    std::vector<ObjectRecord> objects;
    if (getManagedObjects(&objects))
    {
        for (const ObjectRecord &record : objects)
        {
            DeviceInfo device;
            if (!DeviceFromRecord(record, &device) || !HasUUID(device.UUIDs, UUIDs))
                continue;
            reportDevice(device, false);
        }
    }
    return true;
}

void Transport::stopScan()
{
    if (adapterPath_.empty())
        return;
    scanning_.store(false);
    {
        std::lock_guard<std::mutex> lock(mutex_);
        scanUUIDs_.clear();
    }
    call(adapterPath_, kAdapter, "StopDiscovery", nullptr, nullptr, nullptr);
}

bool Transport::connect(const std::string &path, std::string *errorName)
{
    if (!call(path, kDevice, "Connect", nullptr, nullptr, errorName))
    {
        Event event;
        event.type = EventType::ConnectFailed;
        event.path = path;
        event.errorCode = ErrorCodeForDBusName(errorName != nullptr ? *errorName : "");
        emit(event);
        return false;
    }
    bool emitChange = false;
    {
        std::lock_guard<std::mutex> lock(mutex_);
        const auto previous = connected_.find(path);
        emitChange = previous == connected_.end() || !previous->second;
        connected_[path] = true;
    }
    if (emitChange)
    {
        Event event;
        event.type = EventType::Connected;
        event.path = path;
        emit(event);
    }
    return true;
}

bool Transport::disconnect(const std::string &path)
{
    std::string errorName;
    if (!call(path, kDevice, "Disconnect", nullptr, nullptr, &errorName))
        return false;
    bool emitChange = false;
    {
        std::lock_guard<std::mutex> lock(mutex_);
        const auto previous = connected_.find(path);
        emitChange = previous != connected_.end() && previous->second;
        connected_[path] = false;
    }
    if (emitChange)
    {
        Event event;
        event.type = EventType::Disconnected;
        event.path = path;
        emit(event);
    }
    return true;
}

bool Transport::discoverServices(const std::string &devicePath,
                                 const std::vector<std::string> &UUIDs,
                                 std::vector<ServiceInfo> *services)
{
    waitForServicesResolved(devicePath);
    std::vector<ObjectRecord> objects;
    if (!getManagedObjects(&objects))
        return false;

    std::vector<ServiceInfo> owned;
    std::vector<ServiceInfo> &result = services != nullptr ? *services : owned;
    result.clear();
    for (const ObjectRecord &record : objects)
    {
        if (!PathHasPrefix(record.path, devicePath))
            continue;
        ServiceInfo service;
        if (!ServiceFromRecord(record, &service) || !HasUUID({service.UUID}, UUIDs))
            continue;
        result.push_back(service);
    }

    Event event;
    event.type = EventType::ServicesDiscovered;
    event.path = devicePath;
    event.services = result;
    event.errorCode = result.empty() ? 9 : 0;
    emit(event);
    return true;
}

bool Transport::discoverIncludedServices(const std::string &servicePath,
                                         const std::vector<std::string> &UUIDs,
                                         std::vector<ServiceInfo> *services)
{
    std::vector<ObjectRecord> objects;
    if (!getManagedObjects(&objects))
        return false;

    std::vector<std::string> includes;
    for (const ObjectRecord &record : objects)
    {
        if (record.path != servicePath)
            continue;
        ServiceInfo service;
        if (ServiceFromRecord(record, &service))
            includes = service.includes;
        break;
    }

    std::vector<ServiceInfo> owned;
    std::vector<ServiceInfo> &result = services != nullptr ? *services : owned;
    result.clear();
    for (const ObjectRecord &record : objects)
    {
        if (std::find(includes.begin(), includes.end(), record.path) == includes.end())
            continue;
        ServiceInfo service;
        if (!ServiceFromRecord(record, &service) || !HasUUID({service.UUID}, UUIDs))
            continue;
        result.push_back(service);
    }

    Event event;
    event.type = EventType::IncludedServicesDiscovered;
    event.path = servicePath;
    event.services = result;
    event.errorCode = 0;
    emit(event);
    return true;
}

bool Transport::discoverCharacteristics(const std::string &servicePath,
                                        const std::vector<std::string> &UUIDs,
                                        std::vector<CharacteristicInfo> *characteristics)
{
    std::vector<ObjectRecord> objects;
    if (!getManagedObjects(&objects))
        return false;

    std::vector<CharacteristicInfo> owned;
    std::vector<CharacteristicInfo> &result = characteristics != nullptr ? *characteristics : owned;
    result.clear();
    for (const ObjectRecord &record : objects)
    {
        if (!PathHasPrefix(record.path, servicePath))
            continue;
        CharacteristicInfo characteristic;
        if (!CharacteristicFromRecord(record, &characteristic) ||
            !HasUUID({characteristic.UUID}, UUIDs))
            continue;
        result.push_back(characteristic);
    }

    Event event;
    event.type = EventType::CharacteristicsDiscovered;
    event.path = servicePath;
    event.characteristics = result;
    event.errorCode = result.empty() ? 9 : 0;
    emit(event);
    return true;
}

bool Transport::discoverDescriptors(const std::string &characteristicPath,
                                    std::vector<DescriptorInfo> *descriptors)
{
    std::vector<ObjectRecord> objects;
    if (!getManagedObjects(&objects))
        return false;

    std::vector<DescriptorInfo> owned;
    std::vector<DescriptorInfo> &result = descriptors != nullptr ? *descriptors : owned;
    result.clear();
    for (const ObjectRecord &record : objects)
    {
        if (!PathHasPrefix(record.path, characteristicPath))
            continue;
        DescriptorInfo descriptor;
        if (DescriptorFromRecord(record, &descriptor))
            result.push_back(descriptor);
    }

    Event event;
    event.type = EventType::DescriptorsDiscovered;
    event.path = characteristicPath;
    event.descriptors = result;
    event.errorCode = result.empty() ? 9 : 0;
    emit(event);
    return true;
}

bool Transport::readAttribute(const std::string &path, const char *interface,
                              std::vector<uint8_t> *value, std::string *errorName)
{
    return call(
        path, interface, "ReadValue",
        [this](DBusMessageIter *iter) {
            DBusMessageIter options;
            if (api_->message_iter_open_container(iter, DBUS_TYPE_ARRAY, "{sv}", &options))
                api_->message_iter_close_container(iter, &options);
        },
        [this, value](DBusMessageIter *iter) { return ReadByteArray(api_, iter, value); },
        errorName);
}

bool Transport::writeAttribute(const std::string &path, const char *interface,
                               const std::vector<uint8_t> &value, const char *writeType,
                               std::string *errorName)
{
    return call(
        path, interface, "WriteValue",
        [this, &value, writeType](DBusMessageIter *iter) {
            AppendByteArray(api_, iter, value);
            DBusMessageIter options;
            if (!api_->message_iter_open_container(iter, DBUS_TYPE_ARRAY, "{sv}", &options))
                return;
            if (writeType != nullptr)
                AppendVariantString(api_, &options, "type", writeType);
            api_->message_iter_close_container(iter, &options);
        },
        nullptr, errorName);
}

bool Transport::readValue(const std::string &path, std::vector<uint8_t> *value,
                          std::string *errorName)
{
    const bool result = readAttribute(path, kCharacteristic, value, errorName);
    Event event;
    event.type = EventType::ValueUpdated;
    event.path = path;
    event.value = result ? *value : std::vector<uint8_t>();
    event.errorCode = result ? 0 : ErrorCodeForDBusName(errorName != nullptr ? *errorName : "");
    event.flag = true;
    emit(event);
    return result;
}

bool Transport::readDescriptorValue(const std::string &path, std::vector<uint8_t> *value,
                                    std::string *errorName)
{
    const bool result = readAttribute(path, kDescriptor, value, errorName);
    Event event;
    event.type = EventType::ValueUpdated;
    event.path = path;
    event.value = result ? *value : std::vector<uint8_t>();
    event.errorCode = result ? 0 : ErrorCodeForDBusName(errorName != nullptr ? *errorName : "");
    event.flag = true;
    emit(event);
    return result;
}

bool Transport::writeValue(const std::string &path, const std::vector<uint8_t> &value,
                           bool withResponse, std::string *errorName)
{
    const bool result = writeAttribute(path, kCharacteristic, value,
                                       withResponse ? "request" : "command", errorName);
    // The delegate is told about the outcome of a write only for a write with response;
    // a write without response has no confirmation to report.
    if (withResponse)
    {
        Event event;
        event.type = EventType::WriteCompleted;
        event.path = path;
        event.errorCode = result ? 0 : ErrorCodeForDBusName(errorName != nullptr ? *errorName : "");
        emit(event);
    }
    return result;
}

bool Transport::writeDescriptorValue(const std::string &path, const std::vector<uint8_t> &value,
                                     std::string *errorName)
{
    const bool result = writeAttribute(path, kDescriptor, value, nullptr, errorName);
    Event event;
    event.type = EventType::WriteCompleted;
    event.path = path;
    event.errorCode = result ? 0 : ErrorCodeForDBusName(errorName != nullptr ? *errorName : "");
    emit(event);
    return result;
}

bool Transport::setNotify(const std::string &path, bool enabled, std::string *errorName)
{
    const char *method = enabled ? "StartNotify" : "StopNotify";
    const bool result = call(path, kCharacteristic, method, nullptr, nullptr, errorName);
    if (result)
    {
        std::lock_guard<std::mutex> lock(mutex_);
        notifying_[path] = enabled;
    }
    // BlueZ reports a successful change through the Notifying property, which is what tells
    // the delegate; only a failure has to be reported from here, or the delegate would be
    // told twice.
    if (!result)
    {
        Event event;
        event.type = EventType::NotificationStateChanged;
        event.path = path;
        event.flag = notifying_.count(path) != 0 && notifying_[path];
        event.errorCode = ErrorCodeForDBusName(errorName != nullptr ? *errorName : "");
        emit(event);
    }
    return result;
}

bool Transport::readRSSI(const std::string &devicePath, int *RSSI)
{
    Value value;
    if (!getProperty(devicePath, kDevice, "RSSI", &value) || value.type != Value::Type::Int16)
        return false;
    if (RSSI != nullptr)
        *RSSI = value.int16Value;
    Event event;
    event.type = EventType::RSSIUpdated;
    event.path = devicePath;
    event.rssi = value.int16Value;
    emit(event);
    return true;
}

bool Transport::connectedDevices(const std::vector<std::string> &UUIDs,
                                 std::vector<DeviceInfo> *devices)
{
    std::vector<ObjectRecord> objects;
    if (!getManagedObjects(&objects))
        return false;
    devices->clear();
    for (const ObjectRecord &record : objects)
    {
        DeviceInfo device;
        if (!DeviceFromRecord(record, &device) || !HasUUID(device.UUIDs, UUIDs))
            continue;
        const auto it = record.interfaces.find(kDevice);
        if (it == record.interfaces.end())
            continue;
        const auto connected = it->second.find("Connected");
        if (connected == it->second.end() || connected->second.type != Value::Type::Boolean ||
            !connected->second.boolValue)
            continue;
        devices->push_back(device);
    }
    return true;
}
} // namespace cbbluez

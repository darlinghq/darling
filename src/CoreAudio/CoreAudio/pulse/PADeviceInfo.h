/*
This file is part of Darling.

Copyright (C) 2026 Lubos Dolezel

Darling is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

Darling is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with Darling.  If not, see <http://www.gnu.org/licenses/>.
*/

#ifndef PADEVICEINFO_H
#define PADEVICEINFO_H
#include <cstdint>
#include <string>
#include <vector>

// Pure mapping from what a PulseAudio server says about a sink or source to the vocabulary of
// the HAL (transport types, data sources). Independent of the CoreAudio and PulseAudio headers
// so that it can be unit tested on its own; the four character codes are the public values of
// kAudioDeviceTransportType* and of the data source codes of the audio families.

enum class PAPortKind
{
	Other, Speaker, Headphones, Headset, Line, Microphone, HDMI, SPDIF, Bluetooth, USB, Analog
};

enum class PAPortAvailability { Unknown, No, Yes };

struct PAPortDesc
{
	std::string name;
	std::string description;
	PAPortKind kind = PAPortKind::Other;
	PAPortAvailability available = PAPortAvailability::Unknown;
};

struct PATransportInput
{
	std::string name;          // sink or source name
	std::string driver;        // the server's driver of the device, e.g. "module-null-sink"
	std::string bus;           // device.bus
	std::string api;           // device.api
	std::string formFactor;    // device.form_factor
	bool hasCard = false;      // belongs to a sound card of the server
	const PAPortDesc* activePort = nullptr;
};

// The kAudioDeviceTransportType* value of the device.
uint32_t PATransportType(const PATransportInput& device);

// The data source code of a port, in the scope of an input or an output device.
uint32_t PADataSourceCode(const PAPortDesc& port, bool input);

// The data source code of each port, in order. Codes are unique: a port whose code is taken
// by an earlier port gets a code of its own.
std::vector<uint32_t> PADataSourceCodes(const std::vector<PAPortDesc>& ports, bool input);

// The sample rates a device offers: PulseAudio converts the rate of every stream to the rate of the
// device it connects to, so the usual rates are accepted besides the rate the device runs at and extraRate.
std::vector<double> PAAvailableSampleRates(double nativeRate, double extraRate);

#endif /* PADEVICEINFO_H */

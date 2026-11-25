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

#include "PADeviceInfo.h"
#include <algorithm>
#include <cctype>

static std::string lower(std::string s)
{
	std::transform(s.begin(), s.end(), s.begin(), [](unsigned char c) { return std::tolower(c); });
	return s;
}

static bool contains(const std::string& haystack, const char* needle)
{
	return lower(haystack).find(needle) != std::string::npos;
}

static uint32_t fourCC(char a, char b, char c, char d)
{
	return (uint32_t(uint8_t(a)) << 24) | (uint32_t(uint8_t(b)) << 16) | (uint32_t(uint8_t(c)) << 8) | uint8_t(d);
}

uint32_t PATransportType(const PATransportInput& device)
{
	const uint32_t builtIn = fourCC('b', 'l', 't', 'n'), pci = fourCC('p', 'c', 'i', ' '), usb = fourCC('u', 's', 'b', ' ');
	const uint32_t virt = fourCC('v', 'i', 'r', 't'), aggregate = fourCC('g', 'r', 'u', 'p');

	// A display connector says more than the bus the sound card sits on.
	if (const PAPortDesc* port = device.activePort)
	{
		const bool hdmi = port->kind == PAPortKind::HDMI || contains(port->name, "hdmi") || contains(port->description, "hdmi");
		const bool displayPort = contains(port->name, "displayport") || contains(port->description, "displayport");
		// Drivers label the connectors of a graphics card "HDMI / DisplayPort" whichever they are: that is HDMI.
		if (displayPort && !contains(port->description, "hdmi"))
			return fourCC('d', 'p', 'r', 't');
		if (hdmi || displayPort)
			return fourCC('h', 'd', 'm', 'i');
	}

	if (device.bus == "usb")
		return usb;
	if (device.bus == "bluetooth" || device.api == "bluez")
		return fourCC('b', 'l', 'u', 'e');
	if (device.bus == "firewire")
		return fourCC('1', '3', '9', '4');
	if (device.bus == "thunderbolt")
		return fourCC('t', 'h', 'u', 'n');
	if (device.bus == "avb")
		return fourCC('e', 'a', 'v', 'b');
	if (device.bus == "isa")
		return builtIn;
	if (device.bus == "pci")
	{
		// Sound hardware of the computer itself, with jacks or speakers, is what a Mac calls built in.
		// Anything else on the PCI bus is an add-in card.
		const std::string& form = device.formFactor;
		const bool internalForm = form == "internal" || form == "speaker" || form == "headphone" || form == "headset"
			|| form == "microphone" || form == "handset" || form == "hands-free" || form == "portable" || form == "computer";
		bool analogPort = false;
		if (device.activePort)
		{
			switch (device.activePort->kind)
			{
				case PAPortKind::Speaker: case PAPortKind::Headphones: case PAPortKind::Headset: case PAPortKind::Line:
				case PAPortKind::Microphone: case PAPortKind::Analog:
					analogPort = true;
					break;
				default:
					break;
			}
		}
		return (internalForm || analogPort) ? builtIn : pci;
	}

	if (contains(device.driver, "raop") || device.name.compare(0, 5, "raop_") == 0)
		return fourCC('a', 'i', 'r', 'p');
	if (contains(device.driver, "combine"))
		return aggregate;
	// A device that belongs to no sound card is made by software.
	return device.hasCard ? 0 : virt;
}

uint32_t PADataSourceCode(const PAPortDesc& port, bool input)
{
	switch (port.kind)
	{
		case PAPortKind::Speaker:
			return input ? 0 : fourCC('i', 's', 'p', 'k');
		case PAPortKind::Headphones:
			return input ? 0 : fourCC('h', 'd', 'p', 'n');
		case PAPortKind::Headset:
			return input ? fourCC('e', 'm', 'i', 'c') : fourCC('h', 'd', 'p', 'n');
		case PAPortKind::Line:
			return fourCC('l', 'i', 'n', 'e');
		case PAPortKind::Microphone:
			if (!input)
				return 0;
			return (contains(port.name, "internal") || contains(port.description, "internal")) ? fourCC('i', 'm', 'i', 'c')
				: fourCC('e', 'm', 'i', 'c');
		case PAPortKind::SPDIF:
			return fourCC('s', 'p', 'd', 'f');
		case PAPortKind::HDMI:
			return fourCC('h', 'd', 'm', 'i');
		default:
			return 0;
	}
}

std::vector<uint32_t> PADataSourceCodes(const std::vector<PAPortDesc>& ports, bool input)
{
	std::vector<uint32_t> codes;
	for (size_t i = 0; i < ports.size(); i++)
	{
		uint32_t code = PADataSourceCode(ports[i], input);
		if (code == 0 || std::find(codes.begin(), codes.end(), code) != codes.end())
		{
			// 'p' and 't' followed by the position of the port: stable for as long as the ports are.
			const char digits[] = "0123456789abcdefghijklmnopqrstuvwxyz";
			code = fourCC('p', 't', digits[(i / 36) % 36], digits[i % 36]);
		}
		codes.push_back(code);
	}
	return codes;
}

std::vector<double> PAAvailableSampleRates(double nativeRate, double extraRate)
{
	std::vector<double> rates = { 8000, 11025, 12000, 16000, 22050, 24000, 32000, 44100, 48000, 64000, 88200, 96000, 176400, 192000 };
	for (double extra : { nativeRate, extraRate })
	{
		if (extra > 0 && std::find(rates.begin(), rates.end(), extra) == rates.end())
			rates.push_back(extra);
	}
	std::sort(rates.begin(), rates.end());
	return rates;
}

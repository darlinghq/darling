// src/CoreAudio/CoreAudio/pulse/tests/PADeviceInfoTest.cpp
// Build and run on any host: g++ -std=c++17 PADeviceInfoTest.cpp ../PADeviceInfo.cpp && ./a.out
#include "../PADeviceInfo.h"
#include <algorithm>
#include <cassert>
#include <cstdio>

static uint32_t cc(const char* s)
{
	return (uint32_t(uint8_t(s[0])) << 24) | (uint32_t(uint8_t(s[1])) << 16) | (uint32_t(uint8_t(s[2])) << 8) | uint8_t(s[3]);
}

static PAPortDesc port(const char* name, const char* description, PAPortKind kind, PAPortAvailability available = PAPortAvailability::Yes)
{
	PAPortDesc p;
	p.name = name;
	p.description = description;
	p.kind = kind;
	p.available = available;
	return p;
}

int main()
{
	// The bus decides, for hardware.
	PATransportInput usb;
	usb.bus = "usb";
	usb.hasCard = true;
	assert(PATransportType(usb) == cc("usb "));

	PATransportInput bluetooth;
	bluetooth.bus = "bluetooth";
	assert(PATransportType(bluetooth) == cc("blue"));
	PATransportInput bluez;
	bluez.api = "bluez";
	assert(PATransportType(bluez) == cc("blue"));

	PATransportInput firewire;
	firewire.bus = "firewire";
	assert(PATransportType(firewire) == cc("1394"));

	// A sound chip of the computer is built in when it has speakers or jacks, a card on the PCI bus otherwise.
	PATransportInput pci;
	pci.bus = "pci";
	pci.hasCard = true;
	assert(PATransportType(pci) == cc("pci "));
	pci.formFactor = "internal";
	assert(PATransportType(pci) == cc("bltn"));
	pci.formFactor = "";
	PAPortDesc speaker = port("analog-output-speaker", "Speakers", PAPortKind::Speaker);
	pci.activePort = &speaker;
	assert(PATransportType(pci) == cc("bltn"));

	// A display connector wins over the bus.
	PAPortDesc hdmi = port("hdmi-output-0", "HDMI / DisplayPort", PAPortKind::HDMI);
	pci.activePort = &hdmi;
	assert(PATransportType(pci) == cc("hdmi"));
	PAPortDesc displayport = port("hdmi-output-1", "DisplayPort", PAPortKind::HDMI);
	pci.activePort = &displayport;
	assert(PATransportType(pci) == cc("dprt"));

	// Devices that no hardware stands behind.
	PATransportInput software;
	software.driver = "module-null-sink";
	assert(PATransportType(software) == cc("virt"));
	PATransportInput combined;
	combined.driver = "module-combine-sink";
	assert(PATransportType(combined) == cc("grup"));
	PATransportInput airplay;
	airplay.name = "raop_sink.living-room.local";
	airplay.driver = "module-raop-sink";
	assert(PATransportType(airplay) == cc("airp"));
	// A card whose bus is not known is not called virtual.
	PATransportInput unknown;
	unknown.hasCard = true;
	assert(PATransportType(unknown) == 0);

	// Data sources: by the kind of the port.
	assert(PADataSourceCode(port("p", "Speakers", PAPortKind::Speaker), false) == cc("ispk"));
	assert(PADataSourceCode(port("p", "Headphones", PAPortKind::Headphones), false) == cc("hdpn"));
	assert(PADataSourceCode(port("p", "Internal Microphone", PAPortKind::Microphone), true) == cc("imic"));
	assert(PADataSourceCode(port("analog-input-mic", "Microphone", PAPortKind::Microphone), true) == cc("emic"));
	assert(PADataSourceCode(port("p", "Line", PAPortKind::Line), true) == cc("line"));
	assert(PADataSourceCode(port("p", "Speakers", PAPortKind::Speaker), true) == 0);

	// Every port has a code of its own.
	std::vector<PAPortDesc> ports = {
		port("a", "Speakers", PAPortKind::Speaker),
		port("b", "Headphones", PAPortKind::Headphones),
		port("c", "Headset", PAPortKind::Headset),
		port("d", "Other", PAPortKind::Other),
	};
	std::vector<uint32_t> codes = PADataSourceCodes(ports, false);
	assert(codes.size() == 4);
	assert(codes[0] == cc("ispk") && codes[1] == cc("hdpn"));
	assert(codes[2] != codes[1] && codes[3] != codes[2]);
	std::sort(codes.begin(), codes.end());
	assert(std::unique(codes.begin(), codes.end()) == codes.end());
	// The same ports give the same codes.
	assert(PADataSourceCodes(ports, false) == PADataSourceCodes(ports, false));

	// Sample rates: ascending, with the rate of the device and the one that was asked for.
	std::vector<double> rates = PAAvailableSampleRates(48000, 0);
	assert(std::is_sorted(rates.begin(), rates.end()));
	assert(std::find(rates.begin(), rates.end(), 44100) != rates.end());
	assert(std::find(rates.begin(), rates.end(), 48000) != rates.end());
	assert(std::count(rates.begin(), rates.end(), 48000) == 1);
	rates = PAAvailableSampleRates(32000, 12345);
	assert(std::find(rates.begin(), rates.end(), 12345) != rates.end());
	assert(std::is_sorted(rates.begin(), rates.end()));

	printf("PADeviceInfoTest: OK\n");
	return 0;
}

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
#import "DPSound.h"
#import <AudioToolbox/AudioToolbox.h>
#import <CoreAudio/CoreAudio.h>
#import <math.h>

static const CGFloat kMargin = 16;
static const double kToneSeconds = 1.0;
static const double kSampleRate = 44100;

@interface DPSoundFlippedView : NSView
@end
@implementation DPSoundFlippedView
- (BOOL)isFlipped { return YES; }
@end

@implementation DPLevelView
@synthesize doubleValue = _doubleValue;

- (void)setDoubleValue:(double)value
{
    _doubleValue = MAX(0, MIN(1, value));
    [self setNeedsDisplay:YES];
}

- (void)drawRect:(NSRect)dirty
{
    NSRect bounds = [self bounds];
    [[NSColor whiteColor] setFill];
    NSRectFill(bounds);
    // a decibel scale from -60 dB to 0 dB
    double db = _doubleValue > 0 ? 20 * log10(_doubleValue) : -120;
    double fraction = MAX(0, MIN(1, (db + 60) / 60));
    NSRect bar = NSMakeRect(bounds.origin.x + 1, bounds.origin.y + 1, floor((bounds.size.width - 2) * fraction), bounds.size.height - 2);
    [(db > -3 ? [NSColor colorWithCalibratedRed:0.85 green:0.25 blue:0.2 alpha:1] : [NSColor colorWithCalibratedRed:0.25 green:0.7 blue:0.35 alpha:1]) setFill];
    NSRectFill(bar);
    [[NSColor grayColor] setStroke];
    [NSBezierPath strokeRect:NSInsetRect(bounds, 0.5, 0.5)];
}
@end

#pragma mark HAL helpers

static AudioObjectPropertyAddress Address(AudioObjectPropertySelector selector, AudioObjectPropertyScope scope)
{
    AudioObjectPropertyAddress address = { selector, scope, kAudioObjectPropertyElementMaster };
    return address;
}

static NSString *StringProperty(AudioObjectID object, AudioObjectPropertySelector selector)
{
    AudioObjectPropertyAddress address = Address(selector, kAudioObjectPropertyScopeGlobal);
    CFStringRef value = NULL;
    UInt32 size = sizeof(value);
    if (AudioObjectGetPropertyData(object, &address, 0, NULL, &size, &value) != noErr || value == NULL)
        return nil;
    return CFBridgingRelease(value);
}

static UInt32 ChannelCount(AudioObjectID device, AudioObjectPropertyScope scope)
{
    AudioObjectPropertyAddress address = Address(kAudioDevicePropertyStreamConfiguration, scope);
    UInt32 size = 0;
    if (AudioObjectGetPropertyDataSize(device, &address, 0, NULL, &size) != noErr || size == 0)
        return 0;
    AudioBufferList *list = malloc(size);
    UInt32 channels = 0;
    if (AudioObjectGetPropertyData(device, &address, 0, NULL, &size, list) == noErr)
        for (UInt32 i = 0; i < list->mNumberBuffers; i++)
            channels += list->mBuffers[i].mNumberChannels;
    free(list);
    return channels;
}

static NSString *TransportName(AudioObjectID device)
{
    AudioObjectPropertyAddress address = Address(kAudioDevicePropertyTransportType, kAudioObjectPropertyScopeGlobal);
    UInt32 type = 0, size = sizeof(type);
    if (AudioObjectGetPropertyData(device, &address, 0, NULL, &size, &type) != noErr)
        return @"";
    switch (type) {
    case kAudioDeviceTransportTypeBuiltIn: return NSLocalizedString(@"Built-in", nil);
    case kAudioDeviceTransportTypeUSB: return @"USB";
    case kAudioDeviceTransportTypeBluetooth:
    case kAudioDeviceTransportTypeBluetoothLE: return @"Bluetooth";
    case kAudioDeviceTransportTypeHDMI: return @"HDMI";
    case kAudioDeviceTransportTypeDisplayPort: return @"DisplayPort";
    case kAudioDeviceTransportTypeFireWire: return @"FireWire";
    case kAudioDeviceTransportTypePCI: return @"PCI";
    case kAudioDeviceTransportTypeAirPlay: return @"AirPlay";
    case kAudioDeviceTransportTypeVirtual: return NSLocalizedString(@"Virtual", nil);
    case kAudioDeviceTransportTypeAggregate: return NSLocalizedString(@"Aggregate", nil);
    case kAudioDeviceTransportTypeThunderbolt: return @"Thunderbolt";
    case kAudioDeviceTransportTypeAVB: return @"AVB";
    default: return @"";
    }
}

// The elements that carry a per-device property (volume or mute) on a side of a device: the master element if
// it has the property, else the channels that have it, else none.
static NSArray<NSNumber *> *ElementsWithProperty(AudioObjectID device, AudioObjectPropertySelector selector, AudioObjectPropertyScope scope)
{
    if (device == kAudioObjectUnknown)
        return @[];
    AudioObjectPropertyAddress address = { selector, scope, kAudioObjectPropertyElementMaster };
    if (AudioObjectHasProperty(device, &address))
        return @[ @(kAudioObjectPropertyElementMaster) ];
    NSMutableArray *elements = [NSMutableArray array];
    UInt32 channels = ChannelCount(device, scope);
    for (UInt32 channel = 1; channel <= channels; channel++) {
        address.mElement = channel;
        if (AudioObjectHasProperty(device, &address))
            [elements addObject:@(channel)];
    }
    return elements;
}

// The volume of a side of a device (the mean over its channels if there is no master volume) or NO if it has none.
static BOOL GetVolume(AudioObjectID device, AudioObjectPropertyScope scope, Float32 *volume)
{
    NSArray<NSNumber *> *elements = ElementsWithProperty(device, kAudioDevicePropertyVolumeScalar, scope);
    double sum = 0;
    NSUInteger count = 0;
    for (NSNumber *element in elements) {
        AudioObjectPropertyAddress address = { kAudioDevicePropertyVolumeScalar, scope, [element unsignedIntValue] };
        Float32 value = 0;
        UInt32 size = sizeof(value);
        if (AudioObjectGetPropertyData(device, &address, 0, NULL, &size, &value) == noErr) {
            sum += value;
            count++;
        }
    }
    if (count == 0)
        return NO;
    *volume = (Float32)(sum / count);
    return YES;
}

static void SetVolume(AudioObjectID device, AudioObjectPropertyScope scope, Float32 volume)
{
    for (NSNumber *element in ElementsWithProperty(device, kAudioDevicePropertyVolumeScalar, scope)) {
        AudioObjectPropertyAddress address = { kAudioDevicePropertyVolumeScalar, scope, [element unsignedIntValue] };
        AudioObjectSetPropertyData(device, &address, 0, NULL, sizeof(volume), &volume);
    }
}

// A side of a device is muted when all the elements that have a mute are.
static BOOL GetMute(AudioObjectID device, AudioObjectPropertyScope scope, BOOL *muted)
{
    NSArray<NSNumber *> *elements = ElementsWithProperty(device, kAudioDevicePropertyMute, scope);
    BOOL all = YES, any = NO;
    for (NSNumber *element in elements) {
        AudioObjectPropertyAddress address = { kAudioDevicePropertyMute, scope, [element unsignedIntValue] };
        UInt32 value = 0, size = sizeof(value);
        if (AudioObjectGetPropertyData(device, &address, 0, NULL, &size, &value) == noErr) {
            any = YES;
            all = all && value != 0;
        }
    }
    if (!any)
        return NO;
    *muted = all;
    return YES;
}

static void SetMute(AudioObjectID device, AudioObjectPropertyScope scope, BOOL muted)
{
    UInt32 value = muted ? 1 : 0;
    for (NSNumber *element in ElementsWithProperty(device, kAudioDevicePropertyMute, scope)) {
        AudioObjectPropertyAddress address = { kAudioDevicePropertyMute, scope, [element unsignedIntValue] };
        AudioObjectSetPropertyData(device, &address, 0, NULL, sizeof(value), &value);
    }
}

static Float64 NominalRate(AudioObjectID device)
{
    AudioObjectPropertyAddress address = Address(kAudioDevicePropertyNominalSampleRate, kAudioObjectPropertyScopeGlobal);
    Float64 rate = 0;
    UInt32 size = sizeof(rate);
    if (device == kAudioObjectUnknown || AudioObjectGetPropertyData(device, &address, 0, NULL, &size, &rate) != noErr)
        return 0;
    return rate;
}

// The rates a device can be set to, ascending. A range that is a single rate is that rate; a wider range stands for
// the usual rates inside it.
static NSArray<NSNumber *> *AvailableRates(AudioObjectID device)
{
    static const Float64 usual[] = { 8000, 11025, 16000, 22050, 32000, 44100, 48000, 88200, 96000, 176400, 192000, 352800, 384000 };
    NSMutableSet *rates = [NSMutableSet set];
    AudioObjectPropertyAddress address = Address(kAudioDevicePropertyAvailableNominalSampleRates, kAudioObjectPropertyScopeGlobal);
    UInt32 size = 0;
    if (device != kAudioObjectUnknown && AudioObjectGetPropertyDataSize(device, &address, 0, NULL, &size) == noErr && size >= sizeof(AudioValueRange)) {
        AudioValueRange *ranges = malloc(size);
        if (AudioObjectGetPropertyData(device, &address, 0, NULL, &size, ranges) == noErr) {
            for (UInt32 i = 0; i < size / sizeof(AudioValueRange); i++) {
                if (ranges[i].mMinimum == ranges[i].mMaximum)
                    [rates addObject:@(ranges[i].mMinimum)];
                else
                    for (size_t j = 0; j < sizeof(usual) / sizeof(usual[0]); j++)
                        if (usual[j] >= ranges[i].mMinimum && usual[j] <= ranges[i].mMaximum)
                            [rates addObject:@(usual[j])];
            }
        }
        free(ranges);
    }
    Float64 current = NominalRate(device);
    if (current > 0)
        [rates addObject:@(current)];
    return [[rates allObjects] sortedArrayUsingSelector:@selector(compare:)];
}

static NSString *RateTitle(Float64 rate)
{
    return [NSString stringWithFormat:@"%g Hz", rate];
}

static AudioObjectID DefaultDevice(BOOL input)
{
    AudioObjectPropertyAddress address = Address(input ? kAudioHardwarePropertyDefaultInputDevice : kAudioHardwarePropertyDefaultOutputDevice,
                                                 kAudioObjectPropertyScopeGlobal);
    AudioObjectID device = kAudioObjectUnknown;
    UInt32 size = sizeof(device);
    AudioObjectGetPropertyData(kAudioObjectSystemObject, &address, 0, NULL, &size, &device);
    return device;
}

// The listed devices of a direction. The objects that stand for "the default device" (their identifiers
// begin with PulseAudio:) are not devices a user chooses.
static NSArray<NSDictionary *> *ListDevices(BOOL input)
{
    AudioObjectPropertyAddress address = Address(kAudioHardwarePropertyDevices, kAudioObjectPropertyScopeGlobal);
    UInt32 size = 0;
    if (AudioObjectGetPropertyDataSize(kAudioObjectSystemObject, &address, 0, NULL, &size) != noErr || size == 0)
        return @[];
    AudioObjectID *ids = malloc(size);
    NSMutableArray *devices = [NSMutableArray array];
    if (AudioObjectGetPropertyData(kAudioObjectSystemObject, &address, 0, NULL, &size, ids) == noErr) {
        for (UInt32 i = 0; i < size / sizeof(AudioObjectID); i++) {
            AudioObjectID device = ids[i];
            UInt32 channels = ChannelCount(device, input ? kAudioObjectPropertyScopeInput : kAudioObjectPropertyScopeOutput);
            NSString *uid = StringProperty(device, kAudioDevicePropertyDeviceUID);
            if (channels == 0 || uid == nil || [uid hasPrefix:@"PulseAudio:"])
                continue;
            [devices addObject:@{
                @"id": @(device),
                @"uid": uid,
                @"name": StringProperty(device, kAudioObjectPropertyName) ?: uid,
                @"manufacturer": StringProperty(device, kAudioObjectPropertyManufacturer) ?: @"",
                @"channels": @(channels),
                @"rate": RateTitle(NominalRate(device)),
                @"transport": TransportName(device),
            }];
        }
    }
    free(ids);
    return devices;
}

// The controls of one direction: the volume, mute and sample rate of its default device.
@interface DPSoundDirection : NSObject
@property (nonatomic) BOOL input;
@property (nonatomic, strong) NSSlider *volume;
@property (nonatomic, strong) NSButton *mute;
@property (nonatomic, strong) NSPopUpButton *rate;
@property (nonatomic, strong) NSDate *userChange;   // when the user last moved the volume or mute (see -isHeld)
@property (nonatomic) AudioObjectID listenedDevice; // the device whose properties are listened to
- (AudioObjectPropertyScope)scope;
@end
@implementation DPSoundDirection
- (AudioObjectPropertyScope)scope { return _input ? kAudioObjectPropertyScopeInput : kAudioObjectPropertyScopeOutput; }
@end

// One registered block listener, kept to remove it again.
@interface DPSoundListener : NSObject
@property (nonatomic) AudioObjectID object;
@property (nonatomic) AudioObjectPropertyAddress address;
@property (nonatomic, copy) AudioObjectPropertyListenerBlock block;
@end
@implementation DPSoundListener
@end

#pragma mark Controller

@interface DPSoundController () {
    NSTableView *_outputTable, *_inputTable;
    NSArray<NSDictionary *> *_outputs, *_inputs;
    DPSoundDirection *_out, *_in;
    NSMutableArray<DPSoundListener *> *_listeners;
    NSButton *_toneButton, *_inputButton;
    NSTextField *_toneStatus, *_levelLabel;
    DPLevelView *_level;
    BOOL _active, _reloading;

    AudioQueueRef _toneQueue;
    AudioQueueRef _inputQueue;
    NSTimer *_meterTimer;
    double _peak;                 // written by the audio thread, read on the main thread
}
- (void)propertiesChanged;
- (void)notePeak:(double)peak;
@end

@implementation DPSoundController

static NSTextField *Label(NSString *text, NSRect frame, NSFont *font, NSColor *color, NSTextAlignment alignment)
{
    NSTextField *label = [[NSTextField alloc] initWithFrame:frame];
    [label setStringValue:text ?: @""];
    [label setBezeled:NO];
    [label setBordered:NO];
    [label setDrawsBackground:NO];
    [label setEditable:NO];
    [label setSelectable:NO];
    [label setFont:font];
    [label setTextColor:color];
    [label setAlignment:alignment];
    return label;
}

- (NSTableView *)addListWithKey:(NSString *)key toView:(NSView *)view y:(CGFloat)y width:(CGFloat)width
{
    NSTableView *table = [[NSTableView alloc] initWithFrame:NSMakeRect(0, 0, width, 100)];
    NSArray *columns = @[ @[ @"name", @"Name", @200 ], @[ @"manufacturer", @"Manufacturer", @110 ], @[ @"channels", @"Channels", @70 ],
                          @[ @"rate", @"Sample rate", @90 ], @[ @"transport", @"Transport", @90 ] ];
    for (NSArray *spec in columns) {
        NSTableColumn *column = [[NSTableColumn alloc] initWithIdentifier:spec[0]];
        [[column headerCell] setStringValue:NSLocalizedString(spec[1], nil)];
        [column setWidth:[spec[2] doubleValue]];
        [column setEditable:NO];
        [[column dataCell] setFont:[NSFont systemFontOfSize:[NSFont smallSystemFontSize]]];
        [[column dataCell] setWraps:NO];
        [[column dataCell] setLineBreakMode:NSLineBreakByTruncatingTail];
        [table addTableColumn:column];
    }
    [table setRowHeight:20];
    [table setAllowsEmptySelection:NO];
    [table setAllowsMultipleSelection:NO];
    [table setDataSource:self];
    [table setDelegate:self];
    [table setToolTip:key];
    NSScrollView *scroll = [[NSScrollView alloc] initWithFrame:NSMakeRect(kMargin, y, width, 118)];
    [scroll setDocumentView:table];
    [scroll setHasVerticalScroller:YES];
    [scroll setBorderType:NSBezelBorder];
    [view addSubview:scroll];
    return table;
}

// The volume slider, mute checkbox and sample rate popup of one direction, in a row at y.
- (DPSoundDirection *)addControlsOfInput:(BOOL)input toView:(NSView *)view y:(CGFloat)y
{
    NSFont *regular = [NSFont systemFontOfSize:[NSFont systemFontSize]];
    NSString *prefix = input ? @"SoundInput" : @"Sound";
    DPSoundDirection *direction = [[DPSoundDirection alloc] init];
    [direction setInput:input];
    [view addSubview:Label(NSLocalizedString(@"Volume:", nil), NSMakeRect(kMargin, y + 3, 60, 17), regular, [NSColor blackColor], NSTextAlignmentRight)];
    NSSlider *volume = [[NSSlider alloc] initWithFrame:NSMakeRect(kMargin + 68, y, 160, 22)];
    [volume setMinValue:0];
    [volume setMaxValue:1];
    [volume setTarget:self];
    [volume setAction:@selector(volumeChanged:)];
    [volume setToolTip:[prefix stringByAppendingString:@"Volume"]];
    [view addSubview:volume];
    NSButton *mute = [[NSButton alloc] initWithFrame:NSMakeRect(kMargin + 238, y, 70, 22)];
    [mute setButtonType:NSButtonTypeSwitch];
    [mute setTitle:NSLocalizedString(@"Mute", nil)];
    [mute setTarget:self];
    [mute setAction:@selector(muteChanged:)];
    [mute setToolTip:[prefix stringByAppendingString:@"Mute"]];
    [view addSubview:mute];
    [view addSubview:Label(NSLocalizedString(@"Sample rate:", nil), NSMakeRect(kMargin + 316, y + 3, 84, 17), regular, [NSColor blackColor], NSTextAlignmentRight)];
    NSPopUpButton *rate = [[NSPopUpButton alloc] initWithFrame:NSMakeRect(kMargin + 406, y - 2, 110, 26) pullsDown:NO];
    [rate setTarget:self];
    [rate setAction:@selector(rateChanged:)];
    [rate setToolTip:input ? @"SoundInputRate" : @"SoundOutputRate"];
    [view addSubview:rate];
    [direction setVolume:volume];
    [direction setMute:mute];
    [direction setRate:rate];
    return direction;
}

- (NSView *)viewWithWidth:(CGFloat)width
{
    NSView *view = [[DPSoundFlippedView alloc] initWithFrame:NSMakeRect(0, 0, width, 100)];
    NSFont *small = [NSFont systemFontOfSize:[NSFont smallSystemFontSize]];
    NSFont *bold = [NSFont boldSystemFontOfSize:[NSFont systemFontSize]];
    CGFloat listWidth = width - 2 * kMargin;
    CGFloat y = 12;

    NSTextField *note = Label(NSLocalizedString(@"The devices of the sound server. The selected row is the default device, which new applications use; applications that are running may keep the device they have.", nil),
                              NSMakeRect(kMargin, y, listWidth, 30), small, [NSColor grayColor], NSTextAlignmentLeft);
    [[note cell] setWraps:YES];
    [view addSubview:note];
    y += 38;

    [view addSubview:Label(NSLocalizedString(@"Output", nil), NSMakeRect(kMargin, y, 200, 18), bold, [NSColor blackColor], NSTextAlignmentLeft)];
    y += 22;
    _outputTable = [self addListWithKey:@"SoundOutputList" toView:view y:y width:listWidth];
    y += 126;

    _out = [self addControlsOfInput:NO toView:view y:y];
    y += 30;
    _toneButton = [[NSButton alloc] initWithFrame:NSMakeRect(kMargin, y - 2, 150, 26)];
    [_toneButton setBezelStyle:NSBezelStyleRounded];
    [_toneButton setTitle:NSLocalizedString(@"Play Test Tone", nil)];
    [_toneButton setTarget:self];
    [_toneButton setAction:@selector(playTone:)];
    [_toneButton setToolTip:@"SoundTestTone"];
    [view addSubview:_toneButton];
    _toneStatus = Label(NSLocalizedString(@"The tone plays on the left channel, then on the right one, on the selected output.", nil),
                        NSMakeRect(kMargin + 160, y + 2, listWidth - 160, 32), small, [NSColor grayColor], NSTextAlignmentLeft);
    [[_toneStatus cell] setWraps:YES];
    [_toneStatus setToolTip:@"SoundToneStatus"];
    [view addSubview:_toneStatus];
    y += 36;

    [view addSubview:Label(NSLocalizedString(@"Input", nil), NSMakeRect(kMargin, y, 200, 18), bold, [NSColor blackColor], NSTextAlignmentLeft)];
    y += 22;
    _inputTable = [self addListWithKey:@"SoundInputList" toView:view y:y width:listWidth];
    y += 126;

    _in = [self addControlsOfInput:YES toView:view y:y];
    y += 30;

    _inputButton = [[NSButton alloc] initWithFrame:NSMakeRect(kMargin, y - 2, 90, 26)];
    [_inputButton setButtonType:NSButtonTypePushOnPushOff];
    [_inputButton setBezelStyle:NSBezelStyleRounded];
    [_inputButton setTitle:NSLocalizedString(@"Test", nil)];
    [_inputButton setTarget:self];
    [_inputButton setAction:@selector(toggleInputTest:)];
    [_inputButton setToolTip:@"SoundInputTest"];
    [view addSubview:_inputButton];
    _level = [[DPLevelView alloc] initWithFrame:NSMakeRect(kMargin + 104, y + 4, 330, 14)];
    [_level setToolTip:@"SoundInputLevel"];
    [view addSubview:_level];
    _levelLabel = Label(@"", NSMakeRect(kMargin + 444, y + 2, 120, 17), small, [NSColor grayColor], NSTextAlignmentLeft);
    [view addSubview:_levelLabel];
    y += 30;
    NSTextField *hint = Label(NSLocalizedString(@"Switch Test on and speak: the bar shows the level of the selected input.", nil),
                              NSMakeRect(kMargin, y, listWidth, 16), small, [NSColor grayColor], NSTextAlignmentLeft);
    [view addSubview:hint];
    y += 28;

    [view setFrameSize:NSMakeSize(width, y)];
    [self reload];
    return view;
}

#pragma mark Lists

- (void)selectRowOfDevice:(AudioObjectID)device inTable:(NSTableView *)table devices:(NSArray<NSDictionary *> *)devices
{
    for (NSUInteger row = 0; row < [devices count]; row++) {
        if ([devices[row][@"id"] unsignedIntValue] == device) {
            [table selectRowIndexes:[NSIndexSet indexSetWithIndex:row] byExtendingSelection:NO];
            return;
        }
    }
    [table deselectAll:nil];
}

- (void)reload
{
    _reloading = YES;
    _outputs = ListDevices(NO);
    _inputs = ListDevices(YES);
    [_outputTable reloadData];
    [_inputTable reloadData];
    [self selectRowOfDevice:DefaultDevice(NO) inTable:_outputTable devices:_outputs];
    [self selectRowOfDevice:DefaultDevice(YES) inTable:_inputTable devices:_inputs];
    _reloading = NO;
    [self refreshVolume];
}

- (NSInteger)numberOfRowsInTableView:(NSTableView *)table
{
    return (NSInteger)[(table == _inputTable ? _inputs : _outputs) count];
}

- (id)tableView:(NSTableView *)table objectValueForTableColumn:(NSTableColumn *)column row:(NSInteger)row
{
    NSArray *devices = table == _inputTable ? _inputs : _outputs;
    id value = row < (NSInteger)[devices count] ? devices[row][[column identifier]] : nil;
    return [value isKindOfClass:[NSNumber class]] ? [value stringValue] : value;
}

- (BOOL)tableView:(NSTableView *)table shouldEditTableColumn:(NSTableColumn *)column row:(NSInteger)row
{
    return NO;
}

// Choosing a row makes the device the default one.
- (void)tableViewSelectionDidChange:(NSNotification *)notification
{
    NSTableView *table = [notification object];
    if (_reloading || (table != _outputTable && table != _inputTable))
        return;
    BOOL input = table == _inputTable;
    NSArray *devices = input ? _inputs : _outputs;
    NSInteger row = [table selectedRow];
    if (row < 0 || row >= (NSInteger)[devices count])
        return;
    AudioObjectID device = [devices[row][@"id"] unsignedIntValue];
    if (device == DefaultDevice(input))
        return;
    if (input)
        [self stopInputTest];
    AudioObjectPropertyAddress address = Address(input ? kAudioHardwarePropertyDefaultInputDevice : kAudioHardwarePropertyDefaultOutputDevice,
                                                 kAudioObjectPropertyScopeGlobal);
    OSStatus status = AudioObjectSetPropertyData(kAudioObjectSystemObject, &address, 0, NULL, sizeof(device), &device);
    if (status != noErr) {
        _reloading = YES;
        [self selectRowOfDevice:DefaultDevice(input) inTable:table devices:devices];
        _reloading = NO;
    }
    [self refreshVolume];
}

#pragma mark Volume, mute and sample rate

- (DPSoundDirection *)directionOfSender:(id)sender
{
    return (sender == [_in volume] || sender == [_in mute] || sender == [_in rate]) ? _in : _out;
}

// The user moved a control: until the server has answered, what it still reports must not move the control back.
static const NSTimeInterval kHold = 0.4;

- (BOOL)isHeld:(DPSoundDirection *)direction
{
    return [direction userChange] != nil && -[[direction userChange] timeIntervalSinceNow] < kHold;
}

// The controls belong to the default device of their direction, which is the selected row.
- (void)refreshControlsOfDirection:(DPSoundDirection *)direction
{
    AudioObjectID device = DefaultDevice([direction input]);
    AudioObjectPropertyScope scope = [direction scope];
    Float32 volume = 0;
    BOOL muted = NO;
    BOOL hasVolume = GetVolume(device, scope, &volume), hasMute = GetMute(device, scope, &muted);
    [[direction volume] setEnabled:hasVolume];
    [[direction mute] setEnabled:hasMute];
    if (hasVolume && ![self isHeld:direction])
        [[direction volume] setDoubleValue:volume];
    if (hasMute && ![self isHeld:direction])
        [[direction mute] setState:muted ? NSControlStateValueOn : NSControlStateValueOff];
    if (!hasVolume)
        [[direction volume] setDoubleValue:0];
    if (!hasMute)
        [[direction mute] setState:NSControlStateValueOff];

    NSPopUpButton *popup = [direction rate];
    NSArray<NSNumber *> *rates = AvailableRates(device);
    Float64 current = NominalRate(device);
    NSMutableArray *shown = [NSMutableArray array];
    for (NSMenuItem *item in [popup itemArray])
        [shown addObject:[item representedObject]];
    if (![shown isEqualToArray:rates]) {
        [popup removeAllItems];
        for (NSNumber *rate in rates) {
            [popup addItemWithTitle:RateTitle([rate doubleValue])];
            [[popup lastItem] setRepresentedObject:rate];
        }
    }
    for (NSInteger i = 0; i < [popup numberOfItems]; i++)
        if ([[[popup itemAtIndex:i] representedObject] doubleValue] == current)
            [popup selectItemAtIndex:i];
    AudioObjectPropertyAddress rateAddress = Address(kAudioDevicePropertyNominalSampleRate, kAudioObjectPropertyScopeGlobal);
    Boolean settable = false;
    BOOL canSet = device != kAudioObjectUnknown && AudioObjectIsPropertySettable(device, &rateAddress, &settable) == noErr && settable;
    [popup setEnabled:canSet && [rates count] > 1];
}

- (void)refreshVolume
{
    [self refreshControlsOfDirection:_out];
    [self refreshControlsOfDirection:_in];
}

- (void)volumeChanged:(id)sender
{
    DPSoundDirection *direction = [self directionOfSender:sender];
    [direction setUserChange:[NSDate date]];
    SetVolume(DefaultDevice([direction input]), [direction scope], (Float32)[[direction volume] doubleValue]);
}

- (void)muteChanged:(id)sender
{
    DPSoundDirection *direction = [self directionOfSender:sender];
    [direction setUserChange:[NSDate date]];
    SetMute(DefaultDevice([direction input]), [direction scope], [[direction mute] state] == NSControlStateValueOn);
}

// Choosing a rate asks the device for it; the popup shows what the device then reports.
- (void)rateChanged:(id)sender
{
    DPSoundDirection *direction = [self directionOfSender:sender];
    AudioObjectID device = DefaultDevice([direction input]);
    NSNumber *rate = [[[direction rate] selectedItem] representedObject];
    if (rate != nil && device != kAudioObjectUnknown) {
        if ([direction input])
            [self stopInputTest];
        AudioObjectPropertyAddress address = Address(kAudioDevicePropertyNominalSampleRate, kAudioObjectPropertyScopeGlobal);
        Float64 value = [rate doubleValue];
        AudioObjectSetPropertyData(device, &address, 0, NULL, sizeof(value), &value);
    }
    [self reload];
}

#pragma mark Output test

static void ToneFinished(void *context, AudioQueueRef queue, AudioQueueBufferRef buffer)
{
    DPSoundController *controller = (__bridge DPSoundController *)context;
    dispatch_async(dispatch_get_main_queue(), ^{ [controller toneFinished:queue]; });
}

- (void)toneFinished:(AudioQueueRef)queue
{
    if (queue != _toneQueue)
        return;
    _toneQueue = NULL;
    AudioQueueStop(queue, true);
    AudioQueueDispose(queue, true);
    [_toneButton setEnabled:YES];
    [_toneStatus setStringValue:NSLocalizedString(@"The tone plays on the left channel, then on the right one, on the selected output.", nil)];
}

- (void)playTone:(id)sender
{
    if (_toneQueue != NULL)
        return;
    NSInteger row = [_outputTable selectedRow];
    if (row < 0 || row >= (NSInteger)[_outputs count])
        return;
    AudioStreamBasicDescription format = { kSampleRate, kAudioFormatLinearPCM, kAudioFormatFlagIsFloat | kAudioFormatFlagIsPacked, 8, 1, 8, 2, 32, 0 };
    AudioQueueRef queue = NULL;
    if (AudioQueueNewOutput(&format, ToneFinished, (__bridge void *)self, NULL, NULL, 0, &queue) != noErr) {
        [_toneStatus setStringValue:NSLocalizedString(@"The output could not be opened.", nil)];
        return;
    }
    CFStringRef uid = (__bridge CFStringRef)_outputs[row][@"uid"];
    AudioQueueSetProperty(queue, kAudioQueueProperty_CurrentDevice, &uid, sizeof(uid));

    UInt32 frames = (UInt32)(kSampleRate * kToneSeconds);
    AudioQueueBufferRef buffer = NULL;
    if (AudioQueueAllocateBuffer(queue, frames * 8, &buffer) != noErr) {
        AudioQueueDispose(queue, true);
        return;
    }
    float *samples = buffer->mAudioData;
    UInt32 half = frames / 2, fade = (UInt32)(kSampleRate * 0.01);
    for (UInt32 i = 0; i < frames; i++) {
        UInt32 position = i < half ? i : i - half;
        double envelope = MIN(1.0, MIN((double)position / fade, (double)(half - position) / fade));
        float value = (float)(0.3 * envelope * sin(2 * M_PI * 440 * (double)position / kSampleRate));
        samples[2 * i] = i < half ? value : 0;            // the left channel first,
        samples[2 * i + 1] = i < half ? 0 : value;        // then the right one
    }
    buffer->mAudioDataByteSize = frames * 8;
    AudioQueueEnqueueBuffer(queue, buffer, 0, NULL);
    _toneQueue = queue;
    [_toneButton setEnabled:NO];
    [_toneStatus setStringValue:NSLocalizedString(@"Playing the test tone…", nil)];
    if (AudioQueueStart(queue, NULL) != noErr)
        [self toneFinished:queue];
    else
        // the queue is drained after the buffer, then stopped
        AudioQueueStop(queue, false);
}

#pragma mark Input test

static void InputBuffer(void *context, AudioQueueRef queue, AudioQueueBufferRef buffer, const AudioTimeStamp *time,
                        UInt32 packets, const AudioStreamPacketDescription *descriptions)
{
    DPSoundController *controller = (__bridge DPSoundController *)context;
    const float *samples = buffer->mAudioData;
    UInt32 count = buffer->mAudioDataByteSize / sizeof(float);
    double peak = 0;
    for (UInt32 i = 0; i < count; i++)
        peak = MAX(peak, fabs(samples[i]));
    [controller notePeak:peak];
    AudioQueueEnqueueBuffer(queue, buffer, 0, NULL);
}

- (void)toggleInputTest:(id)sender
{
    if ([_inputButton state] == NSControlStateValueOn)
        [self startInputTest];
    else
        [self stopInputTest];
}

- (void)startInputTest
{
    NSInteger row = [_inputTable selectedRow];
    if (_inputQueue != NULL || row < 0 || row >= (NSInteger)[_inputs count]) {
        [_inputButton setState:NSControlStateValueOff];
        return;
    }
    AudioStreamBasicDescription format = { kSampleRate, kAudioFormatLinearPCM, kAudioFormatFlagIsFloat | kAudioFormatFlagIsPacked, 4, 1, 4, 1, 32, 0 };
    AudioQueueRef queue = NULL;
    if (AudioQueueNewInput(&format, InputBuffer, (__bridge void *)self, NULL, NULL, 0, &queue) != noErr) {
        [_inputButton setState:NSControlStateValueOff];
        [_levelLabel setStringValue:NSLocalizedString(@"Cannot open the input", nil)];
        return;
    }
    CFStringRef uid = (__bridge CFStringRef)_inputs[row][@"uid"];
    AudioQueueSetProperty(queue, kAudioQueueProperty_CurrentDevice, &uid, sizeof(uid));
    for (int i = 0; i < 3; i++) {
        AudioQueueBufferRef buffer = NULL;
        if (AudioQueueAllocateBuffer(queue, 2048 * 4, &buffer) == noErr)
            AudioQueueEnqueueBuffer(queue, buffer, 0, NULL);
    }
    _peak = 0;
    if (AudioQueueStart(queue, NULL) != noErr) {
        AudioQueueDispose(queue, true);
        [_inputButton setState:NSControlStateValueOff];
        [_levelLabel setStringValue:NSLocalizedString(@"Cannot start the input", nil)];
        return;
    }
    _inputQueue = queue;
    _meterTimer = [NSTimer scheduledTimerWithTimeInterval:0.05 target:self selector:@selector(updateMeter:) userInfo:nil repeats:YES];
}

// the meter takes the highest peak since it last looked
- (void)notePeak:(double)peak
{
    @synchronized (self) {
        _peak = MAX(_peak, peak);
    }
}

- (void)updateMeter:(NSTimer *)timer
{
    double peak;
    @synchronized (self) {
        peak = _peak;
        _peak = 0;
    }
    // the bar falls back slowly
    double shown = MAX(peak, [_level doubleValue] * 0.85);
    [_level setDoubleValue:shown];
    [_levelLabel setStringValue:shown > 0.00002 ? [NSString stringWithFormat:@"%.0f dB", 20 * log10(shown)] : @"−∞ dB"];
}

- (void)stopInputTest
{
    [_meterTimer invalidate];
    _meterTimer = nil;
    if (_inputQueue != NULL) {
        AudioQueueRef queue = _inputQueue;
        _inputQueue = NULL;
        AudioQueueStop(queue, true);
        AudioQueueDispose(queue, true);
    }
    [_inputButton setState:NSControlStateValueOff];
    [_level setDoubleValue:0];
    [_levelLabel setStringValue:@""];
}

- (void)stopTests
{
    [self stopInputTest];
    if (_toneQueue != NULL)
        [self toneFinished:_toneQueue];
}

#pragma mark Listeners

- (void)addListener:(AudioObjectID)object selector:(AudioObjectPropertySelector)selector scope:(AudioObjectPropertyScope)scope element:(AudioObjectPropertyElement)element
{
    DPSoundListener *listener = [[DPSoundListener alloc] init];
    AudioObjectPropertyAddress address = { selector, scope, element };
    __weak DPSoundController *weak = self;
    [listener setObject:object];
    [listener setAddress:address];
    [listener setBlock:^(UInt32 count, const AudioObjectPropertyAddress *addresses) {
        [weak propertiesChanged];
    }];
    if (AudioObjectAddPropertyListenerBlock(object, &address, dispatch_get_main_queue(), [listener block]) == noErr)
        [_listeners addObject:listener];
}

// The system object tells when the devices or the defaults change; the default devices tell when their volume,
// mute or sample rate change. The latter are listened to again whenever a default device changes.
- (void)listenToDevice:(DPSoundDirection *)direction
{
    AudioObjectID device = DefaultDevice([direction input]);
    AudioObjectPropertyScope scope = [direction scope];
    [direction setListenedDevice:device];
    if (device == kAudioObjectUnknown)
        return;
    [self addListener:device selector:kAudioDevicePropertyNominalSampleRate scope:kAudioObjectPropertyScopeGlobal element:kAudioObjectPropertyElementMaster];
    for (NSNumber *selector in @[ @(kAudioDevicePropertyVolumeScalar), @(kAudioDevicePropertyMute) ])
        for (NSNumber *element in [@[ @(kAudioObjectPropertyElementMaster) ] arrayByAddingObjectsFromArray:ElementsWithProperty(device, [selector unsignedIntValue], scope)])
            [self addListener:device selector:[selector unsignedIntValue] scope:scope element:[element unsignedIntValue]];
}

- (void)removeListeners:(BOOL)all
{
    NSMutableArray *kept = [NSMutableArray array];
    for (DPSoundListener *listener in _listeners) {
        if (!all && [listener object] == kAudioObjectSystemObject) {
            [kept addObject:listener];
            continue;
        }
        AudioObjectPropertyAddress address = [listener address];
        AudioObjectRemovePropertyListenerBlock([listener object], &address, dispatch_get_main_queue(), [listener block]);
    }
    _listeners = kept;
}

- (void)propertiesChanged
{
    if (!_active)
        return;
    if (DefaultDevice(NO) != [_out listenedDevice] || DefaultDevice(YES) != [_in listenedDevice]) {
        [self removeListeners:NO];
        [self listenToDevice:_out];
        [self listenToDevice:_in];
    }
    [self reload];
}

- (void)activate
{
    if (_active)
        return;
    _active = YES;
    _listeners = [NSMutableArray array];
    for (NSNumber *selector in @[ @(kAudioHardwarePropertyDevices), @(kAudioHardwarePropertyDefaultOutputDevice), @(kAudioHardwarePropertyDefaultInputDevice) ])
        [self addListener:kAudioObjectSystemObject selector:[selector unsignedIntValue] scope:kAudioObjectPropertyScopeGlobal element:kAudioObjectPropertyElementMaster];
    [self listenToDevice:_out];
    [self listenToDevice:_in];
    [self reload];
}

- (void)deactivate
{
    if (!_active)
        return;
    _active = NO;
    [self removeListeners:YES];
    [self stopTests];
}

@end

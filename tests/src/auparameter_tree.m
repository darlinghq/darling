/*
 * AUParameterTree, AUParameterGroup and AUParameter (AudioToolbox, macOS 10.11+).
 *
 * Checks the documented behaviour of the parameter tree an audio unit
 * publishes: the factory methods, key paths and key-value coding, lookup by
 * address, the value property with its implementor hooks and observers,
 * string conversion and secure coding. The values asserted were observed on
 * macOS 26 (value is not clamped, observer delivery order, error
 * exceptions). Exit status is 0 when every check passed.
 *
 * Copyright (C) 2026 Lubos Dolezel
 */
#import <AudioToolbox/AudioToolbox.h>
#import <AudioToolbox/AUAudioUnitImplementation.h>
#import <Foundation/Foundation.h>
#include <stdio.h>
#include <unistd.h>

static int failures;

#define CHECK(cond) do { \
	if (cond) printf("PASS %s\n", #cond); \
	else { printf("FAIL %s\n", #cond); failures++; } \
} while (0)

static AUParameter *makeParam(NSString *ident, AUParameterAddress address, float min, float max,
	AudioUnitParameterOptions flags, NSArray *strings)
{
	return [AUParameterTree createParameterWithIdentifier:ident name:[ident uppercaseString]
		address:address min:min max:max unit:kAudioUnitParameterUnit_Hertz unitName:@"Hz"
		flags:flags valueStrings:strings dependentParameters:nil];
}

static void waitFor(volatile int *flag, int expected)
{
	for (int i = 0; i < 200 && *flag < expected; i++)
		usleep(10000);
}

int main(void)
{
	setvbuf(stdout, NULL, _IONBF, 0);
	@autoreleasepool {
		AUParameter *vol = makeParam(@"volume", 1, 0, 10, 0, nil);
		AUParameter *rate = makeParam(@"rate", 2, -5, 5, 0, nil);
		AUParameter *mode = makeParam(@"mode", 3, 0, 2, kAudioUnitParameterFlag_ValuesHaveStrings,
			@[@"off", @"on", @"auto"]);
		AUParameterGroup *lfo = [AUParameterTree createGroupWithIdentifier:@"lfo" name:@"LFO"
			children:@[rate, mode]];
		AUParameterTree *tree = [AUParameterTree createTreeWithChildren:@[vol, lfo]];

		// Structure
		CHECK([tree isKindOfClass:[AUParameterGroup class]]);
		CHECK([tree.identifier isEqualToString:@""] && [tree.keyPath isEqualToString:@""]);
		CHECK([lfo.keyPath isEqualToString:@"lfo"] && [lfo.displayName isEqualToString:@"LFO"]);
		CHECK([vol.keyPath isEqualToString:@"volume"]);
		CHECK([rate.keyPath isEqualToString:@"lfo.rate"]);
		CHECK(tree.children.count == 2 && lfo.children.count == 2);
		CHECK(tree.allParameters.count == 3);
		CHECK([[tree.allParameters valueForKey:@"identifier"] isEqual:(@[@"volume", @"rate", @"mode"])]);
		CHECK(vol.minValue == 0 && vol.maxValue == 10 && vol.address == 1);
		CHECK(vol.unit == kAudioUnitParameterUnit_Hertz && [vol.unitName isEqualToString:@"Hz"]);
		CHECK(vol.valueStrings == nil && [mode.valueStrings isEqual:(@[@"off", @"on", @"auto"])]);
		CHECK(vol.value == 0);

		// Lookup and key-value coding
		CHECK([tree parameterWithAddress:2] == rate);
		CHECK([tree parameterWithAddress:99] == nil);
		CHECK([tree parameterWithID:2 scope:0 element:0] == nil);
		CHECK([tree valueForKeyPath:@"lfo.rate"] == rate);
		CHECK([tree valueForKey:@"lfo"] == lfo);
		CHECK([tree valueForKey:@"volume"] == vol);
		CHECK([lfo valueForKey:@"identifier"] == nil);
		CHECK([tree valueForKey:@"nosuch"] == nil);
		CHECK([[tree valueForKeyPath:@"lfo.rate.value"] floatValue] == 0);
		[tree setValue:@3 forKeyPath:@"lfo.rate.value"];
		CHECK(rate.value == 3);
		rate.value = 0;

		// The value is not limited to the parameter's range
		vol.value = 20;
		CHECK(vol.value == 20);
		vol.value = -3;
		CHECK(vol.value == -3);
		vol.value = 7.5f;
		CHECK(vol.value == 7.5f);

		// Value hooks and observers: the audio unit's observer runs on the caller's thread before
		// the setter returns, the others are called afterwards, nearest node first.
		__block volatile int seq = 0;
		__block int implementorSeq = 0, paramSeq = 0, groupSeq = 0, treeSeq = 0;
		__block float lastValue = -1;
		__block AUParameterAddress lastAddress = 99;
		vol.implementorValueObserver = ^(AUParameter *p, AUValue v) { implementorSeq = ++seq; };
		AUParameterObserverToken tokP = [vol tokenByAddingParameterObserver:^(AUParameterAddress a, AUValue v) {
			lastAddress = a; lastValue = v; paramSeq = ++seq;
		}];
		AUParameterObserverToken tokT = [tree tokenByAddingParameterObserver:^(AUParameterAddress a, AUValue v) {
			treeSeq = ++seq;
		}];
		[lfo tokenByAddingParameterObserver:^(AUParameterAddress a, AUValue v) { groupSeq = ++seq; }];
		CHECK(tokP != NULL && tokT != NULL && tokP != tokT);

		vol.value = 3;
		CHECK(implementorSeq == 1);
		waitFor(&seq, 3);
		CHECK(paramSeq == 2 && treeSeq == 3 && groupSeq == 0);
		CHECK(lastAddress == 1 && lastValue == 3);

		// Observers of other parameters in the group see this group's changes
		rate.value = 1;
		waitFor(&seq, 5);
		CHECK(groupSeq == 4 && treeSeq == 5);

		// The originator is not notified, the others are
		seq = 0; implementorSeq = paramSeq = groupSeq = treeSeq = 0;
		[vol setValue:4 originator:tokP];
		waitFor(&seq, 2);
		CHECK(implementorSeq == 1 && paramSeq == 0 && treeSeq == 2);
		CHECK(vol.value == 4);

		// A removed observer is no longer called
		[vol removeParameterObserver:tokP];
		seq = 0; implementorSeq = paramSeq = groupSeq = treeSeq = 0;
		vol.value = 2;
		waitFor(&seq, 2);
		CHECK(paramSeq == 0 && treeSeq == 2);

		// Automation and recording observers receive the host time and the event type
		__block volatile int autoCount = 0, recCount = 0;
		__block AUParameterAutomationEvent lastAuto;
		__block AURecordedParameterEvent lastRec;
		[tree tokenByAddingParameterAutomationObserver:^(NSInteger n, const AUParameterAutomationEvent *e) {
			lastAuto = e[0]; autoCount += (int)n;
		}];
		[tree tokenByAddingParameterRecordingObserver:^(NSInteger n, const AURecordedParameterEvent *e) {
			lastRec = e[0]; recCount += (int)n;
		}];
		[vol setValue:6 originator:NULL atHostTime:1234 eventType:AUParameterAutomationEventTypeTouch];
		waitFor(&autoCount, 1);
		CHECK(autoCount == 1 && lastAuto.eventType == AUParameterAutomationEventTypeTouch
			&& lastAuto.hostTime == 1234 && lastAuto.address == 1 && lastAuto.value == 6);
		[vol setValue:8 originator:NULL atHostTime:1237];
		waitFor(&autoCount, 2);
		waitFor(&recCount, 1);
		CHECK(lastAuto.eventType == AUParameterAutomationEventTypeValue && lastAuto.hostTime == 1237);
		CHECK(recCount == 2 && lastRec.hostTime == 1237 && lastRec.address == 1 && lastRec.value == 8);
		[vol setValue:9 originator:NULL atHostTime:0];
		waitFor(&autoCount, 3);
		CHECK(lastAuto.hostTime != 0);

		// Value provider: asked for the value after a change; later reads use the stored value
		__block int providerCalls = 0;
		vol.implementorValueProvider = ^AUValue(AUParameter *p) { providerCalls++; return 8; };
		vol.value = 4;
		CHECK(providerCalls == 1 && vol.value == 8 && providerCalls == 1);
		vol.implementorValueProvider = nil;
		vol.value = 2.5f;
		CHECK(vol.value == 2.5f);

		// A hook set on the tree applies to the nodes below that have none of their own
		__block int treeHookValues = 0, groupHookValues = 0;
		tree.implementorValueObserver = ^(AUParameter *p, AUValue v) { treeHookValues++; };
		lfo.implementorValueObserver = ^(AUParameter *p, AUValue v) { groupHookValues++; };
		mode.value = 1;
		rate.value = 1;
		vol.implementorValueObserver = nil;
		vol.value = 1;
		CHECK(groupHookValues == 2 && treeHookValues == 1);
		tree.implementorValueObserver = nil;
		lfo.implementorValueObserver = nil;

		// Strings
		mode.value = 1;
		AUValue two = 1.7f, big = 1e10f;
		CHECK([[mode stringFromValue:NULL] isEqualToString:@"1"]);
		CHECK([[mode stringFromValue:&two] isEqualToString:@"1.7"]);
		CHECK([[vol stringFromValue:&big] isEqualToString:@"1e+10"]);
		CHECK([mode valueFromString:@"on"] == 0 && [vol valueFromString:@"abc"] == 0);
		vol.implementorStringFromValueCallback = ^NSString *(AUParameter *p, const AUValue *v) {
			return [NSString stringWithFormat:@"v%g", v ? *v : -1.0];
		};
		vol.implementorValueFromStringCallback = ^AUValue(AUParameter *p, NSString *s) { return 42; };
		CHECK([[vol stringFromValue:NULL] isEqualToString:@"v-1"] && [vol valueFromString:@"x"] == 42);
		CHECK([[vol displayNameWithLength:3] isEqualToString:@"VOLUME"]);
		tree.implementorDisplayNameWithLengthCallback = ^NSString *(AUParameterNode *n, NSInteger l) {
			return [NSString stringWithFormat:@"%@%ld", n.identifier, (long)l];
		};
		CHECK([[rate displayNameWithLength:3] isEqualToString:@"rate3"]);
		CHECK([rate.displayName isEqualToString:@"RATE"]);

		// Templates
		AUParameterGroup *tpl = [AUParameterTree createGroupTemplate:@[makeParam(@"a", 0, 0, 1, 0, nil),
			makeParam(@"b", 1, 0, 1, 0, nil)]];
		CHECK(tpl.children.count == 2 && [tpl.identifier isEqualToString:@""]);
		AUParameterGroup *band1 = [AUParameterTree createGroupFromTemplate:tpl identifier:@"band1" name:@"Band 1" addressOffset:100];
		AUParameterGroup *band2 = [AUParameterTree createGroupFromTemplate:tpl identifier:@"band2" name:@"Band 2" addressOffset:200];
		AUParameterTree *tree2 = [AUParameterTree createTreeWithChildren:@[band1, band2]];
		NSMutableArray *kp = [NSMutableArray array], *addr = [NSMutableArray array];
		for (AUParameter *p in tree2.allParameters) {
			[kp addObject:p.keyPath];
			[addr addObject:@(p.address)];
		}
		CHECK([kp isEqual:(@[@"band1.a", @"band1.b", @"band2.a", @"band2.b"])]);
		CHECK([addr isEqual:(@[@100, @101, @200, @201])]);
		CHECK([tree2 parameterWithAddress:201] == tree2.allParameters[3]);

		// Invalid trees raise AUInvalidNodeInfoException
		BOOL raisedAddress = NO, raisedIdentifier = NO;
		@try {
			[AUParameterTree createTreeWithChildren:@[makeParam(@"x", 7, 0, 1, 0, nil), makeParam(@"y", 7, 0, 1, 0, nil)]];
		} @catch (NSException *e) {
			raisedAddress = [e.name isEqualToString:@"AUInvalidNodeInfoException"];
		}
		@try {
			[AUParameterTree createGroupWithIdentifier:@"g" name:@"G" children:@[makeParam(@"x", 7, 0, 1, 0, nil), makeParam(@"x", 8, 0, 1, 0, nil)]];
		} @catch (NSException *e) {
			raisedIdentifier = [e.name isEqualToString:@"AUInvalidNodeInfoException"];
		}
		CHECK(raisedAddress && raisedIdentifier);
		CHECK([AUParameterTree createTreeWithChildren:@[]].allParameters.count == 0);

		// Description
		NSString *desc = [vol description];
		CHECK([desc hasPrefix:@"<AUParameter: 0x"] && [desc containsString:@"\"volume\", \"VOLUME\", addr 0x1, val "]);

		// Secure coding
		CHECK([AUParameterTree supportsSecureCoding] && [AUParameterGroup supportsSecureCoding] && [AUParameter supportsSecureCoding]);
		vol.value = 5;
		NSError *error = nil;
		NSData *data = [NSKeyedArchiver archivedDataWithRootObject:tree requiringSecureCoding:YES error:&error];
		CHECK(data != nil);
		NSSet *classes = [NSSet setWithObjects:[AUParameterTree class], [AUParameterGroup class], [AUParameter class],
			[NSArray class], [NSString class], [NSNumber class], nil];
		AUParameterTree *decoded = data ? [NSKeyedUnarchiver unarchivedObjectOfClasses:classes fromData:data error:&error] : nil;
		CHECK([decoded isKindOfClass:[AUParameterTree class]] && decoded.allParameters.count == 3);
		AUParameter *dvol = [decoded parameterWithAddress:1];
		CHECK(dvol != nil && [dvol.identifier isEqualToString:@"volume"] && dvol.value == 5 && dvol.maxValue == 10);
		CHECK([[decoded parameterWithAddress:3].keyPath isEqualToString:@"lfo.mode"]);
		CHECK([[decoded parameterWithAddress:3].valueStrings isEqual:(@[@"off", @"on", @"auto"])]);
		CHECK([[decoded valueForKeyPath:@"lfo.rate"] isKindOfClass:[AUParameter class]]);
	}
	printf("%s\n", failures ? "FAILED" : "ALL PASSED");
	return failures != 0;
}

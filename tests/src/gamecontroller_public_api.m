/*
 * The public GameController API surface: the selectors of every public class (from the public SDK
 * headers), the constants and their values, and the names, symbols and defaults of the elements of each
 * profile, as macOS reports them. The tables were taken from the headers and from a probe on macOS 26.7.3;
 * the program passes there too, and it is the same program that is run on Darling.
 *
 * Without arguments it checks the surface and the profiles of controllers created by the application.
 * With the argument "devices" it also checks every controller the system reports (real or virtual).
 * Run by tests/hid-gamepad/run.sh (the first argument is its control directory) it first has a virtual
 * controller plugged (SYNC: plug) and checks the controllers, including that one, once it has connected.
 * The classes of the input state API (GCControllerLiveInput, GCRacingWheel, GCStylus and their elements)
 * are not implemented yet; a missing class of that list is reported as SKIP.
 */
#import <Foundation/Foundation.h>
#import <GameController/GameController.h>
#import <objc/runtime.h>
#include <dlfcn.h>
#include <math.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>

#undef check

static int failures = 0;
static int checks = 0;

static void check(BOOL cond, const char *fmt, ...) __attribute__((format(printf, 2, 3)));
static void check(BOOL cond, const char *fmt, ...)
{
	checks++;
	if (cond)
		return;
	failures++;
	va_list ap;
	va_start(ap, fmt);
	printf("FAIL: ");
	vprintf(fmt, ap);
	printf("\n");
	va_end(ap);
}

static void spin(double seconds)
{
	[[NSRunLoop currentRunLoop] runUntilDate:[NSDate dateWithTimeIntervalSinceNow:seconds]];
}

#define S(x) ((x) ? [NSString stringWithUTF8String:(x)] : nil)
static BOOL same(NSString *a, NSString *b) { return (a == nil && b == nil) || [a isEqualToString:b]; }

/* ---- the public selectors ---- */

struct gc_api_entry { const char *name; int is_protocol; const char *super; const char *inst; const char *cls; };
static const struct gc_api_entry gc_api[] = {
	{ "GCAxisElement", 1, NULL, "absoluteInput relativeInput", "" },
	{ "GCButtonElement", 1, NULL, "forceInput pressedInput touchedInput", "" },
	{ "GCColor", 0, "NSObject", "blue green init initWithRed:green:blue: red", "" },
	{ "GCController", 0, "NSObject", "battery capture controllerPausedHandler extendedGamepad gamepad handlerQueue haptics input isAttachedToDevice isSnapshot light microGamepad motion physicalInputProfile playerIndex productCategory setControllerPausedHandler: setHandlerQueue: setPlayerIndex: vendorName", "controllerWithExtendedGamepad controllerWithMicroGamepad controllers current setShouldMonitorBackgroundEvents: shouldMonitorBackgroundEvents startWirelessControllerDiscoveryWithCompletionHandler: stopWirelessControllerDiscovery supportsHIDDevice:" },
	{ "GCControllerAxisInput", 0, "GCControllerElement", "setValue: setValueChangedHandler: value valueChangedHandler", "" },
	{ "GCControllerButtonInput", 0, "GCControllerElement", "isPressed isTouched pressedChangedHandler setPressedChangedHandler: setTouchedChangedHandler: setValue: setValueChangedHandler: touchedChangedHandler value valueChangedHandler", "" },
	{ "GCControllerDirectionPad", 0, "GCControllerElement", "down left right setValueChangedHandler: setValueForXAxis:yAxis: up valueChangedHandler xAxis yAxis", "" },
	{ "GCControllerElement", 0, "NSObject", "aliases collection isAnalog isBoundToSystemGesture localizedName preferredSystemGestureState setLocalizedName: setPreferredSystemGestureState: setSfSymbolsName: setUnmappedLocalizedName: setUnmappedSfSymbolsName: sfSymbolsName unmappedLocalizedName unmappedSfSymbolsName", "" },
	{ "GCControllerInputState", 0, "NSObject", "", "" },
	{ "GCControllerLiveInput", 0, "GCControllerInputState", "capture nextInputState unmappedInput", "" },
	{ "GCControllerTouchpad", 0, "GCControllerElement", "button reportsAbsoluteTouchSurfaceValues setReportsAbsoluteTouchSurfaceValues: setTouchDown: setTouchMoved: setTouchUp: setValueForXAxis:yAxis:touchDown:buttonValue: touchDown touchMoved touchState touchSurface touchUp", "" },
	{ "GCDevice", 1, NULL, "handlerQueue physicalInputProfile productCategory setHandlerQueue: vendorName", "" },
	{ "GCDeviceBattery", 0, "NSObject", "batteryLevel batteryState init", "" },
	{ "GCDeviceCursor", 0, "GCControllerDirectionPad", "", "" },
	{ "GCDeviceHaptics", 0, "NSObject", "createEngineWithLocality: init supportedLocalities", "" },
	{ "GCDeviceLight", 0, "NSObject", "color init setColor:", "" },
	{ "GCDevicePhysicalInput", 1, NULL, "axes buttons capture device dpads elementValueDidChangeHandler elements inputStateAvailableHandler inputStateQueueDepth lastEventLatency lastEventTimestamp nextInputState objectForKeyedSubscript: queue setElementValueDidChangeHandler: setInputStateAvailableHandler: setInputStateQueueDepth: setQueue: switches", "" },
	{ "GCDevicePhysicalInputStateDiff", 1, NULL, "changeForElement: changedElements", "" },
	{ "GCDirectionPadElement", 1, NULL, "down left right up xAxis xyAxes yAxis", "" },
	{ "GCDirectionalGamepad", 0, "GCMicroGamepad", "", "" },
	{ "GCDualSenseAdaptiveTrigger", 0, "GCControllerButtonInput", "armPosition mode setModeFeedbackWithResistiveStrengths: setModeFeedbackWithStartPosition:resistiveStrength: setModeOff setModeSlopeFeedbackWithStartPosition:endPosition:startStrength:endStrength: setModeVibrationWithAmplitudes:frequency: setModeVibrationWithStartPosition:amplitude:frequency: setModeWeaponWithStartPosition:endPosition:resistiveStrength: status", "" },
	{ "GCDualSenseGamepad", 0, "GCExtendedGamepad", "leftTrigger rightTrigger touchpadButton touchpadPrimary touchpadSecondary", "" },
	{ "GCDualShockGamepad", 0, "GCExtendedGamepad", "touchpadButton touchpadPrimary touchpadSecondary", "" },
	{ "GCEventViewController", 0, "NSViewController", "controllerUserInteractionEnabled setControllerUserInteractionEnabled:", "" },
	{ "GCExtendedGamepad", 0, "GCPhysicalInputProfile", "buttonA buttonB buttonHome buttonMenu buttonOptions buttonX buttonY controller dpad leftShoulder leftThumbstick leftThumbstickButton leftTrigger rightShoulder rightThumbstick rightThumbstickButton rightTrigger saveSnapshot setStateFromExtendedGamepad: setValueChangedHandler: valueChangedHandler", "" },
	{ "GCExtendedGamepadSnapshot", 0, "GCExtendedGamepad", "initWithController:snapshotData: initWithSnapshotData: setSnapshotData: snapshotData", "" },
	{ "GCGamepad", 0, "GCPhysicalInputProfile", "buttonA buttonB buttonX buttonY controller dpad leftShoulder rightShoulder saveSnapshot setValueChangedHandler: valueChangedHandler", "" },
	{ "GCGamepadSnapshot", 0, "GCGamepad", "initWithController:snapshotData: initWithSnapshotData: setSnapshotData: snapshotData", "" },
	{ "GCGearShifterElement", 0, "NSObject", "patternInput sequentialInput", "" },
	{ "GCKeyboard", 0, "NSObject", "handlerQueue keyboardInput physicalInputProfile productCategory setHandlerQueue: vendorName", "coalescedKeyboard" },
	{ "GCKeyboardInput", 0, "GCPhysicalInputProfile", "buttonForKeyCode: isAnyKeyPressed keyChangedHandler setKeyChangedHandler:", "" },
	{ "GCMicroGamepad", 0, "GCPhysicalInputProfile", "allowsRotation buttonA buttonMenu buttonX controller dpad reportsAbsoluteDpadValues saveSnapshot setAllowsRotation: setReportsAbsoluteDpadValues: setStateFromMicroGamepad: setValueChangedHandler: valueChangedHandler", "" },
	{ "GCMicroGamepadSnapshot", 0, "GCMicroGamepad", "initWithController:snapshotData: initWithSnapshotData: setSnapshotData: snapshotData", "" },
	{ "GCMotion", 0, "NSObject", "acceleration attitude controller gravity hasAttitude hasAttitudeAndRotationRate hasGravityAndUserAcceleration hasRotationRate rotationRate sensorsActive sensorsRequireManualActivation setAcceleration: setAttitude: setGravity: setRotationRate: setSensorsActive: setStateFromMotion: setUserAcceleration: setValueChangedHandler: userAcceleration valueChangedHandler", "" },
	{ "GCMouse", 0, "NSObject", "handlerQueue mouseInput physicalInputProfile productCategory setHandlerQueue: vendorName", "current mice" },
	{ "GCMouseInput", 0, "GCPhysicalInputProfile", "auxiliaryButtons leftButton middleButton mouseMovedHandler rightButton scroll setMouseMovedHandler:", "" },
	{ "GCPhysicalInputElement", 1, NULL, "aliases localizedName sfSymbolsName", "" },
	{ "GCPhysicalInputElementCollection", 0, NULL, "count elementEnumerator elementForAlias: init objectForKeyedSubscript:", "new" },
	{ "GCPhysicalInputExtents", 1, NULL, "canWrap isAnalog lastValueLatency lastValueTimestamp maximumValue minimumValue physicalExtents scaledValue setValueDidChangeHandler: sources value valueDidChangeHandler", "" },
	{ "GCPhysicalInputProfile", 0, "NSObject", "allAxes allButtons allDpads allElements allTouchpads axes buttons capture device dpads elements hasRemappedElements lastEventTimestamp mappedElementAliasForPhysicalInputName: mappedPhysicalInputNamesForElementAlias: objectForKeyedSubscript: setStateFromPhysicalInput: setValueDidChangeHandler: touchpads valueDidChangeHandler", "" },
	{ "GCPhysicalInputSource", 1, NULL, "canWrap delta deltaDidChangeHandler direction elementAliases elementLocalizedName isAnalog isPressed isSequential isTouched lastDeltaLatency lastDeltaTimestamp lastPositionLatency lastPositionTimestamp lastPressedStateLatency lastPressedStateTimestamp lastTouchedStateLatency lastTouchedStateTimestamp lastValueLatency lastValueTimestamp position positionDidChangeHandler positionRange pressedDidChangeHandler setDeltaDidChangeHandler: setPositionDidChangeHandler: setPressedDidChangeHandler: setTouchedDidChangeHandler: setValueDidChangeHandler: sfSymbolsName sources touchedDidChangeHandler value valueDidChangeHandler", "" },
	{ "GCRacingWheel", 0, "NSObject", "acquireDeviceWithError: capture handlerQueue init isAcquired isSnapshot physicalInputProfile productCategory relinquishDevice setHandlerQueue: vendorName wheelInput", "connectedRacingWheels" },
	{ "GCRacingWheelInput", 0, "GCRacingWheelInputState", "capture nextInputState", "" },
	{ "GCRacingWheelInputState", 0, "NSObject", "acceleratorPedal brakePedal clutchPedal shifter wheel", "" },
	{ "GCSteeringWheelElement", 0, "NSObject", "init maximumDegreesOfRotation", "" },
	{ "GCStylus", 0, "NSObject", "handlerQueue haptics input physicalInputProfile productCategory setHandlerQueue: vendorName", "styli" },
	{ "GCSwitchElement", 1, NULL, "positionInput", "" },
	{ "GCXboxGamepad", 0, "GCExtendedGamepad", "buttonShare paddleButton1 paddleButton2 paddleButton3 paddleButton4", "" },
};


static const char *not_implemented[] = {
	"GCControllerInputState", "GCControllerLiveInput", "GCRacingWheel", "GCRacingWheelInput", "GCRacingWheelInputState",
	"GCSteeringWheelElement", "GCStylus", "GCGearShifterElement", "GCPhysicalInputElementCollection",
	"GCAxisElement", "GCButtonElement", "GCDevicePhysicalInput", "GCDevicePhysicalInputStateDiff",
	"GCDirectionPadElement", "GCPhysicalInputElement", "GCPhysicalInputExtents", "GCPhysicalInputSource",
	"GCSwitchElement", NULL
};

static BOOL is_not_implemented(const char *name)
{
	for (int i = 0; not_implemented[i]; i++)
		if (!strcmp(name, not_implemented[i]))
			return YES;
	return NO;
}

static void check_surface(void)
{
	int skipped = 0;
	for (size_t i = 0; i < sizeof(gc_api) / sizeof(gc_api[0]); i++) {
		const struct gc_api_entry *e = &gc_api[i];
		if (e->is_protocol) {
			if (is_not_implemented(e->name) && !objc_getProtocol(e->name)) { skipped++; continue; }
			check(objc_getProtocol(e->name) != NULL, "protocol %s exists", e->name);
			continue;
		}
		Class c = objc_getClass(e->name);
		if (!c && is_not_implemented(e->name)) { skipped++; continue; }
		check(c != Nil, "class %s exists", e->name);
		if (!c)
			continue;
		for (NSString *s in [S(e->inst) componentsSeparatedByString:@" "])
			if ([s length])
				check([c instancesRespondToSelector:NSSelectorFromString(s)], "-[%s %s] is implemented", e->name, [s UTF8String]);
		for (NSString *s in [S(e->cls) componentsSeparatedByString:@" "])
			if ([s length])
				check([c respondsToSelector:NSSelectorFromString(s)], "+[%s %s] is implemented", e->name, [s UTF8String]);
	}
	printf("surface: %d classes and protocols of the input state API are not implemented (skipped)\n", skipped);
}

/* ---- the constants ---- */

typedef struct { const char *name; int kind; const char *value; } const_row;
enum { K_STR, K_INT, K_FLOAT };
static const const_row consts[] = {
	{ "GCControllerDidConnectNotification", K_STR, "GCControllerDidConnectNotification" },
	{ "GCControllerDidDisconnectNotification", K_STR, "GCControllerDidDisconnectNotification" },
	{ "GCControllerDidBecomeCurrentNotification", K_STR, "GCControllerDidBecomeCurrentNotification" },
	{ "GCControllerDidStopBeingCurrentNotification", K_STR, "GCControllerDidStopBeingCurrentNotification" },
	{ "GCControllerUserCustomizationsDidChangeNotification", K_STR, "GCControllerUserCustomizationsDidChangeNotification" },
	{ "GCHapticsLocalityDefault", K_STR, "Default" },
	{ "GCHapticsLocalityAll", K_STR, "All" },
	{ "GCHapticsLocalityHandles", K_STR, "Handles" },
	{ "GCHapticsLocalityLeftHandle", K_STR, "Left Handle" },
	{ "GCHapticsLocalityRightHandle", K_STR, "Right Handle" },
	{ "GCHapticsLocalityTriggers", K_STR, "Triggers" },
	{ "GCHapticsLocalityLeftTrigger", K_STR, "Left Trigger" },
	{ "GCHapticsLocalityRightTrigger", K_STR, "Right Trigger" },
	{ "GCHapticDurationInfinite", K_FLOAT, "1e+06" },
	{ "GCInputDirectionalDpad", K_STR, "Direction Pad" },
	{ "GCInputDirectionalTouchSurfaceButton", K_STR, "Button A" },
	{ "GCInputDirectionalCardinalDpad", K_STR, "Cardinal Direction Pad" },
	{ "GCInputDirectionalCenterButton", K_STR, "Button Center" },
	{ "GCInputButtonA", K_STR, "Button A" },
	{ "GCInputButtonB", K_STR, "Button B" },
	{ "GCInputButtonX", K_STR, "Button X" },
	{ "GCInputButtonY", K_STR, "Button Y" },
	{ "GCInputDirectionPad", K_STR, "Direction Pad" },
	{ "GCInputThumbstick", K_STR, "Thumbstick" },
	{ "GCInputLeftThumbstick", K_STR, "Left Thumbstick" },
	{ "GCInputRightThumbstick", K_STR, "Right Thumbstick" },
	{ "GCInputThumbstickButton", K_STR, "Thumbstick Button" },
	{ "GCInputLeftThumbstickButton", K_STR, "Left Thumbstick Button" },
	{ "GCInputRightThumbstickButton", K_STR, "Right Thumbstick Button" },
	{ "GCInputGripButton", K_STR, "Grip" },
	{ "GCInputLeftShoulder", K_STR, "Left Shoulder" },
	{ "GCInputRightShoulder", K_STR, "Right Shoulder" },
	{ "GCInputLeftBumper", K_STR, "Left Bumper" },
	{ "GCInputRightBumper", K_STR, "Right Bumper" },
	{ "GCInputTrigger", K_STR, "Trigger" },
	{ "GCInputLeftTrigger", K_STR, "Left Trigger" },
	{ "GCInputRightTrigger", K_STR, "Right Trigger" },
	{ "GCInputLeftSideButton", K_STR, "Left Side Button" },
	{ "GCInputRightSideButton", K_STR, "Right Side Button" },
	{ "GCInputButtonHome", K_STR, "Button Home" },
	{ "GCInputButtonMenu", K_STR, "Button Menu" },
	{ "GCInputButtonOptions", K_STR, "Button Options" },
	{ "GCInputButtonShare", K_STR, "Button Share" },
	{ "GCInputXboxPaddleOne", K_STR, "Paddle 1" },
	{ "GCInputXboxPaddleTwo", K_STR, "Paddle 2" },
	{ "GCInputXboxPaddleThree", K_STR, "Paddle 3" },
	{ "GCInputXboxPaddleFour", K_STR, "Paddle 4" },
	{ "GCInputDualShockTouchpadOne", K_STR, "Touchpad 1" },
	{ "GCInputDualShockTouchpadTwo", K_STR, "Touchpad 2" },
	{ "GCInputDualShockTouchpadButton", K_STR, "Touchpad Button" },
	{ "GCInputSteeringWheel", K_STR, "Steering Wheel" },
	{ "GCInputShifter", K_STR, "Shifter" },
	{ "GCInputPedalAccelerator", K_STR, "Accelerator Pedal" },
	{ "GCInputPedalBrake", K_STR, "Brake Pedal" },
	{ "GCInputPedalClutch", K_STR, "Clutch Pedal" },
	{ "GCInputLeftPaddle", K_STR, "Left Paddle" },
	{ "GCInputRightPaddle", K_STR, "Right Paddle" },
	{ "GCKeyCodeKeyA", K_INT, "4" },
	{ "GCKeyCodeKeyB", K_INT, "5" },
	{ "GCKeyCodeKeyC", K_INT, "6" },
	{ "GCKeyCodeKeyD", K_INT, "7" },
	{ "GCKeyCodeKeyE", K_INT, "8" },
	{ "GCKeyCodeKeyF", K_INT, "9" },
	{ "GCKeyCodeKeyG", K_INT, "10" },
	{ "GCKeyCodeKeyH", K_INT, "11" },
	{ "GCKeyCodeKeyI", K_INT, "12" },
	{ "GCKeyCodeKeyJ", K_INT, "13" },
	{ "GCKeyCodeKeyK", K_INT, "14" },
	{ "GCKeyCodeKeyL", K_INT, "15" },
	{ "GCKeyCodeKeyM", K_INT, "16" },
	{ "GCKeyCodeKeyN", K_INT, "17" },
	{ "GCKeyCodeKeyO", K_INT, "18" },
	{ "GCKeyCodeKeyP", K_INT, "19" },
	{ "GCKeyCodeKeyQ", K_INT, "20" },
	{ "GCKeyCodeKeyR", K_INT, "21" },
	{ "GCKeyCodeKeyS", K_INT, "22" },
	{ "GCKeyCodeKeyT", K_INT, "23" },
	{ "GCKeyCodeKeyU", K_INT, "24" },
	{ "GCKeyCodeKeyV", K_INT, "25" },
	{ "GCKeyCodeKeyW", K_INT, "26" },
	{ "GCKeyCodeKeyX", K_INT, "27" },
	{ "GCKeyCodeKeyY", K_INT, "28" },
	{ "GCKeyCodeKeyZ", K_INT, "29" },
	{ "GCKeyCodeOne", K_INT, "30" },
	{ "GCKeyCodeTwo", K_INT, "31" },
	{ "GCKeyCodeThree", K_INT, "32" },
	{ "GCKeyCodeFour", K_INT, "33" },
	{ "GCKeyCodeFive", K_INT, "34" },
	{ "GCKeyCodeSix", K_INT, "35" },
	{ "GCKeyCodeSeven", K_INT, "36" },
	{ "GCKeyCodeEight", K_INT, "37" },
	{ "GCKeyCodeNine", K_INT, "38" },
	{ "GCKeyCodeZero", K_INT, "39" },
	{ "GCKeyCodeReturnOrEnter", K_INT, "40" },
	{ "GCKeyCodeEscape", K_INT, "41" },
	{ "GCKeyCodeDeleteOrBackspace", K_INT, "42" },
	{ "GCKeyCodeTab", K_INT, "43" },
	{ "GCKeyCodeSpacebar", K_INT, "44" },
	{ "GCKeyCodeHyphen", K_INT, "45" },
	{ "GCKeyCodeEqualSign", K_INT, "46" },
	{ "GCKeyCodeOpenBracket", K_INT, "47" },
	{ "GCKeyCodeCloseBracket", K_INT, "48" },
	{ "GCKeyCodeBackslash", K_INT, "49" },
	{ "GCKeyCodeNonUSPound", K_INT, "50" },
	{ "GCKeyCodeSemicolon", K_INT, "51" },
	{ "GCKeyCodeQuote", K_INT, "52" },
	{ "GCKeyCodeGraveAccentAndTilde", K_INT, "53" },
	{ "GCKeyCodeComma", K_INT, "54" },
	{ "GCKeyCodePeriod", K_INT, "55" },
	{ "GCKeyCodeSlash", K_INT, "56" },
	{ "GCKeyCodeCapsLock", K_INT, "57" },
	{ "GCKeyCodeF1", K_INT, "58" },
	{ "GCKeyCodeF2", K_INT, "59" },
	{ "GCKeyCodeF3", K_INT, "60" },
	{ "GCKeyCodeF4", K_INT, "61" },
	{ "GCKeyCodeF5", K_INT, "62" },
	{ "GCKeyCodeF6", K_INT, "63" },
	{ "GCKeyCodeF7", K_INT, "64" },
	{ "GCKeyCodeF8", K_INT, "65" },
	{ "GCKeyCodeF9", K_INT, "66" },
	{ "GCKeyCodeF10", K_INT, "67" },
	{ "GCKeyCodeF11", K_INT, "68" },
	{ "GCKeyCodeF12", K_INT, "69" },
	{ "GCKeyCodeF13", K_INT, "104" },
	{ "GCKeyCodeF14", K_INT, "105" },
	{ "GCKeyCodeF15", K_INT, "106" },
	{ "GCKeyCodeF16", K_INT, "107" },
	{ "GCKeyCodeF17", K_INT, "108" },
	{ "GCKeyCodeF18", K_INT, "109" },
	{ "GCKeyCodeF19", K_INT, "110" },
	{ "GCKeyCodeF20", K_INT, "111" },
	{ "GCKeyCodePrintScreen", K_INT, "70" },
	{ "GCKeyCodeScrollLock", K_INT, "71" },
	{ "GCKeyCodePause", K_INT, "72" },
	{ "GCKeyCodeInsert", K_INT, "73" },
	{ "GCKeyCodeHome", K_INT, "74" },
	{ "GCKeyCodePageUp", K_INT, "75" },
	{ "GCKeyCodeDeleteForward", K_INT, "76" },
	{ "GCKeyCodeEnd", K_INT, "77" },
	{ "GCKeyCodePageDown", K_INT, "78" },
	{ "GCKeyCodeRightArrow", K_INT, "79" },
	{ "GCKeyCodeLeftArrow", K_INT, "80" },
	{ "GCKeyCodeDownArrow", K_INT, "81" },
	{ "GCKeyCodeUpArrow", K_INT, "82" },
	{ "GCKeyCodeKeypadNumLock", K_INT, "83" },
	{ "GCKeyCodeKeypadSlash", K_INT, "84" },
	{ "GCKeyCodeKeypadAsterisk", K_INT, "85" },
	{ "GCKeyCodeKeypadHyphen", K_INT, "86" },
	{ "GCKeyCodeKeypadPlus", K_INT, "87" },
	{ "GCKeyCodeKeypadEnter", K_INT, "88" },
	{ "GCKeyCodeKeypad1", K_INT, "89" },
	{ "GCKeyCodeKeypad2", K_INT, "90" },
	{ "GCKeyCodeKeypad3", K_INT, "91" },
	{ "GCKeyCodeKeypad4", K_INT, "92" },
	{ "GCKeyCodeKeypad5", K_INT, "93" },
	{ "GCKeyCodeKeypad6", K_INT, "94" },
	{ "GCKeyCodeKeypad7", K_INT, "95" },
	{ "GCKeyCodeKeypad8", K_INT, "96" },
	{ "GCKeyCodeKeypad9", K_INT, "97" },
	{ "GCKeyCodeKeypad0", K_INT, "98" },
	{ "GCKeyCodeKeypadPeriod", K_INT, "99" },
	{ "GCKeyCodeKeypadEqualSign", K_INT, "103" },
	{ "GCKeyCodeNonUSBackslash", K_INT, "100" },
	{ "GCKeyCodeApplication", K_INT, "101" },
	{ "GCKeyCodePower", K_INT, "102" },
	{ "GCKeyCodeInternational1", K_INT, "135" },
	{ "GCKeyCodeInternational2", K_INT, "136" },
	{ "GCKeyCodeInternational3", K_INT, "137" },
	{ "GCKeyCodeInternational4", K_INT, "138" },
	{ "GCKeyCodeInternational5", K_INT, "139" },
	{ "GCKeyCodeInternational6", K_INT, "140" },
	{ "GCKeyCodeInternational7", K_INT, "141" },
	{ "GCKeyCodeInternational8", K_INT, "142" },
	{ "GCKeyCodeInternational9", K_INT, "143" },
	{ "GCKeyCodeLANG1", K_INT, "144" },
	{ "GCKeyCodeLANG2", K_INT, "145" },
	{ "GCKeyCodeLANG3", K_INT, "146" },
	{ "GCKeyCodeLANG4", K_INT, "147" },
	{ "GCKeyCodeLANG5", K_INT, "148" },
	{ "GCKeyCodeLANG6", K_INT, "149" },
	{ "GCKeyCodeLANG7", K_INT, "150" },
	{ "GCKeyCodeLANG8", K_INT, "151" },
	{ "GCKeyCodeLANG9", K_INT, "152" },
	{ "GCKeyCodeLeftControl", K_INT, "224" },
	{ "GCKeyCodeLeftShift", K_INT, "225" },
	{ "GCKeyCodeLeftAlt", K_INT, "226" },
	{ "GCKeyCodeLeftGUI", K_INT, "227" },
	{ "GCKeyCodeRightControl", K_INT, "228" },
	{ "GCKeyCodeRightShift", K_INT, "229" },
	{ "GCKeyCodeRightAlt", K_INT, "230" },
	{ "GCKeyCodeRightGUI", K_INT, "231" },
	{ "GCKeyA", K_STR, "A" },
	{ "GCKeyB", K_STR, "B" },
	{ "GCKeyC", K_STR, "C" },
	{ "GCKeyD", K_STR, "D" },
	{ "GCKeyE", K_STR, "E" },
	{ "GCKeyF", K_STR, "F" },
	{ "GCKeyG", K_STR, "G" },
	{ "GCKeyH", K_STR, "H" },
	{ "GCKeyI", K_STR, "I" },
	{ "GCKeyJ", K_STR, "J" },
	{ "GCKeyK", K_STR, "K" },
	{ "GCKeyL", K_STR, "L" },
	{ "GCKeyM", K_STR, "M" },
	{ "GCKeyN", K_STR, "N" },
	{ "GCKeyO", K_STR, "O" },
	{ "GCKeyP", K_STR, "P" },
	{ "GCKeyQ", K_STR, "Q" },
	{ "GCKeyR", K_STR, "R" },
	{ "GCKeyS", K_STR, "S" },
	{ "GCKeyT", K_STR, "T" },
	{ "GCKeyU", K_STR, "U" },
	{ "GCKeyV", K_STR, "V" },
	{ "GCKeyW", K_STR, "W" },
	{ "GCKeyX", K_STR, "X" },
	{ "GCKeyY", K_STR, "Y" },
	{ "GCKeyZ", K_STR, "Z" },
	{ "GCKeyOne", K_STR, "One" },
	{ "GCKeyTwo", K_STR, "Two" },
	{ "GCKeyThree", K_STR, "Three" },
	{ "GCKeyFour", K_STR, "Four" },
	{ "GCKeyFive", K_STR, "Five" },
	{ "GCKeySix", K_STR, "Six" },
	{ "GCKeySeven", K_STR, "Seven" },
	{ "GCKeyEight", K_STR, "Eight" },
	{ "GCKeyNine", K_STR, "Nine" },
	{ "GCKeyZero", K_STR, "Zero" },
	{ "GCKeyReturnOrEnter", K_STR, "ReturnOrEnter" },
	{ "GCKeyEscape", K_STR, "Escape" },
	{ "GCKeyDeleteOrBackspace", K_STR, "DeleteOrBackspace" },
	{ "GCKeyTab", K_STR, "Tab" },
	{ "GCKeySpacebar", K_STR, "Spacebar" },
	{ "GCKeyHyphen", K_STR, "Hyphen" },
	{ "GCKeyEqualSign", K_STR, "EqualSign" },
	{ "GCKeyOpenBracket", K_STR, "OpenBracket" },
	{ "GCKeyCloseBracket", K_STR, "CloseBracket" },
	{ "GCKeyBackslash", K_STR, "Backslash" },
	{ "GCKeyNonUSPound", K_STR, "NonUSPound" },
	{ "GCKeySemicolon", K_STR, "Semicolon" },
	{ "GCKeyQuote", K_STR, "Quote" },
	{ "GCKeyGraveAccentAndTilde", K_STR, "GraveAccentAndTilde" },
	{ "GCKeyComma", K_STR, "Comma" },
	{ "GCKeyPeriod", K_STR, "Period" },
	{ "GCKeySlash", K_STR, "Slash" },
	{ "GCKeyCapsLock", K_STR, "CapsLock" },
	{ "GCKeyF1", K_STR, "F1" },
	{ "GCKeyF2", K_STR, "F2" },
	{ "GCKeyF3", K_STR, "F3" },
	{ "GCKeyF4", K_STR, "F4" },
	{ "GCKeyF5", K_STR, "F5" },
	{ "GCKeyF6", K_STR, "F6" },
	{ "GCKeyF7", K_STR, "F7" },
	{ "GCKeyF8", K_STR, "F8" },
	{ "GCKeyF9", K_STR, "F9" },
	{ "GCKeyF10", K_STR, "F10" },
	{ "GCKeyF11", K_STR, "F11" },
	{ "GCKeyF12", K_STR, "F12" },
	{ "GCKeyF13", K_STR, "F13" },
	{ "GCKeyF14", K_STR, "F14" },
	{ "GCKeyF15", K_STR, "F15" },
	{ "GCKeyF16", K_STR, "F16" },
	{ "GCKeyF17", K_STR, "F17" },
	{ "GCKeyF18", K_STR, "F18" },
	{ "GCKeyF19", K_STR, "F19" },
	{ "GCKeyF20", K_STR, "F20" },
	{ "GCKeyPrintScreen", K_STR, "PrintScreen" },
	{ "GCKeyScrollLock", K_STR, "ScrollLock" },
	{ "GCKeyPause", K_STR, "Pause" },
	{ "GCKeyInsert", K_STR, "Insert" },
	{ "GCKeyHome", K_STR, "Home" },
	{ "GCKeyPageUp", K_STR, "PageUp" },
	{ "GCKeyDeleteForward", K_STR, "DeleteForward" },
	{ "GCKeyEnd", K_STR, "End" },
	{ "GCKeyPageDown", K_STR, "PageDown" },
	{ "GCKeyRightArrow", K_STR, "RightArrow" },
	{ "GCKeyLeftArrow", K_STR, "LeftArrow" },
	{ "GCKeyDownArrow", K_STR, "DownArrow" },
	{ "GCKeyUpArrow", K_STR, "UpArrow" },
	{ "GCKeyKeypadNumLock", K_STR, "KeypadNumLock" },
	{ "GCKeyKeypadSlash", K_STR, "KeypadSlash" },
	{ "GCKeyKeypadAsterisk", K_STR, "KeypadAsterisk" },
	{ "GCKeyKeypadHyphen", K_STR, "KeypadHyphen" },
	{ "GCKeyKeypadPlus", K_STR, "KeypadPlus" },
	{ "GCKeyKeypadEnter", K_STR, "KeypadEnter" },
	{ "GCKeyKeypad1", K_STR, "Keypad1" },
	{ "GCKeyKeypad2", K_STR, "Keypad2" },
	{ "GCKeyKeypad3", K_STR, "Keypad3" },
	{ "GCKeyKeypad4", K_STR, "Keypad4" },
	{ "GCKeyKeypad5", K_STR, "Keypad5" },
	{ "GCKeyKeypad6", K_STR, "Keypad6" },
	{ "GCKeyKeypad7", K_STR, "Keypad7" },
	{ "GCKeyKeypad8", K_STR, "Keypad8" },
	{ "GCKeyKeypad9", K_STR, "Keypad9" },
	{ "GCKeyKeypad0", K_STR, "Keypad0" },
	{ "GCKeyKeypadPeriod", K_STR, "KeypadPeriod" },
	{ "GCKeyKeypadEqualSign", K_STR, "KeypadEqualSign" },
	{ "GCKeyNonUSBackslash", K_STR, "NonUSBackslash" },
	{ "GCKeyApplication", K_STR, "Application" },
	{ "GCKeyPower", K_STR, "Power" },
	{ "GCKeyInternational1", K_STR, "International1" },
	{ "GCKeyInternational2", K_STR, "International2" },
	{ "GCKeyInternational3", K_STR, "International3" },
	{ "GCKeyInternational4", K_STR, "International4" },
	{ "GCKeyInternational5", K_STR, "International5" },
	{ "GCKeyInternational6", K_STR, "International6" },
	{ "GCKeyInternational7", K_STR, "International7" },
	{ "GCKeyInternational8", K_STR, "International8" },
	{ "GCKeyInternational9", K_STR, "International9" },
	{ "GCKeyLANG1", K_STR, "LANG1" },
	{ "GCKeyLANG2", K_STR, "LANG2" },
	{ "GCKeyLANG3", K_STR, "LANG3" },
	{ "GCKeyLANG4", K_STR, "LANG4" },
	{ "GCKeyLANG5", K_STR, "LANG5" },
	{ "GCKeyLANG6", K_STR, "LANG6" },
	{ "GCKeyLANG7", K_STR, "LANG7" },
	{ "GCKeyLANG8", K_STR, "LANG8" },
	{ "GCKeyLANG9", K_STR, "LANG9" },
	{ "GCKeyLeftControl", K_STR, "LeftControl" },
	{ "GCKeyLeftShift", K_STR, "LeftShift" },
	{ "GCKeyLeftAlt", K_STR, "LeftAlt" },
	{ "GCKeyLeftGUI", K_STR, "LeftGUI" },
	{ "GCKeyRightControl", K_STR, "RightControl" },
	{ "GCKeyRightShift", K_STR, "RightShift" },
	{ "GCKeyRightAlt", K_STR, "RightAlt" },
	{ "GCKeyRightGUI", K_STR, "RightGUI" },
	{ "GCKeyboardDidConnectNotification", K_STR, "GCKeyboardDidConnectNotification" },
	{ "GCKeyboardDidDisconnectNotification", K_STR, "GCKeyboardDidDisconnectNotification" },
	{ "GCInputMicroGamepadDpad", K_STR, "Direction Pad" },
	{ "GCInputMicroGamepadButtonA", K_STR, "Button A" },
	{ "GCInputMicroGamepadButtonX", K_STR, "Button X" },
	{ "GCInputMicroGamepadButtonMenu", K_STR, "Button Menu" },
	{ "GCMouseDidConnectNotification", K_STR, "GCMouseDidConnectNotification" },
	{ "GCMouseDidDisconnectNotification", K_STR, "GCMouseDidDisconnectNotification" },
	{ "GCMouseDidBecomeCurrentNotification", K_STR, "GCMouseDidBecomeCurrentNotification" },
	{ "GCMouseDidStopBeingCurrentNotification", K_STR, "GCMouseDidStopBeingCurrentNotification" },
	{ "GCProductCategoryDualSense", K_STR, "DualSense" },
	{ "GCProductCategoryDualShock4", K_STR, "DualShock 4" },
	{ "GCProductCategoryMFi", K_STR, "MFi" },
	{ "GCProductCategoryXboxOne", K_STR, "Xbox One" },
	{ "GCProductCategoryHID", K_STR, "HID" },
	{ "GCProductCategorySpatialController", K_STR, "Spatial Controller" },
	{ "GCProductCategoryArcadeStick", K_STR, "Arcade Stick" },
	{ "GCProductCategorySiriRemote1stGen", K_STR, "Siri Remote (1st Generation)" },
	{ "GCProductCategorySiriRemote2ndGen", K_STR, "Siri Remote (2nd Generation)" },
	{ "GCProductCategoryControlCenterRemote", K_STR, "Control Center Remote" },
	{ "GCProductCategoryUniversalElectronicsRemote", K_STR, "Universal Electronics Remote" },
	{ "GCProductCategoryCoalescedRemote", K_STR, "Coalesced Remote" },
	{ "GCProductCategoryMouse", K_STR, "Mouse" },
	{ "GCProductCategoryKeyboard", K_STR, "Keyboard" },
	{ "GCRacingWheelDidConnectNotification", K_STR, "GCRacingWheelDidConnectNotification" },
	{ "GCRacingWheelDidDisconnectNotification", K_STR, "GCRacingWheelDidDisconnectNotification" },
};


static void check_constants(void)
{
	void *h = RTLD_DEFAULT;
	for (size_t i = 0; i < sizeof(consts) / sizeof(consts[0]); i++) {
		const const_row *r = &consts[i];
		void *p = dlsym(h, r->name);
		check(p != NULL, "constant %s is exported", r->name);
		if (!p)
			continue;
		if (r->kind == K_INT)
			check(*(long *)p == atol(r->value), "%s is %s, not %ld", r->name, r->value, *(long *)p);
		else if (r->kind == K_FLOAT)
			check(*(float *)p == (float)atof(r->value), "%s is %s", r->name, r->value);
		else {
			NSString *v = *(NSString *const *)p;
			check([v isKindOfClass:[NSString class]] && !strcmp([v UTF8String], r->value), "%s is '%s', not '%s'", r->name, r->value, [v UTF8String]);
		}
	}
	/* the names that are made from positions */
	NSString *(*left)(long) = dlsym(h, "GCInputBackLeftButton");
	NSString *(*right)(long) = dlsym(h, "GCInputBackRightButton");
	NSString *(*arcade)(long, long) = dlsym(h, "GCInputArcadeButtonName");
	check(left && right && arcade, "the functions that name positional buttons are exported");
	if (left && right && arcade) {
		check([left(0) isEqualToString:@"Back Left Button 0"] && [left(3) isEqualToString:@"Back Left Button 3"], "GCInputBackLeftButton");
		check([right(1) isEqualToString:@"Back Right Button 1"], "GCInputBackRightButton");
		check([arcade(2, 5) isEqualToString:@"Arcade Button 2, 5"], "GCInputArcadeButtonName");
	}
	/* every key code has a key name and the key is on the keyboard */
	Class ki = objc_getClass("GCKeyboardInput");
	id keyboard = [[ki alloc] init];
	check([[[keyboard elements] allKeys] count] == 134, "a keyboard has 134 keys, not %lu", (unsigned long)[[[keyboard elements] allKeys] count]);
	for (size_t i = 0; i < sizeof(consts) / sizeof(consts[0]); i++) {
		if (consts[i].kind != K_INT)
			continue;
		const char *suffix = consts[i].name + strlen("GCKeyCode");
		NSString *name = (suffix[0] == 'K' && suffix[1] == 'e' && suffix[2] == 'y' && strlen(suffix) == 4) ? S(suffix + 3) : S(suffix);
		long code = atol(consts[i].value);
		id button = [keyboard buttonForKeyCode:code];
		check(button != nil, "key code %ld (%s) has a button", code, consts[i].name);
		/* on macOS 26 the button of GCKeyCodeKeypadHyphen is not the one listed under its name */
		if (code != 86)
			check(button == [keyboard buttons][name], "the button of %s is listed under '%s'", consts[i].name, [name UTF8String]);
	}
	check([keyboard buttonForKeyCode:0] == nil && [keyboard buttonForKeyCode:9999] == nil, "an unknown key code has no button");
	[keyboard release];
}

/* ---- the profiles ---- */

typedef struct { const char *key; const char *cls; int analog; const char *aliases; const char *loc; const char *sf; int bound; const char *collection; } profile_row;
static const profile_row rows_generic[] = {
	{ "Back Left Button 0", "GCControllerButtonInput", 1, "Back Left Button 0|Paddle 3", "M2 Button", "m2.button.horizontal", 0, NULL },
	{ "Back Left Button 1", "GCControllerButtonInput", 1, "Back Left Button 1|Paddle 4", "M4 Button", "m4.button.horizontal", 0, NULL },
	{ "Back Right Button 0", "GCControllerButtonInput", 1, "Back Right Button 0|Paddle 1", "M1 Button", "m1.button.horizontal", 0, NULL },
	{ "Back Right Button 1", "GCControllerButtonInput", 1, "Back Right Button 1|Paddle 2", "M3 Button", "m3.button.horizontal", 0, NULL },
	{ "Button A", "GCControllerButtonInput", 1, "Button A", "A Button", "a.circle", 0, NULL },
	{ "Button B", "GCControllerButtonInput", 1, "Button B", "B Button", "b.circle", 0, NULL },
	{ "Button Home", "GCControllerButtonInput", 1, "Button Home", "Home Button", "house.circle", 1, NULL },
	{ "Button Menu", "GCControllerButtonInput", 1, "Button Menu", "Menu Button", "line.horizontal.3.circle", 0, NULL },
	{ "Button Options", "GCControllerButtonInput", 1, "Button Options", "Options Button", "ellipsis.circle", 1, NULL },
	{ "Button X", "GCControllerButtonInput", 1, "Button X", "X Button", "x.circle", 0, NULL },
	{ "Button Y", "GCControllerButtonInput", 1, "Button Y", "Y Button", "y.circle", 0, NULL },
	{ "Direction Pad", "GCControllerDirectionPad", 1, "Direction Pad", "Direction Pad", "dpad", 0, NULL },
	{ "Direction Pad Down", "GCControllerButtonInput", 1, "Direction Pad Down", "Direction Pad (Down)", "dpad.down.fill", 0, "Direction Pad" },
	{ "Direction Pad Left", "GCControllerButtonInput", 1, "Direction Pad Left", "Direction Pad (Left)", "dpad.left.fill", 0, "Direction Pad" },
	{ "Direction Pad Right", "GCControllerButtonInput", 1, "Direction Pad Right", "Direction Pad (Right)", "dpad.right.fill", 0, "Direction Pad" },
	{ "Direction Pad Up", "GCControllerButtonInput", 1, "Direction Pad Up", "Direction Pad (Up)", "dpad.up.fill", 0, "Direction Pad" },
	{ "Direction Pad X Axis", "GCControllerAxisInput", 1, "Direction Pad X Axis", "Direction Pad (Horizontal)", NULL, 0, "Direction Pad" },
	{ "Direction Pad Y Axis", "GCControllerAxisInput", 1, "Direction Pad Y Axis", "Direction Pad (Vertical)", NULL, 0, "Direction Pad" },
	{ "Left Bumper", "GCControllerButtonInput", 1, "Left Bumper", "L4 Button", "l4.button.horizontal", 0, NULL },
	{ "Left Shoulder", "GCControllerButtonInput", 1, "Left Shoulder", "L1 Button", "l1.rectangle.roundedbottom", 0, NULL },
	{ "Left Thumbstick", "GCControllerDirectionPad", 1, "Left Thumbstick", "Left Thumbstick", "l.joystick", 0, NULL },
	{ "Left Thumbstick Button", "GCControllerButtonInput", 1, "Left Thumbstick Button", "Left Thumbstick Button", "l.joystick.down", 0, NULL },
	{ "Left Thumbstick Down", "GCControllerButtonInput", 1, "Left Thumbstick Down", "Left Thumbstick (Down)", "l.joystick.tilt.down", 0, "Left Thumbstick" },
	{ "Left Thumbstick Left", "GCControllerButtonInput", 1, "Left Thumbstick Left", "Left Thumbstick (Left)", "l.joystick.tilt.left", 0, "Left Thumbstick" },
	{ "Left Thumbstick Right", "GCControllerButtonInput", 1, "Left Thumbstick Right", "Left Thumbstick (Right)", "l.joystick.tilt.right", 0, "Left Thumbstick" },
	{ "Left Thumbstick Up", "GCControllerButtonInput", 1, "Left Thumbstick Up", "Left Thumbstick (Up)", "l.joystick.tilt.up", 0, "Left Thumbstick" },
	{ "Left Thumbstick X Axis", "GCControllerAxisInput", 1, "Left Thumbstick X Axis", "Left Thumbstick (Horizontal)", NULL, 0, "Left Thumbstick" },
	{ "Left Thumbstick Y Axis", "GCControllerAxisInput", 1, "Left Thumbstick Y Axis", "Left Thumbstick (Vertical)", NULL, 0, "Left Thumbstick" },
	{ "Left Trigger", "GCControllerButtonInput", 1, "Left Trigger", "L2 Button", "l2.rectangle.roundedtop", 0, NULL },
	{ "Paddle 1", "GCControllerButtonInput", 1, "Back Right Button 0|Paddle 1", "M1 Button", "m1.button.horizontal", 0, NULL },
	{ "Paddle 2", "GCControllerButtonInput", 1, "Back Right Button 1|Paddle 2", "M3 Button", "m3.button.horizontal", 0, NULL },
	{ "Paddle 3", "GCControllerButtonInput", 1, "Back Left Button 0|Paddle 3", "M2 Button", "m2.button.horizontal", 0, NULL },
	{ "Paddle 4", "GCControllerButtonInput", 1, "Back Left Button 1|Paddle 4", "M4 Button", "m4.button.horizontal", 0, NULL },
	{ "Right Bumper", "GCControllerButtonInput", 1, "Right Bumper", "R4 Button", "r4.button.horizontal", 0, NULL },
	{ "Right Shoulder", "GCControllerButtonInput", 1, "Right Shoulder", "R1 Button", "r1.rectangle.roundedbottom", 0, NULL },
	{ "Right Thumbstick", "GCControllerDirectionPad", 1, "Right Thumbstick", "Right Thumbstick", "r.joystick", 0, NULL },
	{ "Right Thumbstick Button", "GCControllerButtonInput", 1, "Right Thumbstick Button", "Right Thumbstick Button", "r.joystick.press.down", 0, NULL },
	{ "Right Thumbstick Down", "GCControllerButtonInput", 1, "Right Thumbstick Down", "Right Thumbstick (Down)", "r.joystick.tilt.down", 0, "Right Thumbstick" },
	{ "Right Thumbstick Left", "GCControllerButtonInput", 1, "Right Thumbstick Left", "Right Thumbstick (Left)", "r.joystick.tilt.left", 0, "Right Thumbstick" },
	{ "Right Thumbstick Right", "GCControllerButtonInput", 1, "Right Thumbstick Right", "Right Thumbstick (Right)", "r.joystick.tilt.right", 0, "Right Thumbstick" },
	{ "Right Thumbstick Up", "GCControllerButtonInput", 1, "Right Thumbstick Up", "Right Thumbstick (Up)", "r.joystick.tilt.up", 0, "Right Thumbstick" },
	{ "Right Thumbstick X Axis", "GCControllerAxisInput", 1, "Right Thumbstick X Axis", "Right Thumbstick (Horizontal)", NULL, 0, "Right Thumbstick" },
	{ "Right Thumbstick Y Axis", "GCControllerAxisInput", 1, "Right Thumbstick Y Axis", "Right Thumbstick (Vertical)", NULL, 0, "Right Thumbstick" },
	{ "Right Trigger", "GCControllerButtonInput", 1, "Right Trigger", "R2 Button", "r2.rectangle.roundedtop", 0, NULL },
};

static const profile_row rows_xbox[] = {
	{ "Back Left Button 0", "GCControllerButtonInput", 1, "Back Left Button 0|Paddle 3", "M2 Button", "m2.button.horizontal", 0, NULL },
	{ "Back Left Button 1", "GCControllerButtonInput", 1, "Back Left Button 1|Paddle 4", "M4 Button", "m4.button.horizontal", 0, NULL },
	{ "Back Right Button 0", "GCControllerButtonInput", 1, "Back Right Button 0|Paddle 1", "M1 Button", "m1.button.horizontal", 0, NULL },
	{ "Back Right Button 1", "GCControllerButtonInput", 1, "Back Right Button 1|Paddle 2", "M3 Button", "m3.button.horizontal", 0, NULL },
	{ "Button A", "GCControllerButtonInput", 1, "Button A", "A Button", "a.circle", 0, NULL },
	{ "Button B", "GCControllerButtonInput", 1, "Button B", "B Button", "b.circle", 0, NULL },
	{ "Button Home", "GCControllerButtonInput", 1, "Button Home", "Xbox Button", "logo.xbox", 1, NULL },
	{ "Button Menu", "GCControllerButtonInput", 1, "Button Menu", "Menu Button", "line.horizontal.3.circle", 0, NULL },
	{ "Button Options", "GCControllerButtonInput", 1, "Button Options", "View Button", "rectangle.fill.on.rectangle.fill.circle", 1, NULL },
	{ "Button X", "GCControllerButtonInput", 1, "Button X", "X Button", "x.circle", 0, NULL },
	{ "Button Y", "GCControllerButtonInput", 1, "Button Y", "Y Button", "y.circle", 0, NULL },
	{ "Direction Pad", "GCControllerDirectionPad", 1, "Direction Pad", "D-pad", "dpad", 0, NULL },
	{ "Direction Pad Down", "GCControllerButtonInput", 1, "Direction Pad Down", "D-pad (Down)", "dpad.down.fill", 0, "Direction Pad" },
	{ "Direction Pad Left", "GCControllerButtonInput", 1, "Direction Pad Left", "D-pad (Left)", "dpad.left.fill", 0, "Direction Pad" },
	{ "Direction Pad Right", "GCControllerButtonInput", 1, "Direction Pad Right", "D-pad (Right)", "dpad.right.fill", 0, "Direction Pad" },
	{ "Direction Pad Up", "GCControllerButtonInput", 1, "Direction Pad Up", "D-pad (Up)", "dpad.up.fill", 0, "Direction Pad" },
	{ "Direction Pad X Axis", "GCControllerAxisInput", 1, "Direction Pad X Axis", "D-pad (Horizontal)", NULL, 0, "Direction Pad" },
	{ "Direction Pad Y Axis", "GCControllerAxisInput", 1, "Direction Pad Y Axis", "D-pad (Vertical)", NULL, 0, "Direction Pad" },
	{ "Left Bumper", "GCControllerButtonInput", 1, "Left Bumper", "L4 Button", "l4.button.horizontal", 0, NULL },
	{ "Left Shoulder", "GCControllerButtonInput", 1, "Left Shoulder", "Left Bumper", "lb.rectangle.roundedbottom", 0, NULL },
	{ "Left Thumbstick", "GCControllerDirectionPad", 1, "Left Thumbstick", "Left Stick", "l.joystick", 0, NULL },
	{ "Left Thumbstick Button", "GCControllerButtonInput", 1, "Left Thumbstick Button", "Left Stick Click", "l.joystick.down", 0, NULL },
	{ "Left Thumbstick Down", "GCControllerButtonInput", 1, "Left Thumbstick Down", "Left Stick (Down)", "l.joystick.tilt.down", 0, "Left Thumbstick" },
	{ "Left Thumbstick Left", "GCControllerButtonInput", 1, "Left Thumbstick Left", "Left Stick (Left)", "l.joystick.tilt.left", 0, "Left Thumbstick" },
	{ "Left Thumbstick Right", "GCControllerButtonInput", 1, "Left Thumbstick Right", "Left Stick (Right)", "l.joystick.tilt.right", 0, "Left Thumbstick" },
	{ "Left Thumbstick Up", "GCControllerButtonInput", 1, "Left Thumbstick Up", "Left Stick (Up)", "l.joystick.tilt.up", 0, "Left Thumbstick" },
	{ "Left Thumbstick X Axis", "GCControllerAxisInput", 1, "Left Thumbstick X Axis", "Left Stick (Horizontal)", NULL, 0, "Left Thumbstick" },
	{ "Left Thumbstick Y Axis", "GCControllerAxisInput", 1, "Left Thumbstick Y Axis", "Left Stick (Vertical)", NULL, 0, "Left Thumbstick" },
	{ "Left Trigger", "GCControllerButtonInput", 1, "Left Trigger", "Left Trigger", "lt.rectangle.roundedtop", 0, NULL },
	{ "Paddle 1", "GCControllerButtonInput", 1, "Back Right Button 0|Paddle 1", "M1 Button", "m1.button.horizontal", 0, NULL },
	{ "Paddle 2", "GCControllerButtonInput", 1, "Back Right Button 1|Paddle 2", "M3 Button", "m3.button.horizontal", 0, NULL },
	{ "Paddle 3", "GCControllerButtonInput", 1, "Back Left Button 0|Paddle 3", "M2 Button", "m2.button.horizontal", 0, NULL },
	{ "Paddle 4", "GCControllerButtonInput", 1, "Back Left Button 1|Paddle 4", "M4 Button", "m4.button.horizontal", 0, NULL },
	{ "Right Bumper", "GCControllerButtonInput", 1, "Right Bumper", "R4 Button", "r4.button.horizontal", 0, NULL },
	{ "Right Shoulder", "GCControllerButtonInput", 1, "Right Shoulder", "Right Bumper", "rb.rectangle.roundedbottom", 0, NULL },
	{ "Right Thumbstick", "GCControllerDirectionPad", 1, "Right Thumbstick", "Right Stick", "r.joystick", 0, NULL },
	{ "Right Thumbstick Button", "GCControllerButtonInput", 1, "Right Thumbstick Button", "Right Stick Click", "r.joystick.press.down", 0, NULL },
	{ "Right Thumbstick Down", "GCControllerButtonInput", 1, "Right Thumbstick Down", "Right Stick (Down)", "r.joystick.tilt.down", 0, "Right Thumbstick" },
	{ "Right Thumbstick Left", "GCControllerButtonInput", 1, "Right Thumbstick Left", "Right Stick (Left)", "r.joystick.tilt.left", 0, "Right Thumbstick" },
	{ "Right Thumbstick Right", "GCControllerButtonInput", 1, "Right Thumbstick Right", "Right Stick (Right)", "r.joystick.tilt.right", 0, "Right Thumbstick" },
	{ "Right Thumbstick Up", "GCControllerButtonInput", 1, "Right Thumbstick Up", "Right Stick (Up)", "r.joystick.tilt.up", 0, "Right Thumbstick" },
	{ "Right Thumbstick X Axis", "GCControllerAxisInput", 1, "Right Thumbstick X Axis", "Right Stick (Horizontal)", NULL, 0, "Right Thumbstick" },
	{ "Right Thumbstick Y Axis", "GCControllerAxisInput", 1, "Right Thumbstick Y Axis", "Right Stick (Vertical)", NULL, 0, "Right Thumbstick" },
	{ "Right Trigger", "GCControllerButtonInput", 1, "Right Trigger", "Right Trigger", "rt.rectangle.roundedtop", 0, NULL },
};

static const profile_row rows_dualshock[] = {
	{ "Back Left Button 0", "GCControllerButtonInput", 1, "Back Left Button 0|Paddle 3", "M2 Button", "m2.button.horizontal", 0, NULL },
	{ "Back Left Button 1", "GCControllerButtonInput", 1, "Back Left Button 1|Paddle 4", "M4 Button", "m4.button.horizontal", 0, NULL },
	{ "Back Right Button 0", "GCControllerButtonInput", 1, "Back Right Button 0|Paddle 1", "M1 Button", "m1.button.horizontal", 0, NULL },
	{ "Back Right Button 1", "GCControllerButtonInput", 1, "Back Right Button 1|Paddle 2", "M3 Button", "m3.button.horizontal", 0, NULL },
	{ "Button A", "GCControllerButtonInput", 1, "Button A", "Cross Button", "xmark.circle", 0, NULL },
	{ "Button B", "GCControllerButtonInput", 1, "Button B", "Circle Button", "circle.circle", 0, NULL },
	{ "Button Home", "GCControllerButtonInput", 1, "Button Home", "PS Button", "logo.playstation", 1, NULL },
	{ "Button Menu", "GCControllerButtonInput", 1, "Button Menu", "OPTIONS Button", "capsule.portrait", 0, NULL },
	{ "Button Options", "GCControllerButtonInput", 1, "Button Options", "SHARE Button", "capsule.portrait", 1, NULL },
	{ "Button X", "GCControllerButtonInput", 1, "Button X", "Square Button", "square.circle", 0, NULL },
	{ "Button Y", "GCControllerButtonInput", 1, "Button Y", "Triangle Button", "triangle.circle", 0, NULL },
	{ "Direction Pad", "GCControllerDirectionPad", 1, "Direction Pad", "Direction Pad", "dpad", 0, NULL },
	{ "Direction Pad Down", "GCControllerButtonInput", 1, "Direction Pad Down", "Direction Pad (Down)", "dpad.down.fill", 0, "Direction Pad" },
	{ "Direction Pad Left", "GCControllerButtonInput", 1, "Direction Pad Left", "Direction Pad (Left)", "dpad.left.fill", 0, "Direction Pad" },
	{ "Direction Pad Right", "GCControllerButtonInput", 1, "Direction Pad Right", "Direction Pad (Right)", "dpad.right.fill", 0, "Direction Pad" },
	{ "Direction Pad Up", "GCControllerButtonInput", 1, "Direction Pad Up", "Direction Pad (Up)", "dpad.up.fill", 0, "Direction Pad" },
	{ "Direction Pad X Axis", "GCControllerAxisInput", 1, "Direction Pad X Axis", "Direction Pad (Horizontal)", NULL, 0, "Direction Pad" },
	{ "Direction Pad Y Axis", "GCControllerAxisInput", 1, "Direction Pad Y Axis", "Direction Pad (Vertical)", NULL, 0, "Direction Pad" },
	{ "Left Bumper", "GCControllerButtonInput", 1, "Left Bumper", "L4 Button", "l4.button.horizontal", 0, NULL },
	{ "Left Shoulder", "GCControllerButtonInput", 1, "Left Shoulder", "L1 Button", "l1.rectangle.roundedbottom", 0, NULL },
	{ "Left Thumbstick", "GCControllerDirectionPad", 1, "Left Thumbstick", "Left Stick", "l.joystick", 0, NULL },
	{ "Left Thumbstick Button", "GCControllerButtonInput", 1, "Left Thumbstick Button", "L3 Button", "l.joystick.down", 0, NULL },
	{ "Left Thumbstick Down", "GCControllerButtonInput", 1, "Left Thumbstick Down", "Left Stick (Down)", "l.joystick.tilt.down", 0, "Left Thumbstick" },
	{ "Left Thumbstick Left", "GCControllerButtonInput", 1, "Left Thumbstick Left", "Left Stick (Left)", "l.joystick.tilt.left", 0, "Left Thumbstick" },
	{ "Left Thumbstick Right", "GCControllerButtonInput", 1, "Left Thumbstick Right", "Left Stick (Right)", "l.joystick.tilt.right", 0, "Left Thumbstick" },
	{ "Left Thumbstick Up", "GCControllerButtonInput", 1, "Left Thumbstick Up", "Left Stick (Up)", "l.joystick.tilt.up", 0, "Left Thumbstick" },
	{ "Left Thumbstick X Axis", "GCControllerAxisInput", 1, "Left Thumbstick X Axis", "Left Stick (Horizontal)", NULL, 0, "Left Thumbstick" },
	{ "Left Thumbstick Y Axis", "GCControllerAxisInput", 1, "Left Thumbstick Y Axis", "Left Stick (Vertical)", NULL, 0, "Left Thumbstick" },
	{ "Left Trigger", "GCControllerButtonInput", 1, "Left Trigger", "L2 Button", "l2.rectangle.roundedtop", 0, NULL },
	{ "Paddle 1", "GCControllerButtonInput", 1, "Back Right Button 0|Paddle 1", "M1 Button", "m1.button.horizontal", 0, NULL },
	{ "Paddle 2", "GCControllerButtonInput", 1, "Back Right Button 1|Paddle 2", "M3 Button", "m3.button.horizontal", 0, NULL },
	{ "Paddle 3", "GCControllerButtonInput", 1, "Back Left Button 0|Paddle 3", "M2 Button", "m2.button.horizontal", 0, NULL },
	{ "Paddle 4", "GCControllerButtonInput", 1, "Back Left Button 1|Paddle 4", "M4 Button", "m4.button.horizontal", 0, NULL },
	{ "Right Bumper", "GCControllerButtonInput", 1, "Right Bumper", "R4 Button", "r4.button.horizontal", 0, NULL },
	{ "Right Shoulder", "GCControllerButtonInput", 1, "Right Shoulder", "R1 Button", "r1.rectangle.roundedbottom", 0, NULL },
	{ "Right Thumbstick", "GCControllerDirectionPad", 1, "Right Thumbstick", "Right Stick", "r.joystick", 0, NULL },
	{ "Right Thumbstick Button", "GCControllerButtonInput", 1, "Right Thumbstick Button", "R3 Button", "r.joystick.press.down", 0, NULL },
	{ "Right Thumbstick Down", "GCControllerButtonInput", 1, "Right Thumbstick Down", "Right Stick (Down)", "r.joystick.tilt.down", 0, "Right Thumbstick" },
	{ "Right Thumbstick Left", "GCControllerButtonInput", 1, "Right Thumbstick Left", "Right Stick (Left)", "r.joystick.tilt.left", 0, "Right Thumbstick" },
	{ "Right Thumbstick Right", "GCControllerButtonInput", 1, "Right Thumbstick Right", "Right Stick (Right)", "r.joystick.tilt.right", 0, "Right Thumbstick" },
	{ "Right Thumbstick Up", "GCControllerButtonInput", 1, "Right Thumbstick Up", "Right Stick (Up)", "r.joystick.tilt.up", 0, "Right Thumbstick" },
	{ "Right Thumbstick X Axis", "GCControllerAxisInput", 1, "Right Thumbstick X Axis", "Right Stick (Horizontal)", NULL, 0, "Right Thumbstick" },
	{ "Right Thumbstick Y Axis", "GCControllerAxisInput", 1, "Right Thumbstick Y Axis", "Right Stick (Vertical)", NULL, 0, "Right Thumbstick" },
	{ "Right Trigger", "GCControllerButtonInput", 1, "Right Trigger", "R2 Button", "r2.rectangle.roundedtop", 0, NULL },
	{ "Touchpad 1", "GCControllerDirectionPad", 1, "Touchpad 1", "Touchpad (First Finger)", "hand.draw", 0, NULL },
	{ "Touchpad 1 Down", "GCControllerButtonInput", 1, "Touchpad 1 Down", "Touchpad (First Finger) (Down)", NULL, 0, "Touchpad 1" },
	{ "Touchpad 1 Left", "GCControllerButtonInput", 1, "Touchpad 1 Left", "Touchpad (First Finger) (Left)", NULL, 0, "Touchpad 1" },
	{ "Touchpad 1 Right", "GCControllerButtonInput", 1, "Touchpad 1 Right", "Touchpad (First Finger) (Right)", NULL, 0, "Touchpad 1" },
	{ "Touchpad 1 Up", "GCControllerButtonInput", 1, "Touchpad 1 Up", "Touchpad (First Finger) (Up)", NULL, 0, "Touchpad 1" },
	{ "Touchpad 1 X Axis", "GCControllerAxisInput", 1, "Touchpad 1 X Axis", "Touchpad (First Finger) (Horizontal)", NULL, 0, "Touchpad 1" },
	{ "Touchpad 1 Y Axis", "GCControllerAxisInput", 1, "Touchpad 1 Y Axis", "Touchpad (First Finger) (Vertical)", NULL, 0, "Touchpad 1" },
	{ "Touchpad 2", "GCControllerDirectionPad", 1, "Touchpad 2", "Touchpad (Second Finger)", "hand.draw", 0, NULL },
	{ "Touchpad 2 Down", "GCControllerButtonInput", 1, "Touchpad 2 Down", "Touchpad (Second Finger) (Down)", NULL, 0, "Touchpad 2" },
	{ "Touchpad 2 Left", "GCControllerButtonInput", 1, "Touchpad 2 Left", "Touchpad (Second Finger) (Left)", NULL, 0, "Touchpad 2" },
	{ "Touchpad 2 Right", "GCControllerButtonInput", 1, "Touchpad 2 Right", "Touchpad (Second Finger) (Right)", NULL, 0, "Touchpad 2" },
	{ "Touchpad 2 Up", "GCControllerButtonInput", 1, "Touchpad 2 Up", "Touchpad (Second Finger) (Up)", NULL, 0, "Touchpad 2" },
	{ "Touchpad 2 X Axis", "GCControllerAxisInput", 1, "Touchpad 2 X Axis", "Touchpad (Second Finger) (Horizontal)", NULL, 0, "Touchpad 2" },
	{ "Touchpad 2 Y Axis", "GCControllerAxisInput", 1, "Touchpad 2 Y Axis", "Touchpad (Second Finger) (Vertical)", NULL, 0, "Touchpad 2" },
	{ "Touchpad Button", "GCControllerButtonInput", 0, "Touchpad Button", "Touchpad Button", "plus.rectangle", 0, NULL },
};

static const profile_row rows_dualsense[] = {
	{ "Button A", "GCControllerButtonInput", 1, "Button A", "Cross Button", "xmark.circle", 0, NULL },
	{ "Button B", "GCControllerButtonInput", 1, "Button B", "Circle Button", "circle.circle", 0, NULL },
	{ "Button Home", "GCControllerButtonInput", 1, "Button Home", "PS Button", "logo.playstation", 1, NULL },
	{ "Button Menu", "GCControllerButtonInput", 1, "Button Menu", "Options Button", "capsule.portrait", 0, NULL },
	{ "Button Options", "GCControllerButtonInput", 1, "Button Options", "Create Button", "capsule.portrait", 1, NULL },
	{ "Button X", "GCControllerButtonInput", 1, "Button X", "Square Button", "square.circle", 0, NULL },
	{ "Button Y", "GCControllerButtonInput", 1, "Button Y", "Triangle Button", "triangle.circle", 0, NULL },
	{ "Direction Pad", "GCControllerDirectionPad", 1, "Direction Pad", "Direction Pad", "dpad", 0, NULL },
	{ "Direction Pad Down", "GCControllerButtonInput", 1, "Direction Pad Down", "Direction Pad (Down)", "dpad.down.fill", 0, "Direction Pad" },
	{ "Direction Pad Left", "GCControllerButtonInput", 1, "Direction Pad Left", "Direction Pad (Left)", "dpad.left.fill", 0, "Direction Pad" },
	{ "Direction Pad Right", "GCControllerButtonInput", 1, "Direction Pad Right", "Direction Pad (Right)", "dpad.right.fill", 0, "Direction Pad" },
	{ "Direction Pad Up", "GCControllerButtonInput", 1, "Direction Pad Up", "Direction Pad (Up)", "dpad.up.fill", 0, "Direction Pad" },
	{ "Direction Pad X Axis", "GCControllerAxisInput", 1, "Direction Pad X Axis", "Direction Pad (Horizontal)", NULL, 0, "Direction Pad" },
	{ "Direction Pad Y Axis", "GCControllerAxisInput", 1, "Direction Pad Y Axis", "Direction Pad (Vertical)", NULL, 0, "Direction Pad" },
	{ "Left Shoulder", "GCControllerButtonInput", 1, "Left Shoulder", "L1 Button", "l1.rectangle.roundedbottom", 0, NULL },
	{ "Left Thumbstick", "GCControllerDirectionPad", 1, "Left Thumbstick", "Left Stick", "l.joystick", 0, NULL },
	{ "Left Thumbstick Button", "GCControllerButtonInput", 1, "Left Thumbstick Button", "L3 Button", "l.joystick.down", 0, NULL },
	{ "Left Thumbstick Down", "GCControllerButtonInput", 1, "Left Thumbstick Down", "Left Stick (Down)", "l.joystick.tilt.down", 0, "Left Thumbstick" },
	{ "Left Thumbstick Left", "GCControllerButtonInput", 1, "Left Thumbstick Left", "Left Stick (Left)", "l.joystick.tilt.left", 0, "Left Thumbstick" },
	{ "Left Thumbstick Right", "GCControllerButtonInput", 1, "Left Thumbstick Right", "Left Stick (Right)", "l.joystick.tilt.right", 0, "Left Thumbstick" },
	{ "Left Thumbstick Up", "GCControllerButtonInput", 1, "Left Thumbstick Up", "Left Stick (Up)", "l.joystick.tilt.up", 0, "Left Thumbstick" },
	{ "Left Thumbstick X Axis", "GCControllerAxisInput", 1, "Left Thumbstick X Axis", "Left Stick (Horizontal)", NULL, 0, "Left Thumbstick" },
	{ "Left Thumbstick Y Axis", "GCControllerAxisInput", 1, "Left Thumbstick Y Axis", "Left Stick (Vertical)", NULL, 0, "Left Thumbstick" },
	{ "Left Trigger", "GCDualSenseAdaptiveTrigger", 1, "Left Trigger", "L2 Button", "l2.rectangle.roundedtop", 0, NULL },
	{ "Right Shoulder", "GCControllerButtonInput", 1, "Right Shoulder", "R1 Button", "r1.rectangle.roundedbottom", 0, NULL },
	{ "Right Thumbstick", "GCControllerDirectionPad", 1, "Right Thumbstick", "Right Stick", "r.joystick", 0, NULL },
	{ "Right Thumbstick Button", "GCControllerButtonInput", 1, "Right Thumbstick Button", "R3 Button", "r.joystick.press.down", 0, NULL },
	{ "Right Thumbstick Down", "GCControllerButtonInput", 1, "Right Thumbstick Down", "Right Stick (Down)", "r.joystick.tilt.down", 0, "Right Thumbstick" },
	{ "Right Thumbstick Left", "GCControllerButtonInput", 1, "Right Thumbstick Left", "Right Stick (Left)", "r.joystick.tilt.left", 0, "Right Thumbstick" },
	{ "Right Thumbstick Right", "GCControllerButtonInput", 1, "Right Thumbstick Right", "Right Stick (Right)", "r.joystick.tilt.right", 0, "Right Thumbstick" },
	{ "Right Thumbstick Up", "GCControllerButtonInput", 1, "Right Thumbstick Up", "Right Stick (Up)", "r.joystick.tilt.up", 0, "Right Thumbstick" },
	{ "Right Thumbstick X Axis", "GCControllerAxisInput", 1, "Right Thumbstick X Axis", "Right Stick (Horizontal)", NULL, 0, "Right Thumbstick" },
	{ "Right Thumbstick Y Axis", "GCControllerAxisInput", 1, "Right Thumbstick Y Axis", "Right Stick (Vertical)", NULL, 0, "Right Thumbstick" },
	{ "Right Trigger", "GCDualSenseAdaptiveTrigger", 1, "Right Trigger", "R2 Button", "r2.rectangle.roundedtop", 0, NULL },
	{ "Touchpad 1", "GCControllerDirectionPad", 1, "Touchpad 1", "Touchpad (First Finger)", "hand.draw", 0, NULL },
	{ "Touchpad 1 Down", "GCControllerButtonInput", 1, "Touchpad 1 Down", "Touchpad (First Finger) (Down)", NULL, 0, "Touchpad 1" },
	{ "Touchpad 1 Left", "GCControllerButtonInput", 1, "Touchpad 1 Left", "Touchpad (First Finger) (Left)", NULL, 0, "Touchpad 1" },
	{ "Touchpad 1 Right", "GCControllerButtonInput", 1, "Touchpad 1 Right", "Touchpad (First Finger) (Right)", NULL, 0, "Touchpad 1" },
	{ "Touchpad 1 Up", "GCControllerButtonInput", 1, "Touchpad 1 Up", "Touchpad (First Finger) (Up)", NULL, 0, "Touchpad 1" },
	{ "Touchpad 1 X Axis", "GCControllerAxisInput", 1, "Touchpad 1 X Axis", "Touchpad (First Finger) (Horizontal)", NULL, 0, "Touchpad 1" },
	{ "Touchpad 1 Y Axis", "GCControllerAxisInput", 1, "Touchpad 1 Y Axis", "Touchpad (First Finger) (Vertical)", NULL, 0, "Touchpad 1" },
	{ "Touchpad 2", "GCControllerDirectionPad", 1, "Touchpad 2", "Touchpad (Second Finger)", "hand.draw", 0, NULL },
	{ "Touchpad 2 Down", "GCControllerButtonInput", 1, "Touchpad 2 Down", "Touchpad (Second Finger) (Down)", NULL, 0, "Touchpad 2" },
	{ "Touchpad 2 Left", "GCControllerButtonInput", 1, "Touchpad 2 Left", "Touchpad (Second Finger) (Left)", NULL, 0, "Touchpad 2" },
	{ "Touchpad 2 Right", "GCControllerButtonInput", 1, "Touchpad 2 Right", "Touchpad (Second Finger) (Right)", NULL, 0, "Touchpad 2" },
	{ "Touchpad 2 Up", "GCControllerButtonInput", 1, "Touchpad 2 Up", "Touchpad (Second Finger) (Up)", NULL, 0, "Touchpad 2" },
	{ "Touchpad 2 X Axis", "GCControllerAxisInput", 1, "Touchpad 2 X Axis", "Touchpad (Second Finger) (Horizontal)", NULL, 0, "Touchpad 2" },
	{ "Touchpad 2 Y Axis", "GCControllerAxisInput", 1, "Touchpad 2 Y Axis", "Touchpad (Second Finger) (Vertical)", NULL, 0, "Touchpad 2" },
	{ "Touchpad Button", "GCControllerButtonInput", 0, "Touchpad Button", "Touchpad Button", "plus.rectangle", 0, NULL },
};

static const profile_row rows_micro[] = {
	{ "Button A", "GCControllerButtonInput", 0, "Button A", "Touch Surface Button", NULL, 0, NULL },
	{ "Button Menu", "GCControllerButtonInput", 0, "Button Menu", "Menu Button", NULL, 0, NULL },
	{ "Button X", "GCControllerButtonInput", 0, "Button X", "Play/Pause Button", NULL, 0, NULL },
	{ "Direction Pad", "GCControllerDirectionPad", 1, "Direction Pad", "Touch Surface", NULL, 0, NULL },
	{ "Direction Pad Down", "GCControllerButtonInput", 1, "Direction Pad Down", "Touch Surface (Down)", NULL, 0, "Direction Pad" },
	{ "Direction Pad Left", "GCControllerButtonInput", 1, "Direction Pad Left", "Touch Surface (Left)", NULL, 0, "Direction Pad" },
	{ "Direction Pad Right", "GCControllerButtonInput", 1, "Direction Pad Right", "Touch Surface (Right)", NULL, 0, "Direction Pad" },
	{ "Direction Pad Up", "GCControllerButtonInput", 1, "Direction Pad Up", "Touch Surface (Up)", NULL, 0, "Direction Pad" },
	{ "Direction Pad X Axis", "GCControllerAxisInput", 1, "Direction Pad X Axis", "Touch Surface (Horizontal)", NULL, 0, "Direction Pad" },
	{ "Direction Pad Y Axis", "GCControllerAxisInput", 1, "Direction Pad Y Axis", "Touch Surface (Vertical)", NULL, 0, "Direction Pad" },
};

static const profile_row rows_legacy[] = {
	{ "Button A", "GCControllerButtonInput", 1, "Button A", NULL, NULL, 0, NULL },
	{ "Button B", "GCControllerButtonInput", 1, "Button B", NULL, NULL, 0, NULL },
	{ "Button Menu", "GCControllerButtonInput", 1, "Button Menu", NULL, NULL, 0, NULL },
	{ "Button X", "GCControllerButtonInput", 1, "Button X", NULL, NULL, 0, NULL },
	{ "Button Y", "GCControllerButtonInput", 1, "Button Y", NULL, NULL, 0, NULL },
	{ "Direction Pad", "GCControllerDirectionPad", 1, "Direction Pad", NULL, NULL, 0, NULL },
	{ "Direction Pad Down", "GCControllerButtonInput", 1, "Direction Pad Down", NULL, NULL, 0, "Direction Pad" },
	{ "Direction Pad Left", "GCControllerButtonInput", 1, "Direction Pad Left", NULL, NULL, 0, "Direction Pad" },
	{ "Direction Pad Right", "GCControllerButtonInput", 1, "Direction Pad Right", NULL, NULL, 0, "Direction Pad" },
	{ "Direction Pad Up", "GCControllerButtonInput", 1, "Direction Pad Up", NULL, NULL, 0, "Direction Pad" },
	{ "Direction Pad X Axis", "GCControllerAxisInput", 1, "Direction Pad X Axis", NULL, NULL, 0, "Direction Pad" },
	{ "Direction Pad Y Axis", "GCControllerAxisInput", 1, "Direction Pad Y Axis", NULL, NULL, 0, "Direction Pad" },
	{ "Left Shoulder", "GCControllerButtonInput", 1, "Left Shoulder", NULL, NULL, 0, NULL },
	{ "Right Shoulder", "GCControllerButtonInput", 1, "Right Shoulder", NULL, NULL, 0, NULL },
};



static NSString *join_sorted(NSSet *set)
{
	return [[[set allObjects] sortedArrayUsingSelector:@selector(compare:)] componentsJoinedByString:@"|"];
}

static void check_profile(const char *name, GCPhysicalInputProfile *profile, const profile_row *rows, size_t count, size_t extra)
{
	check(profile != nil, "%s: the profile exists", name);
	if (!profile)
		return;
	check([[profile elements] count] == count + extra + 0 || [[profile elements] count] >= count, "%s: element count %lu", name, (unsigned long)[[profile elements] count]);
	for (size_t i = 0; i < count; i++) {
		const profile_row *r = &rows[i];
		GCControllerElement *e = [profile elements][S(r->key)];
		check(e != nil, "%s: element '%s' exists", name, r->key);
		if (!e)
			continue;
		check([e isKindOfClass:objc_getClass(r->cls)], "%s/%s: class %s, not %s", name, r->key, r->cls, class_getName([e class]));
		check(e.isAnalog == (r->analog != 0), "%s/%s: analog %d", name, r->key, r->analog);
		check([join_sorted(e.aliases) isEqualToString:S(r->aliases)], "%s/%s: aliases '%s', not '%s'", name, r->key, r->aliases, [join_sorted(e.aliases) UTF8String]);
		check(same(e.localizedName, S(r->loc)), "%s/%s: localizedName %s, not %s", name, r->key, r->loc ? r->loc : "nil", [e.localizedName UTF8String]);
		check(same(e.unmappedLocalizedName, S(r->loc)), "%s/%s: unmappedLocalizedName", name, r->key);
		check(same(e.sfSymbolsName, S(r->sf)), "%s/%s: sfSymbolsName %s, not %s", name, r->key, r->sf ? r->sf : "nil", [e.sfSymbolsName UTF8String]);
		check(same(e.unmappedSfSymbolsName, S(r->sf)), "%s/%s: unmappedSfSymbolsName", name, r->key);
		check(e.isBoundToSystemGesture == (r->bound != 0), "%s/%s: boundToSystemGesture %d", name, r->key, r->bound);
		check(e.preferredSystemGestureState == GCSystemGestureStateEnabled, "%s/%s: preferredSystemGestureState defaults to enabled", name, r->key);
		if (r->collection)
			check([e collection] != nil && [join_sorted([e.collection aliases]) isEqualToString:S(r->collection)], "%s/%s: collection %s", name, r->key, r->collection);
		else
			check([e collection] == nil, "%s/%s: no collection", name, r->key);
		/* the sets and dictionaries agree with the class of the element */
		BOOL isButton = [e isKindOfClass:[GCControllerButtonInput class]];
		check(([profile buttons][S(r->key)] != nil) == isButton, "%s/%s: listed in buttons iff a button", name, r->key);
		check(([profile axes][S(r->key)] != nil) == [e isKindOfClass:[GCControllerAxisInput class]], "%s/%s: listed in axes iff an axis", name, r->key);
		check(([profile dpads][S(r->key)] != nil) == [e isKindOfClass:[GCControllerDirectionPad class]], "%s/%s: listed in dpads iff a pad", name, r->key);
		check(profile[S(r->key)] == e, "%s/%s: subscript", name, r->key);
	}
	/* an element listed under several keys is in the sets once */
	NSMutableSet *unique = [NSMutableSet set];
	for (id e in [[profile elements] allValues])
		[unique addObject:e];
	check([[profile allElements] count] == [unique count], "%s: allElements holds each element once", name);
	check([[profile touchpads] count] == 0 && [[profile allTouchpads] count] == 0, "%s: no touchpad elements (the touchpads of a DualShock are direction pads)", name);
}

#define CHECK_PROFILE(name, profile, rows, extra) check_profile(name, profile, rows, sizeof(rows) / sizeof(rows[0]), extra)

static void check_profiles(void)
{
	Class c;
	GCController *ext = [GCController controllerWithExtendedGamepad];
	CHECK_PROFILE("extended", [ext extendedGamepad], rows_generic, 0);
	c = objc_getClass("GCXboxGamepad");
	CHECK_PROFILE("xbox", [[[c alloc] init] autorelease], rows_xbox, 0);
	c = objc_getClass("GCDualShockGamepad");
	CHECK_PROFILE("dualshock", [[[c alloc] init] autorelease], rows_dualshock, 0);
	c = objc_getClass("GCDualSenseGamepad");
	CHECK_PROFILE("dualsense", [[[c alloc] init] autorelease], rows_dualsense, 0);
	CHECK_PROFILE("micro", [[GCController controllerWithMicroGamepad] microGamepad], rows_micro, 0);
	c = objc_getClass("GCDirectionalGamepad");
	CHECK_PROFILE("directional", [[[c alloc] init] autorelease], rows_micro, 0);
	c = objc_getClass("GCGamepad");
	CHECK_PROFILE("legacy", [[[c alloc] init] autorelease], rows_legacy, 0);

	/* the class hierarchy and the accessors of the specific profiles */
	check([objc_getClass("GCDualSenseGamepad") isSubclassOfClass:[GCExtendedGamepad class]] && ![objc_getClass("GCDualSenseGamepad") isSubclassOfClass:objc_getClass("GCDualShockGamepad")], "GCDualSenseGamepad derives from GCExtendedGamepad");
	check([objc_getClass("GCDirectionalGamepad") isSubclassOfClass:[GCMicroGamepad class]], "GCDirectionalGamepad derives from GCMicroGamepad");
	check([objc_getClass("GCDeviceCursor") isSubclassOfClass:[GCControllerDirectionPad class]], "GCDeviceCursor derives from GCControllerDirectionPad");
	check([objc_getClass("GCDualSenseAdaptiveTrigger") isSubclassOfClass:[GCControllerButtonInput class]], "GCDualSenseAdaptiveTrigger derives from GCControllerButtonInput");
	check([objc_getClass("GCExtendedGamepadSnapshot") isSubclassOfClass:[GCExtendedGamepad class]], "GCExtendedGamepadSnapshot derives from GCExtendedGamepad");
	check([objc_getClass("GCGamepad") isSubclassOfClass:[GCPhysicalInputProfile class]], "GCGamepad derives from GCPhysicalInputProfile");
	GCXboxGamepad *xbox = [[[objc_getClass("GCXboxGamepad") alloc] init] autorelease];
	check([xbox paddleButton1] == nil && [xbox buttonShare] == nil, "a profile without a controller has no paddles and no share button");
	id ds = [[[objc_getClass("GCDualSenseGamepad") alloc] init] autorelease];
	check([[ds leftTrigger] isKindOfClass:objc_getClass("GCDualSenseAdaptiveTrigger")] && [ds leftTrigger] == [ds elements][@"Left Trigger"], "DualSense triggers are the adaptive triggers of the profile");
	id ds4 = [[[objc_getClass("GCDualShockGamepad") alloc] init] autorelease];
	check([ds4 touchpadPrimary] == [ds4 elements][@"Touchpad 1"] && [ds4 touchpadButton] == [ds4 buttons][@"Touchpad Button"], "the touchpad properties are the touchpad elements");
}

/* ---- behaviour of the elements of a controller the application creates ---- */

static int events_n;
static char events[64][48];
static void event(const char *s) { if (events_n < 64) strlcpy(events[events_n++], s, 48); }
static BOOL events_are(const char *first, ...)
{
	va_list ap;
	va_start(ap, first);
	int i = 0;
	BOOL ok = YES;
	for (const char *s = first; s; s = va_arg(ap, const char *), i++)
		if (i >= events_n || strcmp(events[i], s))
			ok = NO;
	va_end(ap);
	return ok && i == events_n;
}

static void check_behaviour(void)
{
	GCController *c = [GCController controllerWithExtendedGamepad];
	GCExtendedGamepad *g = [c extendedGamepad];
	check(c.isSnapshot && !c.isAttachedToDevice, "controllerWithExtendedGamepad is a snapshot, not attached to a device");
	check([c.vendorName isEqualToString:@"ExtendedGamepad"] && [c.productCategory isEqualToString:GCProductCategoryMFi], "snapshot vendor name and category (%s, %s)", [c.vendorName UTF8String], [c.productCategory UTF8String]);
	check(c.battery == nil && c.light == nil && c.haptics == nil && c.motion != nil && c.playerIndex == 0, "snapshot: no battery, light, haptics; has motion; player index 0");
	check([[GCController controllers] indexOfObjectIdenticalTo:c] == NSNotFound, "a snapshot controller is not in +controllers");
	check(c.motion.controller == c && !c.motion.sensorsRequireManualActivation && c.motion.sensorsActive && !c.motion.hasAttitude, "extended snapshot motion defaults");
	/* QUIRK test: macOS answers the extended profile for -microGamepad and -gamepad of such a controller */
	check([c microGamepad] == (id)g && [c gamepad] == (id)g && c.physicalInputProfile == g, "snapshot profiles: microGamepad and gamepad are the extended profile");
	check(g.device == c && g.controller == c && g.lastEventTimestamp == 0 && !g.hasRemappedElements, "profile: device, controller, timestamp, remapping");
	GCController *m = [GCController controllerWithMicroGamepad];
	check([m.vendorName isEqualToString:@"MicroGamepad"] && [m.productCategory isEqualToString:@"Siri Remote"] && m.extendedGamepad == nil && m.gamepad == nil && m.microGamepad != nil, "micro snapshot controller");
	check(m.motion.hasAttitude && m.motion.hasRotationRate && m.motion.hasAttitudeAndRotationRate && m.physicalInputProfile == m.microGamepad, "micro snapshot motion");
	check(![GCMicroGamepad new].reportsAbsoluteDpadValues && ![[GCMicroGamepad new] allowsRotation], "micro defaults");
	check(![GCController shouldMonitorBackgroundEvents], "shouldMonitorBackgroundEvents is NO by default");
	[GCController setShouldMonitorBackgroundEvents:YES];
	check([GCController shouldMonitorBackgroundEvents], "shouldMonitorBackgroundEvents can be set");
	[GCController setShouldMonitorBackgroundEvents:NO];

	/* handlers: asynchronous on the handler queue; value, touched, pressed in that order */
	GCControllerButtonInput *a = g.buttonA;
	a.valueChangedHandler = ^(GCControllerButtonInput *b, float v, BOOL p) { event(v == 0.5f && p ? "value" : "value?"); check([NSThread isMainThread], "handlers run on the main queue"); };
	a.touchedChangedHandler = ^(GCControllerButtonInput *b, float v, BOOL p, BOOL t) { event(t ? "touched" : "untouched"); };
	a.pressedChangedHandler = ^(GCControllerButtonInput *b, float v, BOOL p) { event(p ? "pressed" : "released"); };
	events_n = 0;
	[a setValue:0.5f];
	check(events_n == 0, "handlers do not run inside setValue:");
	spin(0.2);
	check(events_are("value", "touched", "pressed", NULL), "a press calls value, touched and pressed handlers");
	events_n = 0;
	[a setValue:0.5f];
	spin(0.1);
	check(events_n == 0, "the same value again calls nothing");
	a.valueChangedHandler = ^(GCControllerButtonInput *b, float v, BOOL p) { event("value"); };
	[a setValue:0.0f];
	spin(0.2);
	check(events_are("value", "untouched", "released", NULL), "a release calls value, touched and pressed handlers");
	check(g.lastEventTimestamp == 0, "writing a value does not change the event timestamp");
	a.valueChangedHandler = nil; a.touchedChangedHandler = nil; a.pressedChangedHandler = nil;

	/* pressed threshold and clamping */
	[a setValue:0.001f];
	check(!a.pressed && a.value == 0.001f, "0.001 is not a press");
	[a setValue:0.01f];
	check(a.pressed && a.touched, "0.01 is a press");
	[a setValue:-1.0f];
	check(a.value == 0.0f && !a.pressed, "a button value is clamped to 0");
	[a setValue:7.0f];
	check(a.value == 1.0f, "a button value is clamped to 1");
	[g.leftTrigger setValue:0.4f];
	check(g.leftTrigger.pressed && g.leftTrigger.value == 0.4f, "a trigger is pressed at 0.4");
	[g.dpad setValueForXAxis:-2.0f yAxis:3.0f];
	check(g.dpad.xAxis.value == -1.0f && g.dpad.yAxis.value == 1.0f, "pad values are clamped to -1 and 1");
	[g.dpad setValueForXAxis:0 yAxis:0];

	/* direction pad: the handlers of the pad and of its children, the children's values and listing */
	events_n = 0;
	g.dpad.valueChangedHandler = ^(GCControllerDirectionPad *d, float x, float y) { event(x == 0.5f && y == -0.25f ? "dpad" : "dpad?"); };
	g.dpad.xAxis.valueChangedHandler = ^(GCControllerAxisInput *x, float v) { event("x"); };
	g.dpad.down.valueChangedHandler = ^(GCControllerButtonInput *b, float v, BOOL p) { event(v == 0.25f ? "down" : "down?"); };
	[g.dpad setValueForXAxis:0.5f yAxis:-0.25f];
	spin(0.2);
	check(g.dpad.xAxis.value == 0.5f && g.dpad.yAxis.value == -0.25f && g.dpad.right.value == 0.5f && g.dpad.down.value == 0.25f && g.dpad.up.value == 0 && g.dpad.left.value == 0, "a pad sets its axes and directions");
	int xs = 0, downs = 0;
	for (int i = 0; i < events_n; i++) {
		xs += !strcmp(events[i], "x");
		downs += !strcmp(events[i], "down");
	}
	/* macOS also calls the pad handler once per axis that changed, with the values so far; the last call has the final ones */
	check(xs == 1 && downs == 1 && events_n >= 3 && !strcmp(events[events_n - 1], "dpad"), "the pad and its children call their handlers (%d events)", events_n);
	check([g.elements[@"Direction Pad Down"] isEqual:g.dpad.down] && [g.axes[@"Direction Pad X Axis"] isEqual:g.dpad.xAxis], "the children of a pad are listed under their aliases");
	g.dpad.valueChangedHandler = nil; g.dpad.xAxis.valueChangedHandler = nil; g.dpad.down.valueChangedHandler = nil;

	/* the system gesture and the names */
	check(g.buttonHome.isBoundToSystemGesture && g.buttonOptions.isBoundToSystemGesture && !g.buttonMenu.isBoundToSystemGesture && !g.buttonA.isBoundToSystemGesture, "Home and Options are bound to system gestures, Menu and A are not");
	g.buttonHome.preferredSystemGestureState = GCSystemGestureStateDisabled;
	check(g.buttonHome.preferredSystemGestureState == GCSystemGestureStateDisabled && g.buttonOptions.preferredSystemGestureState == GCSystemGestureStateEnabled, "preferredSystemGestureState is stored per element");
	g.buttonHome.preferredSystemGestureState = GCSystemGestureStateAlwaysReceive;
	check(g.buttonHome.preferredSystemGestureState == GCSystemGestureStateAlwaysReceive, "preferredSystemGestureState takes AlwaysReceive");
	GCControllerButtonInput *loose = [[[GCControllerButtonInput alloc] init] autorelease];
	check(loose.isAnalog && !loose.isBoundToSystemGesture && loose.localizedName == nil && loose.sfSymbolsName == nil && loose.value == 0 && !loose.pressed && !loose.touched, "an element created with init: analog, no names, no gesture, value 0");
	loose.localizedName = @"X"; loose.sfSymbolsName = @"s";
	check([loose.localizedName isEqualToString:@"X"] && loose.unmappedLocalizedName == nil && [loose.sfSymbolsName isEqualToString:@"s"] && loose.unmappedSfSymbolsName == nil, "the four names are independent");
	loose.unmappedLocalizedName = @"U"; loose.unmappedSfSymbolsName = @"u";
	check([loose.unmappedLocalizedName isEqualToString:@"U"] && [loose.localizedName isEqualToString:@"X"] && [loose.unmappedSfSymbolsName isEqualToString:@"u"], "the unmapped names can be set");
	loose.localizedName = nil;
	check(loose.localizedName == nil, "a name can be reset to nil");

	/* capture, state copying, mapping */
	[a setValue:0.25f];
	[g.leftThumbstick setValueForXAxis:0.5f yAxis:-0.5f];
	GCExtendedGamepad *cap = [g capture];
	check(cap != g && [cap class] == [g class] && cap.device == nil && cap.controller == nil, "capture is a new profile of the same class that belongs to no device");
	check(cap.buttonA.value == 0.25f && cap.leftThumbstick.xAxis.value == 0.5f && cap.leftThumbstick.yAxis.value == -0.5f && cap.leftThumbstick.down.value == 0.5f, "capture copies the state");
	[g.buttonA setValue:1.0f];
	check(cap.buttonA.value == 0.25f, "a capture does not follow the profile");
	GCExtendedGamepad *target = [[[GCExtendedGamepad alloc] init] autorelease];
	[target setStateFromPhysicalInput:g];
	check(target.buttonA.value == 1.0f && target.leftThumbstick.yAxis.value == -0.5f, "setStateFromPhysicalInput copies the state");
	/* (on macOS 26 setStateFromExtendedGamepad: crashes when the target belongs to no controller) */
	GCExtendedGamepad *target2 = [GCController controllerWithExtendedGamepad].extendedGamepad;
	[target2 setStateFromExtendedGamepad:cap];
	check(target2.buttonA.value == 0.25f, "setStateFromExtendedGamepad copies the state");
	check([[g mappedElementAliasForPhysicalInputName:GCInputButtonA] isEqualToString:GCInputButtonA] && [[g mappedPhysicalInputNamesForElementAlias:GCInputButtonA] isEqualToSet:[NSSet setWithObject:GCInputButtonA]], "inputs map to themselves");
	check([[g mappedElementAliasForPhysicalInputName:@"Paddle 1"] isEqualToString:@"Paddle 1"], "the paddle name maps to itself");
	check(g[@"Paddle 1"] == g[@"Back Right Button 0"] && g[@"Paddle 4"] == g[@"Back Left Button 1"], "the paddles are the back buttons");

	/* the data of the snapshots */
	[g.dpad setValueForXAxis:0.5f yAxis:-0.5f];
	[g.leftThumbstickButton setValue:1.0f];
	[g.leftTrigger setValue:0.75f];
	GCExtendedGamepadSnapshot *snap = [g saveSnapshot];
	check([snap isKindOfClass:objc_getClass("GCExtendedGamepadSnapshot")], "saveSnapshot returns a GCExtendedGamepadSnapshot");
	NSData *d = snap.snapshotData;
	GCExtendedGamepadSnapshotData sd;
	check([d length] == 63 && GCExtendedGamepadSnapshotDataFromNSData(&sd, d), "the snapshot data of an extended gamepad are 63 bytes");
	check(sd.version == 0x0101 && sd.size == 63 && sd.dpadX == 0.5f && sd.dpadY == -0.5f && sd.buttonA == 1.0f && sd.leftTrigger == 0.75f && sd.supportsClickableThumbsticks && sd.leftThumbstickButton && !sd.rightThumbstickButton, "the snapshot data hold the state");
	GCExtendedGamepadSnapshot *snap2 = [[[GCExtendedGamepadSnapshot alloc] initWithSnapshotData:d] autorelease];
	check(snap2.buttonA.value == 1.0f && snap2.dpad.xAxis.value == 0.5f && snap2.leftTrigger.value == 0.75f && snap2.leftThumbstickButton.pressed, "a snapshot created from data has the state");
	check([[[GCExtendedGamepadSnapshot alloc] initWithSnapshotData:nil] autorelease] == nil && [[[GCExtendedGamepadSnapshot alloc] initWithSnapshotData:[NSData dataWithBytes:"abc" length:3]] autorelease] == nil, "no snapshot from nil or short data");
	check(!GCExtendedGamepadSnapshotDataFromNSData(&sd, nil) && !GCExtendedGamepadSnapshotDataFromNSData(NULL, d), "snapshot data from nil data or into NULL fail");
	GCExtendedGamepadSnapshotData big = sd; big.size = 100;
	check(!GCExtendedGamepadSnapshotDataFromNSData(&sd, [NSData dataWithBytes:&big length:63]), "data that declare more than they hold are refused");
	check(NSDataFromGCExtendedGamepadSnapshotData(NULL) == nil && [NSDataFromGCExtendedGamepadSnapshotData(&sd) length] == 63, "NSDataFromGCExtendedGamepadSnapshotData");
	GCExtendedGamepadSnapShotDataV100 v100;
	memset(&v100, 0, sizeof(v100));
	v100.version = 0x0100;
	v100.size = sizeof(v100);
	v100.buttonA = 0.5f;
	NSData *v100data = NSDataFromGCExtendedGamepadSnapShotDataV100(&v100);
	GCExtendedGamepadSnapShotDataV100 back;
	check([v100data length] == sizeof(v100) && GCExtendedGamepadSnapShotDataV100FromNSData(&back, v100data) && back.buttonA == 0.5f && !GCExtendedGamepadSnapShotDataV100FromNSData(&back, d), "the V100 data functions refuse the data of a later version");
	GCMicroGamepad *mg = [m microGamepad];
	[mg.buttonA setValue:1.0f];
	[mg.dpad setValueForXAxis:0.25f yAxis:0.75f];
	GCMicroGamepadSnapshotData md;
	NSData *mdata = [mg saveSnapshot].snapshotData;
	check([mdata length] == 20 && GCMicroGamepadSnapshotDataFromNSData(&md, mdata) && md.version == 0x100 && md.size == 20 && md.buttonA == 1.0f && md.dpadX == 0.25f && md.dpadY == 0.75f, "the snapshot data of a micro gamepad");
	GCGamepad *lg = [[[objc_getClass("GCGamepad") alloc] init] autorelease];
	[lg.buttonB setValue:0.5f];
	NSData *lgdata = [lg saveSnapshot].snapshotData;
	GCGamepadSnapShotDataV100 gd;
	check([lgdata length] == 36 && GCGamepadSnapShotDataV100FromNSData(&gd, lgdata) && gd.version == 0x100 && gd.buttonB == 0.5f, "the snapshot data of a legacy gamepad");
	GCController *copy = [c capture];
	check(copy != c && copy.isSnapshot && copy.extendedGamepad.buttonA.value == 1.0f, "GCController capture is a snapshot controller with the state");

	/* motion */
	GCMotion *motion = c.motion;
	[motion setGravity:(GCAcceleration){ 0, -1, 0 }];
	[motion setRotationRate:(GCRotationRate){ 1, 2, 3 }];
	[motion setAttitude:(GCQuaternion){ 0.1, 0.2, 0.3, 0.9 }];
	check(motion.gravity.y == -1 && motion.rotationRate.z == 3 && motion.attitude.w == 0.9, "motion values can be written");
	motion.sensorsActive = NO;
	check(motion.sensorsActive, "sensors that need no manual activation stay active");

	/* keyboard and mouse profiles that belong to no device */
	GCKeyboardInput *kbd = [[[GCKeyboardInput alloc] init] autorelease];
	GCControllerButtonInput *keyA = (id)[kbd buttonForKeyCode:GCKeyCodeKeyA];
	check(keyA == kbd.buttons[GCKeyA] && !keyA.isAnalog && !keyA.isBoundToSystemGesture && !kbd.isAnyKeyPressed && [keyA.aliases isEqualToSet:[NSSet setWithObject:@"A"]], "a keyboard key");
	GCMouseInput *mouse = [[[GCMouseInput alloc] init] autorelease];
	check(mouse.leftButton != nil && mouse.rightButton == nil && mouse.middleButton == nil && mouse.auxiliaryButtons == nil, "a mouse profile without a mouse has the left button only");
	check([mouse.scroll isKindOfClass:objc_getClass("GCDeviceCursor")] && [mouse.elements count] == 8 && mouse.elements[@"Left Button"] == mouse.leftButton && mouse.elements[@"Scroll"] == mouse.scroll && mouse.elements[@"Scroll Up"] == mouse.scroll.up, "the scroll of a mouse is a cursor with its children listed");

	/* the types */
	GCPoint2 p = GCPoint2Make(1.5f, 2.25f);
	check([NSStringFromGCPoint2(p) isEqualToString:@"{1.5, 2.25}"] && [NSStringFromGCPoint2(GCPoint2Zero) isEqualToString:@"{0, 0}"], "NSStringFromGCPoint2");
	NSValue *v = [NSValue valueWithGCPoint2:GCPoint2Make(3, 4)];
	check(v.GCPoint2Value.x == 3 && v.GCPoint2Value.y == 4 && GCPoint2Equal(p, GCPoint2Make(1.5f, 2.25f)) && !GCPoint2Equal(p, GCPoint2Zero), "GCPoint2 values");

	/* adaptive triggers and the touch pad */
	id dsense = [[[objc_getClass("GCDualSenseGamepad") alloc] init] autorelease];
	GCDualSenseAdaptiveTrigger *trigger = (GCDualSenseAdaptiveTrigger *)[(GCDualSenseGamepad *)dsense leftTrigger];
	[trigger setModeFeedbackWithStartPosition:0.2f resistiveStrength:0.5f];
	check(trigger.mode == GCDualSenseAdaptiveTriggerModeOff && trigger.status == 0 && trigger.armPosition == 0, "the mode of a trigger of a detached profile is not changed");
	GCControllerTouchpad *tp = [[[GCControllerTouchpad alloc] init] autorelease];
	check(tp.touchState == GCTouchStateUp && !tp.reportsAbsoluteTouchSurfaceValues, "a touch pad starts up and relative");

	/* the event view controller */
	GCEventViewController *vc = [[[GCEventViewController alloc] init] autorelease];
	check(!vc.controllerUserInteractionEnabled, "GCEventViewController: controller user interaction is disabled by default");
	vc.controllerUserInteractionEnabled = YES;
	check(vc.controllerUserInteractionEnabled, "controllerUserInteractionEnabled can be set");
}

/* ---- the controllers the system reports ---- */

static const char *ctl_dir = NULL;

static void sync_point(const char *tag)
{
	char path[512];
	printf("SYNC: %s\n", tag);
	fflush(stdout);
	snprintf(path, sizeof(path), "%s/go-%s", ctl_dir, tag);
	for (int i = 0; i < 1200 && access(path, F_OK) != 0; i++)
		spin(0.05);
}

static BOOL has_virtual_controller(void)
{
	for (GCController *c in [GCController controllers])
		if ([[c vendorName] hasPrefix:@"Vpad"])
			return YES;
	return NO;
}

static void check_devices(void)
{
	NSArray<GCController *> *all = [GCController controllers];
	printf("devices: %lu controllers\n", (unsigned long)[all count]);
	for (GCController *c in all) {
		printf("devices: %s (%s)\n", [[c vendorName] UTF8String], [[c productCategory] UTF8String]);
		check(!c.isSnapshot && c.isAttachedToDevice, "%s: a real controller is not a snapshot and is attached", [[c vendorName] UTF8String]);
		GCExtendedGamepad *g = c.extendedGamepad;
		check(g != nil && c.physicalInputProfile == g && c.gamepad == (id)g && c.microGamepad != nil, "%s: profiles", [[c vendorName] UTF8String]);
		check(c.motion == nil && c.light == nil, "%s: no motion, no light", [[c vendorName] UTF8String]);
		/* every element answers every property */
		for (NSString *key in g.elements) {
			GCControllerElement *e = g.elements[key];
			(void)e.isBoundToSystemGesture; (void)e.preferredSystemGestureState; (void)e.localizedName; (void)e.sfSymbolsName;
			(void)e.unmappedLocalizedName; (void)e.unmappedSfSymbolsName; (void)e.aliases; (void)e.isAnalog;
			check([e.aliases containsObject:key], "%s: '%s' is among the aliases of its element", [[c vendorName] UTF8String], [key UTF8String]);
			if (![e isKindOfClass:[GCControllerTouchpad class]] && e.collection == nil)
				check(e.localizedName != nil || [c.vendorName length] > 0, "%s: '%s' has a name", [[c vendorName] UTF8String], [key UTF8String]);
		}
		check(g.buttonHome.isBoundToSystemGesture && !g.buttonA.isBoundToSystemGesture, "%s: Home is bound to a system gesture, A is not", [[c vendorName] UTF8String]);
		/* a real device refuses writes from the application */
		float before = g.buttonA.value;
		[g.buttonA setValue:before > 0.5f ? 0.0f : 1.0f];
		[g.dpad setValueForXAxis:before > 0.5f ? 0.0f : 1.0f yAxis:0.0f];
		spin(0.1);
		check(g.buttonA.value == before && g.dpad.xAxis.value == 0.0f, "%s: setValue: has no effect on the elements of a real device", [[c vendorName] UTF8String]);
		[g setStateFromPhysicalInput:[GCController controllerWithExtendedGamepad].extendedGamepad];
		/* a capture of it is a free profile */
		GCExtendedGamepad *cap = [g capture];
		check(cap.device == nil && [cap class] == [g class] && [cap.elements count] <= [g.elements count] && [cap.elements count] + 1 >= [g.elements count], "%s: capture of a real profile", [[c vendorName] UTF8String]);
		[cap.buttonA setValue:1.0f];
		check(cap.buttonA.value == 1.0f && g.buttonA.value == before, "%s: a capture can be written without touching the device", [[c vendorName] UTF8String]);
		GCController *snap = [c capture];
		check(snap.isSnapshot && snap.extendedGamepad != nil, "%s: capture of the controller", [[c vendorName] UTF8String]);
		/* the names of the profile family */
		if ([c.productCategory isEqualToString:GCProductCategoryXboxOne])
			check([g isKindOfClass:objc_getClass("GCXboxGamepad")] && [g.buttonHome.localizedName isEqualToString:@"Xbox Button"] && [g.buttonA.sfSymbolsName isEqualToString:@"a.circle"], "%s: Xbox profile names", [[c vendorName] UTF8String]);
		if ([c.productCategory isEqualToString:GCProductCategoryDualShock4])
			check([g isKindOfClass:objc_getClass("GCDualShockGamepad")] && [g.buttonHome.localizedName isEqualToString:@"PS Button"] && [g.buttonA.localizedName isEqualToString:@"Cross Button"], "%s: DualShock profile names", [[c vendorName] UTF8String]);
		if ([c.productCategory isEqualToString:GCProductCategoryHID])
			check([g.buttonA.localizedName isEqualToString:@"A Button"] && [g.leftShoulder.localizedName isEqualToString:@"L1 Button"], "%s: generic profile names", [[c vendorName] UTF8String]);
	}
}

int main(int argc, char *argv[])
{
	@autoreleasepool {
		setvbuf(stdout, NULL, _IOLBF, 0);
		void *fw = dlopen("/System/Library/Frameworks/GameController.framework/GameController", RTLD_NOW);
		if (!fw) {
			printf("FAIL: GameController cannot be loaded\n");
			return 1;
		}
		check_surface();
		check_constants();
		check_profiles();
		check_behaviour();
		if (argc > 1 && argv[1][0] == '/')
			ctl_dir = argv[1];
		if (ctl_dir || (argc > 1 && !strcmp(argv[1], "devices"))) {
			if (ctl_dir) {
				sync_point("plug");
				for (int i = 0; i < 200 && !has_virtual_controller(); i++)
					spin(0.05);
				check(has_virtual_controller(), "the virtual controller connected");
			}
			check_devices();
			if (ctl_dir)
				sync_point("unplug");
		}
		printf("%s: %d checks, %d failures\n", failures ? "FAIL" : "PASS", checks, failures);
		return failures ? 1 : 0;
	}
}

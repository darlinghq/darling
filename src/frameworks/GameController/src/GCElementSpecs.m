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

// Names, symbols and defaults of the elements of the profile families, as macOS reports them for a
// profile of each family. Only the elements that are not the children of a direction pad are listed;
// the children (axes and the four directions) are derived by GCApplyElementSpecs.
#ifdef __LP64__
#import "GCInputInternal.h"

static const GCElementSpec GCElementSpecsGeneric[] = {
	{ "Back Left Button 0", "M2 Button", "m2.button.horizontal", 1, 0 },
	{ "Back Left Button 1", "M4 Button", "m4.button.horizontal", 1, 0 },
	{ "Back Right Button 0", "M1 Button", "m1.button.horizontal", 1, 0 },
	{ "Back Right Button 1", "M3 Button", "m3.button.horizontal", 1, 0 },
	{ "Button A", "A Button", "a.circle", 1, 0 },
	{ "Button B", "B Button", "b.circle", 1, 0 },
	{ "Button Home", "Home Button", "house.circle", 1, 1 },
	{ "Button Menu", "Menu Button", "line.horizontal.3.circle", 1, 0 },
	{ "Button Options", "Options Button", "ellipsis.circle", 1, 1 },
	{ "Button X", "X Button", "x.circle", 1, 0 },
	{ "Button Y", "Y Button", "y.circle", 1, 0 },
	{ "Direction Pad", "Direction Pad", "dpad", 1, 0 },
	{ "Left Bumper", "L4 Button", "l4.button.horizontal", 1, 0 },
	{ "Left Shoulder", "L1 Button", "l1.rectangle.roundedbottom", 1, 0 },
	{ "Left Thumbstick", "Left Thumbstick", "l.joystick", 1, 0 },
	{ "Left Thumbstick Button", "Left Thumbstick Button", "l.joystick.down", 1, 0 },
	{ "Left Trigger", "L2 Button", "l2.rectangle.roundedtop", 1, 0 },
	{ "Right Bumper", "R4 Button", "r4.button.horizontal", 1, 0 },
	{ "Right Shoulder", "R1 Button", "r1.rectangle.roundedbottom", 1, 0 },
	{ "Right Thumbstick", "Right Thumbstick", "r.joystick", 1, 0 },
	{ "Right Thumbstick Button", "Right Thumbstick Button", "r.joystick.press.down", 1, 0 },
	{ "Right Trigger", "R2 Button", "r2.rectangle.roundedtop", 1, 0 },
};

static const GCElementSpec GCElementSpecsXbox[] = {
	{ "Back Left Button 0", "M2 Button", "m2.button.horizontal", 1, 0 },
	{ "Back Left Button 1", "M4 Button", "m4.button.horizontal", 1, 0 },
	{ "Back Right Button 0", "M1 Button", "m1.button.horizontal", 1, 0 },
	{ "Back Right Button 1", "M3 Button", "m3.button.horizontal", 1, 0 },
	{ "Button A", "A Button", "a.circle", 1, 0 },
	{ "Button B", "B Button", "b.circle", 1, 0 },
	{ "Button Home", "Xbox Button", "logo.xbox", 1, 1 },
	{ "Button Menu", "Menu Button", "line.horizontal.3.circle", 1, 0 },
	{ "Button Options", "View Button", "rectangle.fill.on.rectangle.fill.circle", 1, 1 },
	{ "Button X", "X Button", "x.circle", 1, 0 },
	{ "Button Y", "Y Button", "y.circle", 1, 0 },
	{ "Direction Pad", "D-pad", "dpad", 1, 0 },
	{ "Left Bumper", "L4 Button", "l4.button.horizontal", 1, 0 },
	{ "Left Shoulder", "Left Bumper", "lb.rectangle.roundedbottom", 1, 0 },
	{ "Left Thumbstick", "Left Stick", "l.joystick", 1, 0 },
	{ "Left Thumbstick Button", "Left Stick Click", "l.joystick.down", 1, 0 },
	{ "Left Trigger", "Left Trigger", "lt.rectangle.roundedtop", 1, 0 },
	{ "Right Bumper", "R4 Button", "r4.button.horizontal", 1, 0 },
	{ "Right Shoulder", "Right Bumper", "rb.rectangle.roundedbottom", 1, 0 },
	{ "Right Thumbstick", "Right Stick", "r.joystick", 1, 0 },
	{ "Right Thumbstick Button", "Right Stick Click", "r.joystick.press.down", 1, 0 },
	{ "Right Trigger", "Right Trigger", "rt.rectangle.roundedtop", 1, 0 },
};

static const GCElementSpec GCElementSpecsDualShock[] = {
	{ "Back Left Button 0", "M2 Button", "m2.button.horizontal", 1, 0 },
	{ "Back Left Button 1", "M4 Button", "m4.button.horizontal", 1, 0 },
	{ "Back Right Button 0", "M1 Button", "m1.button.horizontal", 1, 0 },
	{ "Back Right Button 1", "M3 Button", "m3.button.horizontal", 1, 0 },
	{ "Button A", "Cross Button", "xmark.circle", 1, 0 },
	{ "Button B", "Circle Button", "circle.circle", 1, 0 },
	{ "Button Home", "PS Button", "logo.playstation", 1, 1 },
	{ "Button Menu", "OPTIONS Button", "capsule.portrait", 1, 0 },
	{ "Button Options", "SHARE Button", "capsule.portrait", 1, 1 },
	{ "Button X", "Square Button", "square.circle", 1, 0 },
	{ "Button Y", "Triangle Button", "triangle.circle", 1, 0 },
	{ "Direction Pad", "Direction Pad", "dpad", 1, 0 },
	{ "Left Bumper", "L4 Button", "l4.button.horizontal", 1, 0 },
	{ "Left Shoulder", "L1 Button", "l1.rectangle.roundedbottom", 1, 0 },
	{ "Left Thumbstick", "Left Stick", "l.joystick", 1, 0 },
	{ "Left Thumbstick Button", "L3 Button", "l.joystick.down", 1, 0 },
	{ "Left Trigger", "L2 Button", "l2.rectangle.roundedtop", 1, 0 },
	{ "Right Bumper", "R4 Button", "r4.button.horizontal", 1, 0 },
	{ "Right Shoulder", "R1 Button", "r1.rectangle.roundedbottom", 1, 0 },
	{ "Right Thumbstick", "Right Stick", "r.joystick", 1, 0 },
	{ "Right Thumbstick Button", "R3 Button", "r.joystick.press.down", 1, 0 },
	{ "Right Trigger", "R2 Button", "r2.rectangle.roundedtop", 1, 0 },
	{ "Touchpad 1", "Touchpad (First Finger)", "hand.draw", 1, 0 },
	{ "Touchpad 2", "Touchpad (Second Finger)", "hand.draw", 1, 0 },
	{ "Touchpad Button", "Touchpad Button", "plus.rectangle", 0, 0 },
};

static const GCElementSpec GCElementSpecsDualSense[] = {
	{ "Button A", "Cross Button", "xmark.circle", 1, 0 },
	{ "Button B", "Circle Button", "circle.circle", 1, 0 },
	{ "Button Home", "PS Button", "logo.playstation", 1, 1 },
	{ "Button Menu", "Options Button", "capsule.portrait", 1, 0 },
	{ "Button Options", "Create Button", "capsule.portrait", 1, 1 },
	{ "Button X", "Square Button", "square.circle", 1, 0 },
	{ "Button Y", "Triangle Button", "triangle.circle", 1, 0 },
	{ "Direction Pad", "Direction Pad", "dpad", 1, 0 },
	{ "Left Shoulder", "L1 Button", "l1.rectangle.roundedbottom", 1, 0 },
	{ "Left Thumbstick", "Left Stick", "l.joystick", 1, 0 },
	{ "Left Thumbstick Button", "L3 Button", "l.joystick.down", 1, 0 },
	{ "Left Trigger", "L2 Button", "l2.rectangle.roundedtop", 1, 0 },
	{ "Right Shoulder", "R1 Button", "r1.rectangle.roundedbottom", 1, 0 },
	{ "Right Thumbstick", "Right Stick", "r.joystick", 1, 0 },
	{ "Right Thumbstick Button", "R3 Button", "r.joystick.press.down", 1, 0 },
	{ "Right Trigger", "R2 Button", "r2.rectangle.roundedtop", 1, 0 },
	{ "Touchpad 1", "Touchpad (First Finger)", "hand.draw", 1, 0 },
	{ "Touchpad 2", "Touchpad (Second Finger)", "hand.draw", 1, 0 },
	{ "Touchpad Button", "Touchpad Button", "plus.rectangle", 0, 0 },
};

static const GCElementSpec GCElementSpecsMicro[] = {
	{ "Button A", "Touch Surface Button", NULL, 0, 0 },
	{ "Button Menu", "Menu Button", NULL, 0, 0 },
	{ "Button X", "Play/Pause Button", NULL, 0, 0 },
	{ "Direction Pad", "Touch Surface", NULL, 1, 0 },
};

const GCElementSpec *GCElementSpecsForStyle(GCProfileStyle style, size_t *count)
{
	switch (style) {
		case GCProfileStyleGeneric: *count = sizeof(GCElementSpecsGeneric) / sizeof(GCElementSpecsGeneric[0]); return GCElementSpecsGeneric;
		case GCProfileStyleXbox: *count = sizeof(GCElementSpecsXbox) / sizeof(GCElementSpecsXbox[0]); return GCElementSpecsXbox;
		case GCProfileStyleDualShock: *count = sizeof(GCElementSpecsDualShock) / sizeof(GCElementSpecsDualShock[0]); return GCElementSpecsDualShock;
		case GCProfileStyleDualSense: *count = sizeof(GCElementSpecsDualSense) / sizeof(GCElementSpecsDualSense[0]); return GCElementSpecsDualSense;
		case GCProfileStyleMicro: *count = sizeof(GCElementSpecsMicro) / sizeof(GCElementSpecsMicro[0]); return GCElementSpecsMicro;
	}
	*count = 0;
	return NULL;
}

#endif /* __LP64__ */

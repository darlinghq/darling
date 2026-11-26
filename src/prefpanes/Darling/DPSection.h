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

#import "DPSetting.h"

// A group of settings, shown as one section of the pane.
@interface DPSection : NSObject
@property (copy) NSString *identifier;       // the anchor that selects the section
@property (copy) NSString *title;            // the section's title in the list
@property (copy) NSString *note;             // when the settings take effect, shown above them
// The user settings of a section are written as they change; the system-wide ones are edited
// first and written by Apply.
@property (copy) NSArray<DPSetting *> *userSettings;
@property (copy) NSArray<DPSetting *> *systemSettings;
@property (copy) NSString *systemNote;       // shown above the system settings

// All the sections, in list order.
+ (NSArray<DPSection *> *)allSections;

// The key of the setting that the legacy boolean DarlingInWindowMenuBar stands for
+ (DPSetting *)menuBarStyleSetting;
// The per-application overrides: the settings that an application's own domain can override
+ (NSArray<DPSetting *> *)applicationSettings;
@end

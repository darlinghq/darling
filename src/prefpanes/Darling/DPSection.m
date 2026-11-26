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

#import "DPSection.h"

static NSString * const kAppKit = @"org.darlinghq.AppKit";
static NSString * const kPlatform = @"org.darlinghq.platform";
static NSString * const kSecurity = @"org.darlinghq.security";

@implementation DPSection

static DPSetting *Bool(NSString *key, NSString *domain, DPScope scope, NSString *title, NSString *help, NSString *defaultText)
{
    return [DPSetting settingWithKey:key domain:domain kind:DPKindBool scope:scope title:title help:help defaultText:defaultText];
}

static DPSetting *Choice(NSString *key, NSString *domain, DPScope scope, NSString *title, NSString *help, NSString *defaultText,
                         NSArray *choices, NSArray *titles)
{
    DPSetting *s = [DPSetting settingWithKey:key domain:domain kind:DPKindChoice scope:scope title:title help:help defaultText:defaultText];
    s.choices = choices;
    s.choiceTitles = titles;
    return s;
}

// A printable-ASCII string of the platform identity (at most 63 characters)
static DPSetting *Identity(NSString *key, NSString *title, NSString *help, NSString *placeholder)
{
    DPSetting *s = [DPSetting settingWithKey:key domain:kPlatform kind:DPKindText scope:DPScopeSystem title:title help:help
                                 defaultText:NSLocalizedString(@"as the host reports", nil)];
    s.asciiOnly = YES;
    s.stringLimit = 64;
    s.placeholder = placeholder;
    return s;
}

+ (DPSetting *)menuBarStyleSetting
{
    return Choice(@"DarlingMenuBarStyle", kAppKit, DPScopeUser,
        NSLocalizedString(@"Main menu", nil),
        NSLocalizedString(@"Where an application's main menu is shown: in a strip inside each window, in the desktop's global menu or a bar along the top of the screen, or by the desktop's own convention (automatic). Read when an application starts.", nil),
        NSLocalizedString(@"automatic", nil),
        @[ @"window", @"screen", @"auto" ],
        @[ NSLocalizedString(@"Inside each window", nil), NSLocalizedString(@"On the screen", nil), NSLocalizedString(@"Automatic", nil) ]);
}

+ (DPSetting *)terminateSetting
{
    return Bool(@"TerminateAfterLastWindowClosed", kAppKit, DPScopeUser,
        NSLocalizedString(@"Quit when the last window is closed", nil),
        NSLocalizedString(@"Linux desktops have no Dock, so an application without windows can look hung or invisible. With this on, an application quits when the user closes its last window, unless the application decides that itself. Applications with a status item and background applications are left alone.", nil),
        NSLocalizedString(@"Off", nil));
}

+ (NSArray<DPSetting *> *)applicationSettings
{
    return @[
        [self terminateSetting],
        [self menuBarStyleSetting],
        Bool(@"DesktopFileChooser", kAppKit, DPScopeUser, NSLocalizedString(@"Desktop file chooser", nil), nil, NSLocalizedString(@"Off", nil)),
        Bool(@"CAMetalLayerForceOpenGL", @"org.darlinghq.QuartzCore", DPScopeUser, NSLocalizedString(@"Metal layers through OpenGL", nil), nil, NSLocalizedString(@"Off", nil)),
    ];
}

+ (NSArray<DPSection *> *)allSections
{
    static NSArray *sections;
    static dispatch_once_t once;
    dispatch_once(&once, ^{
        NSMutableArray *list = [NSMutableArray array];
        DPSection *section;

        // Keyboard and input
        section = [[DPSection alloc] init];
        section.identifier = @"keyboard";
        section.title = NSLocalizedString(@"Keyboard", nil);
        section.note = NSLocalizedString(@"Applications read these settings when they start: a change applies to applications started afterwards. The key repeat, the function keys and the keyboard navigation are in the Keyboard pane of System Preferences.", nil);
        section.userSettings = @[
            Bool(@"ControlKeyActsAsCommand", kAppKit, DPScopeUser,
                 NSLocalizedString(@"Control acts as Command", nil),
                 NSLocalizedString(@"Command always comes from the Super key. With this on, Control+C, Control+V and Control+X also work as Command+C, V and X, and menus show those three shortcuts with Ctrl. Only the X11 display backend has this.", nil),
                 NSLocalizedString(@"Off", nil)),
            Bool(@"ConfinePointerToFullscreenWindows", kAppKit, DPScopeUser,
                 NSLocalizedString(@"Keep the pointer in full-screen windows", nil),
                 NSLocalizedString(@"The pointer cannot leave a full-screen window while it is on. Only the X11 display backend has this.", nil),
                 NSLocalizedString(@"Off", nil)),
        ];
        [list addObject:section];

        // Display and windows
        section = [[DPSection alloc] init];
        section.identifier = @"display";
        section.title = NSLocalizedString(@"Display", nil);
        section.note = NSLocalizedString(@"Applications read these settings when they start: a change applies to applications started afterwards. The environment variables DARLING_APPKIT_BACKEND and DARLING_WAYLAND_DECORATIONS override the first two.", nil);
        DPSetting *scale = [DPSetting settingWithKey:@"X11BackingScale" domain:kAppKit kind:DPKindReal scope:DPScopeUser
            title:NSLocalizedString(@"Scale on X11", nil)
            help:NSLocalizedString(@"Device pixels per point (HiDPI) on the X11 display backend, 1 or more. Empty: the session's GDK_SCALE if it has one, else 1.", nil)
            defaultText:NSLocalizedString(@"automatic", nil)];
        scale.minimum = 1;
        scale.placeholder = NSLocalizedString(@"automatic", nil);
        section.userSettings = @[
            Choice(@"DisplayBackend", kAppKit, DPScopeUser,
                   NSLocalizedString(@"Display backend", nil),
                   NSLocalizedString(@"Which window system applications use. Automatic uses Wayland when a Wayland compositor is available and X11 otherwise.", nil),
                   NSLocalizedString(@"automatic", nil),
                   @[ @"x11", @"wayland" ], @[ @"X11", @"Wayland" ]),
            Choice(@"WaylandDecorations", kAppKit, DPScopeUser,
                   NSLocalizedString(@"Window frames on Wayland", nil),
                   NSLocalizedString(@"By default the compositor draws the frames when it offers to. With this set, applications draw them.", nil),
                   NSLocalizedString(@"compositor", nil),
                   @[ @"client" ], @[ NSLocalizedString(@"Drawn by the application", nil) ]),
            scale,
            [self menuBarStyleSetting],
        ];
        [list addObject:section];

        // File dialogs
        section = [[DPSection alloc] init];
        section.identifier = @"files";
        section.title = NSLocalizedString(@"Files", nil);
        section.note = NSLocalizedString(@"Applications read this setting when they start: a change applies to applications started afterwards. An application's own value overrides it, see Applications.", nil);
        section.userSettings = @[
            Bool(@"DesktopFileChooser", kAppKit, DPScopeUser,
                 NSLocalizedString(@"Use the desktop's file chooser", nil),
                 NSLocalizedString(@"Show open and save panels with the desktop's own file chooser (through the xdg-desktop-portal FileChooser interface) instead of Darling's panel. Panels the portal cannot show, such as those with accessory views, still use Darling's panel. The chooser hands the chosen files to the application, so this is opt-in.", nil),
                 NSLocalizedString(@"Off", nil)),
        ];
        [list addObject:section];

        // Graphics
        section = [[DPSection alloc] init];
        section.identifier = @"graphics";
        section.title = NSLocalizedString(@"Graphics", nil);
        section.note = NSLocalizedString(@"Applications read these settings when they start: a change applies to applications started afterwards.", nil);
        section.userSettings = @[
            Bool(@"DMABuf", @"org.darlinghq.iosurface", DPScopeUser,
                 NSLocalizedString(@"Share surfaces as dmabufs", nil),
                 NSLocalizedString(@"IOSurfaces are shared with the host as dmabufs where the host supports it. Turn this off to keep them in ordinary memory.", nil),
                 NSLocalizedString(@"On", nil)),
            Bool(@"CAMetalLayerForceOpenGL", @"org.darlinghq.QuartzCore", DPScopeUser,
                 NSLocalizedString(@"Present Metal layers through OpenGL", nil),
                 NSLocalizedString(@"Metal layers are composited through the OpenGL path instead of a Vulkan swapchain on the window. An application's own value overrides it, see Applications.", nil),
                 NSLocalizedString(@"Off", nil)),
        ];
        section.systemNote = NSLocalizedString(@"What Metal and the display report. These are system-wide settings (for every user): they are edited first and written with Apply.", nil);
        DPSetting *memory = [DPSetting settingWithKey:@"GPUMemoryMB" domain:kPlatform kind:DPKindInteger scope:DPScopeSystem
            title:NSLocalizedString(@"Reported GPU memory", nil)
            help:NSLocalizedString(@"The working set size Metal reports for the default device, in MB.", nil)
            defaultText:NSLocalizedString(@"as the device reports", nil)];
        memory.minimum = 1;
        memory.unit = @"MB";
        DPSetting *gpuName = [DPSetting settingWithKey:@"GPUName" domain:kPlatform kind:DPKindText scope:DPScopeSystem
            title:NSLocalizedString(@"Reported GPU name", nil)
            help:NSLocalizedString(@"The name Metal reports for the default device.", nil)
            defaultText:NSLocalizedString(@"as the device reports", nil)];
        DPSetting *preferred = [DPSetting settingWithKey:@"PreferredGPU" domain:kPlatform kind:DPKindText scope:DPScopeSystem
            title:NSLocalizedString(@"Preferred GPU", nil)
            help:NSLocalizedString(@"Which GPU MTLCreateSystemDefaultDevice returns: the index of a device in MTLCopyAllDevices, or a part of its name.", nil)
            defaultText:NSLocalizedString(@"first device", nil)];
        preferred.placeholder = NSLocalizedString(@"index or part of the name", nil);
        section.systemSettings = @[
            preferred, gpuName, memory,
            Bool(@"GPUHasUnifiedMemory", kPlatform, DPScopeSystem,
                 NSLocalizedString(@"GPU has unified memory", nil),
                 NSLocalizedString(@"Whether Metal reports the devices as having unified memory.", nil),
                 NSLocalizedString(@"On", nil)),
            Bool(@"BuiltinDisplay", kPlatform, DPScopeSystem,
                 NSLocalizedString(@"Main display is built-in", nil),
                 NSLocalizedString(@"Report the main display as the built-in panel. X11 has no reliable notion of one, so by default none is built-in.", nil),
                 NSLocalizedString(@"Off", nil)),
        ];
        [list addObject:section];

        // Sound: the section builds its own view (DPSound.m) from the HAL
        section = [[DPSection alloc] init];
        section.identifier = @"sound";
        section.title = NSLocalizedString(@"Sound", nil);
        section.note = @"";
        section.userSettings = @[];
        [list addObject:section];

        // Platform identity
        section = [[DPSection alloc] init];
        section.identifier = @"identity";
        section.title = NSLocalizedString(@"Identity", nil);
        section.note = NSLocalizedString(@"What the system reports about the Mac it pretends to be. Nothing here is set unless you set it and click Apply, and clearing a field goes back to what the host reports. Changes are system-wide, written with Apply, and apply to the running system within moments (darling-prefsd pushes them to darlingserver).", nil);
        DPSetting *memorySize = [DPSetting settingWithKey:@"MemorySize" domain:kPlatform kind:DPKindInteger scope:DPScopeSystem
            title:NSLocalizedString(@"Memory", nil)
            help:NSLocalizedString(@"The amount of memory the system reports (hw.memsize), in GiB. Fractions are allowed.", nil)
            defaultText:NSLocalizedString(@"as the host reports", nil)];
        memorySize.scale = 1073741824.0;
        memorySize.minimum = 0;
        memorySize.unit = @"GiB";
        DPSetting *platformUUID = [DPSetting settingWithKey:@"PlatformUUID" domain:kPlatform kind:DPKindUUID scope:DPScopeSystem
            title:NSLocalizedString(@"Platform UUID", nil)
            help:NSLocalizedString(@"The hardware UUID (IOPlatformUUID, gethostuuid).", nil)
            defaultText:NSLocalizedString(@"as the host reports", nil)];
        DPSetting *udid = [DPSetting settingWithKey:@"ProvisioningUDID" domain:kPlatform kind:DPKindUUID scope:DPScopeSystem
            title:NSLocalizedString(@"Provisioning UDID", nil)
            help:NSLocalizedString(@"The UDID provisioning checks use. It must equal the platform UUID.", nil)
            defaultText:NSLocalizedString(@"as the host reports", nil)];
        DPSetting *rom = [DPSetting settingWithKey:@"ROM" domain:kPlatform kind:DPKindHex scope:DPScopeSystem
            title:NSLocalizedString(@"ROM", nil)
            help:NSLocalizedString(@"6 bytes, as hexadecimal digits.", nil)
            defaultText:NSLocalizedString(@"as the host reports", nil)];
        rom.byteLength = 6;
        DPSetting *systemID = [DPSetting settingWithKey:@"SystemID" domain:kPlatform kind:DPKindHex scope:DPScopeSystem
            title:NSLocalizedString(@"System ID", nil)
            help:NSLocalizedString(@"16 bytes, as hexadecimal digits.", nil)
            defaultText:NSLocalizedString(@"as the host reports", nil)];
        systemID.byteLength = 16;
        section.systemSettings = @[
            Identity(@"Model", NSLocalizedString(@"Model", nil), NSLocalizedString(@"The model identifier (hw.model), for example iMac20,1.", nil), nil),
            Identity(@"ProductName", NSLocalizedString(@"Product name", nil), nil, nil),
            Identity(@"BoardID", NSLocalizedString(@"Board ID", nil), nil, nil),
            Identity(@"SerialNumber", NSLocalizedString(@"Serial number", nil), nil, nil),
            Identity(@"MLBSerialNumber", NSLocalizedString(@"Logic board serial number", nil), nil, nil),
            Identity(@"CPUBrandString", NSLocalizedString(@"Processor", nil), NSLocalizedString(@"The processor's name (machdep.cpu.brand_string).", nil), nil),
            memorySize, platformUUID, udid, rom, systemID,
        ];
        [list addObject:section];

        // Security
        section = [[DPSection alloc] init];
        section.identifier = @"security";
        section.title = NSLocalizedString(@"Security", nil);
        section.note = NSLocalizedString(@"Nothing is enforced by default: these settings exist for compatibility, and every value shown as the default is what applies until you set it and click Apply. Changes are system-wide and written with Apply. The first four apply to the running system within moments (darling-prefsd pushes them to darlingserver); the host certificate import happens when the system starts.", nil);
        DPSetting *trust = [DPSetting settingWithKey:@"TrustStorePath" domain:kSecurity kind:DPKindPath scope:DPScopeSystem
            title:NSLocalizedString(@"Trust store", nil)
            help:NSLocalizedString(@"Absolute path, inside the prefix, of the signed trust cache of code directory hashes.", nil)
            defaultText:NSLocalizedString(@"/private/var/db/darling/trustcache", nil)];
        trust.stringLimit = 1024;
        section.systemSettings = @[
            Choice(@"CodeSigningPolicy", kSecurity, DPScopeSystem,
                   NSLocalizedString(@"Code signing policy", nil),
                   NSLocalizedString(@"Off: signatures are not checked. Warn: problems are reported. Enforce: unsigned or untrusted code is refused.", nil),
                   NSLocalizedString(@"off", nil),
                   @[ @"off", @"warn", @"enforce" ],
                   @[ NSLocalizedString(@"Off", nil), NSLocalizedString(@"Warn", nil), NSLocalizedString(@"Enforce", nil) ]),
            Bool(@"SealImages", kSecurity, DPScopeSystem,
                 NSLocalizedString(@"Seal loaded images", nil),
                 NSLocalizedString(@"Make the read-only code and link-edit data of the images of hardened programs immutable, so they cannot be remapped, made writable or unmapped. Needs a host kernel that can seal memory.", nil),
                 NSLocalizedString(@"Off", nil)),
            Bool(@"DisablePointerAuthentication", kSecurity, DPScopeSystem,
                 NSLocalizedString(@"Disable pointer authentication", nil),
                 NSLocalizedString(@"Start Darwin processes with arm64 pointer authentication turned off, where the host supports it. Intel programs are not affected.", nil),
                 NSLocalizedString(@"Off", nil)),
            trust,
            Bool(@"ImportHostCertificates", kSecurity, DPScopeSystem,
                 NSLocalizedString(@"Import the host's certificates", nil),
                 NSLocalizedString(@"Import the host's CA bundle into the system keychain when the system starts.", nil),
                 NSLocalizedString(@"On", nil)),
        ];
        [list addObject:section];

        // Metal diagnostics
        section = [[DPSection alloc] init];
        section.identifier = @"metal";
        section.title = NSLocalizedString(@"Metal", nil);
        section.note = NSLocalizedString(@"Applications read these settings when they start: a change applies to applications started afterwards. The summaries are printed to the standard error output when the application exits.", nil);
        section.userSettings = @[
            Bool(@"LogStatistics", @"org.darlinghq.Metal", DPScopeUser,
                 NSLocalizedString(@"Log allocation statistics", nil),
                 NSLocalizedString(@"Print allocation and heap statistics when an application exits.", nil),
                 NSLocalizedString(@"Off", nil)),
            Bool(@"PipelineTiming", @"org.darlinghq.Metal", DPScopeUser,
                 NSLocalizedString(@"Time pipelines", nil),
                 NSLocalizedString(@"Measure the GPU time of each pipeline and include it in that summary.", nil),
                 NSLocalizedString(@"Off", nil)),
        ];
        [list addObject:section];

        // Application lifecycle (the section's own controls are the global value and the per-application list)
        section = [[DPSection alloc] init];
        section.identifier = @"applications";
        section.title = NSLocalizedString(@"Applications", nil);
        section.note = NSLocalizedString(@"Applications read these settings when they start: a change applies to applications started afterwards.", nil);
        section.userSettings = @[ [self terminateSetting] ];
        [list addObject:section];

        sections = list;
    });
    return sections;
}

@end

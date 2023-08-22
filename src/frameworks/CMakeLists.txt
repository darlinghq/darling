project(frameworks)

if (COMPONENT_system)
	set(CMAKE_INSTALL_DEFAULT_COMPONENT_NAME "system")

	# for memberd
	add_subdirectory(DirectoryServices)
endif()

# this is mainly for frameworks that are required for Security,
# since Security is built for both CLI and GUI
if (COMPONENT_cli OR COMPONENT_dev_gui_common)
	set(CMAKE_INSTALL_DEFAULT_COMPONENT_NAME "cli_gui_common")

	add_subdirectory(CoreServices)

	# these are also stubs, but they're needed for Security
	add_subdirectory(Accounts)
	add_subdirectory(CloudKit)
	add_subdirectory(CryptoTokenKit)
	add_subdirectory(LocalAuthentication)
	add_subdirectory(NetFS)
	add_subdirectory(Network)
	add_subdirectory(OpenDirectory)
	add_subdirectory(SecurityFoundation)
	add_subdirectory(SystemConfiguration)

	# this is for Java (obviously)
	add_subdirectory(JavaVM)
	add_subdirectory(JavaRuntimeSupport)
endif()

if (COMPONENT_iokitd OR COMPONENT_dev_gui_common)
	set(CMAKE_INSTALL_DEFAULT_COMPONENT_NAME "iosurface")

	# this is for iokitd (and Xcode)
	add_subdirectory(IOSurface)
endif()

# this is mainly for anything that Xcode requires to run on the CLI
if (COMPONENT_dev_gui_common)
	set(CMAKE_INSTALL_DEFAULT_COMPONENT_NAME "cli_dev_gui_common")

	add_subdirectory(ApplicationServices)
	add_subdirectory(ColorSync)
	add_subdirectory(Carbon)

	# stubs, but required for QuartzCore (in Cocotron)
	add_subdirectory(CoreImage)
	add_subdirectory(CoreVideo)
endif()

# same here, except this is for stubs that Xcode needs
if (COMPONENT_dev_gui_stubs_common)
	set(CMAKE_INSTALL_DEFAULT_COMPONENT_NAME "cli_dev_gui_stubs_common")

	add_subdirectory(Accelerate)
	add_subdirectory(AVFoundation)
	add_subdirectory(Contacts)
	add_subdirectory(CoreMedia)
	add_subdirectory(DiskArbitration)
	add_subdirectory(GLKit)
	add_subdirectory(ScriptingBridge)
	add_subdirectory(SecurityInterface)
	add_subdirectory(ServiceManagement)
	add_subdirectory(Quartz)
	add_subdirectory(QuickLook)
	add_subdirectory(UniformTypeIdentifiers)
	add_subdirectory(UserNotifications)
	add_subdirectory(VideoToolbox)
endif()

# this is for core GUI frameworks with actual implementations
if (COMPONENT_gui)
	set(CMAKE_INSTALL_DEFAULT_COMPONENT_NAME "gui")

	add_subdirectory(OpenGL)
	add_subdirectory(ImageIO)
endif()

# this is for all the other stubbed frameworks
if (COMPONENT_gui_stubs)
	set(CMAKE_INSTALL_DEFAULT_COMPONENT_NAME "gui_stubs")

	add_subdirectory(AddressBook)
	add_subdirectory(AGL)
	add_subdirectory(AuthenticationServices)
	add_subdirectory(Automator)
	add_subdirectory(AVKit)
	add_subdirectory(CalendarStore)
	add_subdirectory(Collaboration)
	add_subdirectory(ContactsUI)
	add_subdirectory(CoreAudioKit)
	add_subdirectory(CoreBluetooth)
	add_subdirectory(CoreLocation)
	add_subdirectory(CoreMIDI)
	add_subdirectory(CoreMediaIO)
	add_subdirectory(CoreTelephony)
	add_subdirectory(CoreWLAN)
	add_subdirectory(DiscRecordingUI)
	add_subdirectory(DiscRecording)
	add_subdirectory(DrawSprocket)
	add_subdirectory(EventKit)
	add_subdirectory(ExceptionHandling)
	add_subdirectory(ForceFeedback)
	add_subdirectory(GameController)
	add_subdirectory(ImageCaptureCore)
	add_subdirectory(InputMethodKit)
	add_subdirectory(InstantMessage)
	add_subdirectory(IOBluetooth)
	add_subdirectory(IOBluetoothUI)
	add_subdirectory(MapKit)
	add_subdirectory(MediaAccessibility)
	add_subdirectory(MediaToolbox)
	add_subdirectory(ModelIO)
	add_subdirectory(MultipeerConnectivity)
	add_subdirectory(OpenAL)
	add_subdirectory(OpenCL)
	add_subdirectory(Photos)
	add_subdirectory(PDFKit)
	add_subdirectory(OSAKit)
	add_subdirectory(QTKit)
	add_subdirectory(ScreenSaver)
	add_subdirectory(ShazamKit)
	add_subdirectory(SpriteKit)
	add_subdirectory(StoreKit)
	add_subdirectory(SyncServices)
	add_subdirectory(VideoDecodeAcceleration)
	add_subdirectory(Vision)
endif()

# Xcode also needs WebKit, even on the CLI! it may be a weak dependency (i didn't check that),
# but what we do is we build our stub if the `webkit` component isn't enabled.
#
# TODO: actually build WebKit. since we *only* have the stub right now, we compile it when either
#       component is enabled.

#if (COMPONENT_dev_gui_common AND NOT COMPONENT_webkit)
if (COMPONENT_dev_gui_common OR COMPONENT_webkit)
	set(CMAKE_INSTALL_DEFAULT_COMPONENT_NAME "cli_dev_gui_common")

	# stub
	add_subdirectory(WebKit)
endif()

# we build these stubs if we're building `cli_dev` and *not* building Cocotron OR if we explicitly want them (`cli_dev_gui_stubs`).
# we explicitly want to build them when building the `all` component because this component is intended to be used for building
# Darling packages; we need to build a package that has these stubs available to be able to install Darling for CLI development
# without GUI dependencies like X11.
if ((COMPONENT_cli_dev AND NOT COMPONENT_gui) OR COMPONENT_cli_dev_gui_stubs)
	set(CMAKE_INSTALL_DEFAULT_COMPONENT_NAME "cli_dev")

	# if we're building the GUI components (i.e. Cocotron), don't install these stubs
	if (COMPONENT_gui)
		set(NO_INSTALL_ARG NO_INSTALL)
		set(STUB_SUFFIX "_stub")
	else()
		set(NO_INSTALL_ARG "")
		set(STUB_SUFFIX "")
	endif()

	add_subdirectory(dev-stubs/AppKit)
	add_subdirectory(dev-stubs/AudioToolbox)
	add_subdirectory(dev-stubs/Cocoa)
	add_subdirectory(dev-stubs/CoreData)
	add_subdirectory(dev-stubs/CoreGraphics)
	add_subdirectory(dev-stubs/CoreText)
	add_subdirectory(dev-stubs/ImageIO)
	add_subdirectory(dev-stubs/OpenGL)
	add_subdirectory(dev-stubs/QuartzCore)
endif()

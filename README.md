# Darling

Darling is a runtime environment for OS X applications.

Please note that no GUI applications are supported at the moment.

## Download

Darling uses many Git submodules, so a plain clone will not do.

````
git clone https://github.com/darlinghq/darling.git
cd darling
git submodule init
git submodule update
````

Updating sources:

````
git pull
git submodule init
git submodule update
````

## Build Instructions

#### For running x86-64 OS X binaries

<a href="http://teamcity.dolezel.info/viewType.html?buildTypeId=Darling_DebianStableX8664&guest=1">
<img src="http://teamcity.dolezel.info/app/rest/builds/buildType:(id:Darling_DebianStableX8664)/statusIcon" title="Debian stable build for x86-64"/>
</a>

````
cd darling
mkdir -p build/x86-64
cd build/x86-64
cmake ../.. -DCMAKE_TOOLCHAIN_FILE=../../Toolchain-x86_64.cmake
make
make install

# Now we go into src/lkm to build the kernel module
cd ../../src/lkm
make
make install
````

Required dependencies on Debian (stable): cmake clang bison flex linux-headers-amd64 xz-utils libfuse-dev libxml2-dev libicu-dev libssl-dev libbz2-dev zlib1g-dev libudev-dev

#### For running i386 OS X binaries

<a href="http://teamcity.dolezel.info/viewType.html?buildTypeId=Darling_DebianStableX8664&guest=1">
<img src="http://teamcity.dolezel.info/app/rest/builds/buildType:(id:Darling_DebianStableX8664)/statusIcon" title="Debian stable build for i386"/>
</a>

````
cd darling
mkdir -p build/i386
cd build/i386
cmake ../.. -DCMAKE_TOOLCHAIN_FILE=../../Toolchain-x86.cmake
make
make install
````

Required additional dependencies on Debian (stable): libc6-dev-i386 libudev-dev:i386 lib32stdc++-4.9-dev

Loading the kernel module:
````
modprobe darling-mach

# ATTENTION: The kernel module is likely unstable,
# full of vulnerabilities, etc.
# You SHOULD restrict access to /dev/mach to trusted
# users only and be prepared to the eventuality of
# kernel hangups (and related data loss).

chmod a+rw /dev/mach
````

### Optional Features

Optionally, you can enable audio support with the ````-DFRAMEWORK_COREAUDIO=On````. This is still under development, so it probably only makes sense if you want to contribute.
This switch enables both ALSA and PulseAudio support by default, you can disable either of them with ````-DENABLE_ALSA=OFF```` or ````-DENABLE_PULSEAUDIO=OFF```` respectively.

Required dependencies on Debian (stable):

* x86-64: libpulse-dev libasound2-dev libavresample-dev libavformat-dev libavcodec-dev
* i386: libpulse-dev:i386 libasound2-dev:i386 libavresample-dev:i386 libavformat-dev:i386 libavcodec-dev:i386

Note that most of the above -dev packages conflict between x86-64 and i386, so if you build for both platforms, you have to reinstall the right -dev variants before every build. There should be no issues at runtime.

## Using Darling

Darling uses DPREFIXes, which are in essence similar to WINEPREFIXes. The are virtual chroot environment with an OS X-like filesystem hierarchy, where you can safely install and run software. Unless you set DPREFIX to your location, the default ````~/.darling```` is used.

The real root filesystem is available through ````/system-root```` and the ````/home```` directory is automatically symlinked, so you should feel at home right away.

At first use, initial prefix contents are downloaded from the Internet.

### Hello world

Let's start with a Hello world:

````
$ darling shell echo Hello world
Hello world
````

Congratulations, you have printed Hello world through Darling's OS X system call emulation and runtime libraries.

### Installing software

You can install ````.pkg```` packages with the installer tool available inside shell. It is a somewhat limited cousin of OS X's installer:

````
$ darling shell
Darling [~]$ installer -pkg mc-4.8.7-0.pkg -target /
````

If you have previously downloaded the Midnight Commander package from [Rudix](http://rudix.org), you can now run ````mc```` to start MC for OS X. Note that not all Rudix packages may work under Darling, namely the Rudix Package Manager doesn't work, as Darling doesn't yet build its own Python runtime.

You can uninstall and list packages with the ````uninstaller```` command.

### Working with DMG images

DMG images can be attached and deattached from inside ````darling shell```` with ````hdiutil````. This is how you can install Xcode along with its toolchain and SDKs (note that Xcode itself doesn't run yet):

````
Darling [~]$ hdiutil attach Xcode_7.2.dmg
/Volumes/Xcode_7.2
Darling [~]$ cp -r /Volumes/Xcode_7.2/Xcode.app /Applications
Darling [~]$ export SDKROOT=/Applications/Xcode.app/Contents/Developer/Platforms/MacOSX.platform/Developer/SDKs/MacOSX10.11.sdk
Darling [~]$ echo 'void main() { puts("Hello world"); }' > helloworld.c
Darling [~]$ /Applications/Xcode.app/Contents/Developer/Toolchains/XcodeDefault.xctoolchain/usr/bin/clang helloworld.c -o helloworld
Darling [~]$ ./helloworld
Hello world
````

Congratulations, you have just compiled and run your own Hello world application with Apple's toolchain.


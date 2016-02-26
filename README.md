# Darling

Darling is a runtime environment for OS X applications.

Please note that no GUI applications are supported at the moment.

## Download

Darling uses many Git submodules, so a plain clone will not do.

````
git clone -r https://github.com/darlinghq/darling.git
````

Updating sources:

````
git pull
git submodule init
git submodule update
````

## Build Instructions

For complete instructions, visit [DarlingHQ.org](http://www.darlinghq.org/build-instructions).

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

### Hello world

Let's start with a Hello world:

````
$ darling shell echo Hello world
Hello world
````

Congratulations, you have printed Hello world through Darling's OS X system call emulation and runtime libraries.

### Installing software

You can install `.pkg` packages with the installer tool available inside shell. It is a somewhat limited cousin of OS X's installer:

````
$ darling shell
Darling [~]$ installer -pkg mc-4.8.7-0.pkg -target /
````

If you have previously downloaded the Midnight Commander package from [Rudix](http://rudix.org), you can now run `mc` to start MC for OS X. For easier installation, install the Rudix Package Manager. Note that not all Rudix packages may work under Darling.

You can uninstall and list packages with the `uninstaller` command.

### Working with DMG images

DMG images can be attached and deattached from inside `darling shell` with `hdiutil`. This is how you can install Xcode along with its toolchain and SDKs (note that Xcode itself doesn't run yet):

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


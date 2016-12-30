# Darling

![Darling logo](http://darlinghq.org/img/darling250.png)

Darling is a runtime environment for OS X applications.

Please note that no GUI applications are supported at the moment.

## Download

Darling uses many Git submodules, so a plain clone will not do.

````
git clone --recurse-submodules https://github.com/darlinghq/darling.git
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

Required dependencies

Debian (stable):

```
$ sudo apt-get install cmake clang bison flex xz-utils libfuse-dev libxml2-dev libicu-dev libssl-dev libbz2-dev zlib1g-dev libudev-dev linux-headers-amd64
```

Ubuntu (16.04):

```
$ sudo apt-get install cmake clang bison flex xz-utils libfuse-dev libxml2-dev libicu-dev libssl-dev libbz2-dev zlib1g-dev libudev-dev linux-headers-generic
```

Arch Linux (4.8):

```
$ sudo pacman -S cmake clang flex icu fuse
```

````
cd darling
mkdir -p build/x86-64
cd build/x86-64
cmake ../.. -DCMAKE_TOOLCHAIN_FILE=../../Toolchain-x86_64.cmake
make
make install
````

To build the new, experimental `darling` executable:

````
cmake ../.. -DCMAKE_TOOLCHAIN_FILE=../../Toolchain-x86_64.cmake -DNEW_DARLING=TRUE
make
make install
chmod +s $(which darling)
````

#### For running i386 OS X binaries

<a href="http://teamcity.dolezel.info/viewType.html?buildTypeId=Darling_DebianStableX8664&guest=1">
<img src="http://teamcity.dolezel.info/app/rest/builds/buildType:(id:Darling_DebianStableX8664)/statusIcon" title="Debian stable build for i386"/>
</a>

Required additional dependencies (on top of x86_64 dependencies)

Debian (stable) / Ubuntu (15.10):

```
$ sudo apt-get install libc6-dev-i386 libudev-dev:i386 lib32stdc++-4.9-dev
```

Arch Linux (4.8):

```
$ sudo pacman -S lib32-libstdc++5 lib32-clang
```

````
cd darling
mkdir -p build/i386
cd build/i386
cmake ../.. -DCMAKE_TOOLCHAIN_FILE=../../Toolchain-x86.cmake
make
make install
````

#### Building and loading the kernel module (x86-64 and i386)

````
# Go into src/lkm to build the kernel module
cd ../../src/lkm
make
make install
````

If this does not work (e.g. on newer Ubuntu versions), try the DKMS build:

````
# Go into src/lkm to build the kernel module
cd ../../src/lkm
sudo ./dkms.sh
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


### AppKit

AppKit is still highly experimental and incomplete, but to work on it you need to configure CMake with `-DFRAMEWORK_APPKIT=1` and install some additional packages.

Ubuntu 16.04:
```
sudo apt install qt5-default qtquick1-5-dev qtscript5-dev qtdeclarative5-dev

```

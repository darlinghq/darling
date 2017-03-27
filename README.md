[![Stories in Ready](https://badge.waffle.io/darlinghq/darling.png?label=ready&title=Ready)](https://waffle.io/darlinghq/darling)
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

For complete instructions, visit [DarlingHQ Wiki](https://wiki.darlinghq.org/build_instructions).

### Prefixes

Darling has support for DPREFIXes, which are very similar to WINEPREFIXes. They are virtual “chroot” environments with an macOS-like filesystem structure, where you can install software safely. The default DPREFIX location is `~/.darling`, but this can be changed by exporting an identically named environment variable. A prefix is automatically created and initialized on first use.

Please note that we use `overlayfs` for creating prefixes, and so we cannot support putting prefix on a filesystem like NFS or eCryptfs. In particular, the default prefix location won't work if you have an encrypted home directory.

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


# Darling

![Darling logo](https://darlinghq.org/img/darling250.png)

Darling is a runtime environment for macOS applications.

Please note that most GUI applications will not run at the moment.

## Download

Packages for some distributions are available for download
under [releases](https://github.com/darlinghq/darling/releases).

## Build Instructions

For build instructions, visit [Darling Docs](https://docs.darlinghq.org/build-instructions.html).

### Prefixes

Darling has support for DPREFIXes, which are very similar to WINEPREFIXes. They are virtual “chroot” environments with an macOS-like filesystem structure, where you can install software safely. The default DPREFIX location is `~/.darling`, but this can be changed by exporting an identically named environment variable. A prefix is automatically created and initialized on first use.

Please note that we use `overlayfs` for creating prefixes, and so we cannot support putting prefix on a filesystem like NFS or eCryptfs. In particular, the default prefix location won't work if you have an encrypted home directory.

### Hello world

Let's start with a Hello world:

````
$ darling shell echo Hello world
Hello world
````

Congratulations, you have printed Hello world through Darling's macOS system call emulation and runtime libraries.

### Installing software

You can install `.pkg` packages with the installer tool available inside shell. It is a somewhat limited cousin of macOS's installer:

````
$ darling shell
Darling [~]$ installer -pkg mc-4.8.7-0.pkg -target /
````

The Midnight Commander package from the above example is [available for download](https://darling-misc.s3.eu-central-1.amazonaws.com/mc-4.8.7-0.pkg).

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

### Working with XIP archives

Xcode is now distributed in `.xip` files. These can be installed using `unxip`:

```
cd /Applications
unxip Xcode_11.3.xip
```

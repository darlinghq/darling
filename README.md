<p align=center>
  <a href="https://darlinghq.org/">
    <img alt="Darling logo" src="https://darlinghq.org/img/darling250.png">
  </a>
</p>

---

<p align=center>
  <a href="https://github.com/darlinghq/darling/releases/latest">
    <img alt="Darling Latest Release" src="https://img.shields.io/badge/latest-release-0688CB.svg">
  </a>
  <a href="https://github.com/darlinghq/darling/blob/master/LICENSE">
    <img alt="License" src="https://img.shields.io/badge/license-GNU_GPL_3.0-0688CB.svg">
  </a>
  <a href="https://opencollective.com/darlinghq">
    <img alt="Donate" src="https://img.shields.io/badge/%24-donate-E44E4A.svg">
  </a>
</p>

## Quick links

[Website](https://darlinghq.org/) &bull;
[Community Discord](https://discord.gg/7knjvhT) &bull;
[Bug Tracker](https://github.com/darlinghq/darling/issues)

## Introduction

Darling is a runtime environment that allows running macOS applications on Linux 
without a virtual machine. It consists of a Mach-O binary loader and a userspace 
kernel server (darlingserver) that implements macOS's Mach IPC, POSIX, and Darwin 
syscall interfaces on top of Linux — along with reimplementations of Apple's 
frameworks, such as Foundation, AppKit (based on Cocotron), CoreAudio, and CoreFoundation.

Darling's low-level components (XNU, libSystem, Security) are based on [Apple's open-source](https://github.com/apple-oss-distributions) releases.
Higher-level frameworks are being actively reimplemented, with many CLI tools 
already functional. GUI application support is in active development, backed by 
an AppKit implementation and an initial Metal backend powered by Vulkan translation.

The code of Darling is licensed under [GNU GPL 3.0](https://github.com/darlinghq/darling/blob/master/LICENSE). Individual submodules may be licensed differently, as indicated within each of them.

## Install

Official packages for some distributions are available under [releases](https://github.com/darlinghq/darling/releases).

### Community Packages

These packages are built and maintained by community contributors, not the Darling team. 
Use them at your own discretion.

| Distribution | Link |
|---|---|
| Arch Linux (AUR) | [`darling-git`](https://aur.archlinux.org/packages/darling-git) |
| Gentoo | [`app-emulation/darling`](https://wiki.gentoo.org/wiki/Project:GURU/Information_for_End_Users) |
| NixOS | [`mio-19/nurpkgs`](https://github.com/mio-19/nurpkgs) |
| Ubuntu | [`jlucfarias/ppa`](https://launchpad.net/~jlucfarias/+archive/ubuntu/ppa) |

## Build Instructions

For build instructions, visit [Darling Docs](https://docs.darlinghq.org/build-instructions.html).

## Usage

### Prefixes

Darling has support for DPREFIXes, which are very similar to WINEPREFIXes. They are virtual “chroot” environments with a macOS-like filesystem structure, where you can install software safely. The default DPREFIX location is `~/.darling`, but this can be changed by exporting an identically named environment variable. A prefix is automatically created and initialized on first use.

Please note that we use `overlayfs` for creating prefixes, and so we cannot support putting prefix on a filesystem like NFS or eCryptfs. In particular, the default prefix location won't work if you have an encrypted home directory.

### Hello world

Let's start with a Hello world:

````
$ darling shell echo Hello world
Hello world
````

Congratulations, you have printed Hello world through Darling's OS X system call emulation and runtime libraries.

### Installing software

#### Working with `.pkg` files

You can install `.pkg` packages with the installer tool available inside shell. It is a somewhat limited cousin of OS X's installer:

```sh
$ darling shell
Darling [~]$ installer -pkg mc-4.8.7-0.pkg -target /
```

> Darling does not support to install [`.mpkg`](https://github.com/darlinghq/darling/issues/1662) files yet

The Midnight Commander package from the above example is [available for download](https://darling-misc.s3.eu-central-1.amazonaws.com/mc-4.8.7-0.pkg).

You can uninstall and list packages with the `uninstaller` command.

#### Working with DMG images

DMG images can be attached and detached from inside `darling shell` with `hdiutil`. This is how you can install Xcode along with its toolchain and SDKs (note that Xcode itself doesn't run yet):

```sh
Darling [~]$ hdiutil attach Xcode_7.2.dmg
/Volumes/Xcode_7.2
Darling [~]$ cp -r /Volumes/Xcode_7.2/Xcode.app /Applications
Darling [~]$ export SDKROOT=/Applications/Xcode.app/Contents/Developer/Platforms/MacOSX.platform/Developer/SDKs/MacOSX10.11.sdk
Darling [~]$ echo 'void main() { puts("Hello world"); }' > helloworld.c
Darling [~]$ /Applications/Xcode.app/Contents/Developer/Toolchains/XcodeDefault.xctoolchain/usr/bin/clang helloworld.c -o helloworld
Darling [~]$ ./helloworld
Hello world
```

Congratulations, you have just compiled and run your own Hello world application with Apple's toolchain.

#### Working with XIP archives

Xcode is now distributed in `.xip` files. These can be installed using `unxip`:

```sh
cd /Applications
unxip Xcode_11.3.xip
```

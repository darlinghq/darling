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
    <img alt="License" src="https://img.shields.io/badge/license-GNU_GPL_3.0-E44E4A.svg">
  </a>
  <a href="https://opencollective.com/darlinghq">
    <img alt="Donate" src="https://img.shields.io/badge/%24-donate-FF813F.svg">
  </a>
</p>

## Quick links

[Website](https://darlinghq.org/) &bull;
[Community Discord](https://discord.gg/XRD3mQA) &bull;
[Bug Tracker](https://github.com/darlinghq/darling/issues) &bull;
[AppDB](https://appdb.darlinghq.org) &bull;
[Blog](https://blog.darlinghq.org)

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

> [!CAUTION]
> **These packages are neither maintained nor vetted by the Darling team.**\
> Use them at your own risk.

For a list of community packages, see the [Community Packages](https://docs.darlinghq.org/community/packages.html) section in the Darling documentation.

## Build Instructions

For build instructions, visit [Darling Docs](https://docs.darlinghq.org/build-instructions.html).

## Usage

### Prefixes

Darling has support for DPREFIXes, which are very similar to WINEPREFIXes. They are virtual “chroot” environments with a macOS-like filesystem structure, where you can install software safely. The default DPREFIX location is `~/.darling`, but this can be changed by exporting an identically named environment variable set to an absolute path. A prefix is automatically created and initialized on first use.

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

> Darling does not support installing [`.mpkg`](https://github.com/darlinghq/darling/issues/1662) files yet

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

#### Opening files from the Linux desktop

`darling open` opens files of the host in Darling, as `open` does inside it. A disk image is mounted by DiskImageMounter (after you accept its licence agreement, if it has one) and its window is shown in Finder:

```sh
$ darling open ~/Downloads/SomeApp.dmg
```

Darling installs a desktop entry for disk images, "Disk Image Mounter (Darling)", so that file managers offer it under "Open With". To make it what a double-click on a `.dmg` does:

```sh
$ xdg-mime default org.darlinghq.Darling.DiskImageMounter.desktop application/x-apple-diskimage
```

#### Running macOS binaries directly (optional)

Darling ships a host-facing [binfmt_misc](https://www.kernel.org/doc/html/latest/admin-guide/binfmt-misc.html)
configuration in `share/darling/binfmt/darling.conf`. It is not enabled by default: the
`lib/binfmt.d/darling.conf` is for binaries already inside a container and must not be used on the
host. With systemd-binfmt, opt in with:

```sh
$ sudo install -Dm644 /usr/local/share/darling/binfmt/darling.conf /etc/binfmt.d/darling-host.conf
$ sudo systemctl restart systemd-binfmt
```

Replace `/usr/local` with your installation prefix. Do not register this file together with the
in-container `lib/binfmt.d/darling.conf`: both match the same magics, and only one interpreter can win.
If your distribution installed that file under `/usr/lib/binfmt.d`, remove it first. The host handler
also serves guest `execve()` calls, so containers keep working. A 64-bit, 32-bit or fat Mach-O
executable can now be run directly; if no container is running, the handler starts one. Remove the
file from `/etc/binfmt.d` and restart systemd-binfmt to undo this. On a host without systemd, the same
lines can be written to `/proc/sys/fs/binfmt_misc/register` as root.

#### Desktop entries for installed apps

`darling-desktop` generates a desktop entry for each `.app` bundle found in a prefix:

```sh
$ darling-desktop
```

Entries go to `$XDG_DATA_HOME/applications`, or `~/.local/share/applications` when that variable is
unset. The MIME types are read from the guest's own UTI declarations (CoreTypes.bundle) and each
app's `CFBundleDocumentTypes`, so an app that declares `public.text` is offered for `text/plain`
files without a hard-coded list. Re-run the command after installing or removing apps. Use
`--prefix`, `--app-dir`, `--core-types` or `--output-dir` to override the defaults.

Darling also installs generic desktop entries and shared MIME information for Mach-O executables and
`.app` bundles, so file managers can offer Darling for them until an app-specific entry is preferred.
The generated app entries call `darling open -a`, which also works by hand:

```sh
$ darling open -a TextEdit ~/Documents/notes.txt
```

#### Working with XIP archives

Xcode is now distributed in `.xip` files. These can be installed using `unxip`:

```sh
cd /Applications
unxip Xcode_11.3.xip
```

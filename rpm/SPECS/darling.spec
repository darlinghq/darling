#disable binary striping, in case this is a problem
%global __os_install_post %{nil}
#Disable debug packages, since these are emulated files mostly
%define debug_package %{nil}
%define commit_date %{getenv:DARLING_COMMIT_DATE}
%if "%{commit_date}" == ""
	%define commit_date 0
%endif
%global _buildshell /bin/bash
# rpm on Debian and Ubuntu has no CMake macros (Fedora's come with cmake-rpm-macros)
%{!?__cmake:%global __cmake /usr/bin/cmake}

# As in debian/rules, darling-core-native (what runs on Linux) is built for every architecture with
# DARLING_BUILD_MACHO=OFF, and the Mach-O part is noarch, with the i386, x86_64 and arm64 slices. The x86-64
# build makes it (only that host has the 32-bit libraries for the i386 slices) and the aarch64 build skips it;
# the noarch packages go into the aarch64 repository too, which is what lets an aarch64 host run x86_64
# programs (FEX). Both builds need the same commit (DARLING_COMMIT_DATE) and %%{?dist}: darling-core requires
# darling-core-native at its exact version and release. `--with macho` or `--without macho` overrides the
# default, `--define 'darling_archs ...'` the slices (`--with macho` off x86_64 needs one without i386).
%ifarch x86_64
%bcond_without macho
%else
%bcond_with macho
%endif
%if %{with macho}
%{!?darling_archs:%global darling_archs i386;x86_64;arm64}
%endif

# explicitly ignore all the bogus dependencies that the auto-scanner finds in `/usr/libexec/darling`
#
# note that we *don't* want to simply use `__requires_exclude_from` to exclude `/usr/libexec/darling` from scanning,
# since we *do* want our Mach-O scanner to scan that tree (and there's no way to only exclude paths for some dependency generators; it's all or nothing).
%global __requires_exclude ^(/bin/sed|/bin/sh|/usr/bin/perl|/usr/bin/python2.7|/System/Library/Frameworks/Python.framework/Versions/2.7/bin/python2.7|/usr/bin/python|/System/Library/Frameworks/Ruby.framework/Versions/2.6/usr/bin/ruby|/usr/bin/env|/usr/bin/ruby)$

Name:           darling
Version:        0.1.%{commit_date}
Release:        1%{?dist}
Summary:        Darling

Group:          Utility
License:        GPLv3
URL:            https://www.darlinghq.org/
# Use this line for Source0 if there are ever official versions.
# Source0:        https://github.com/darlinghq/darling/archive/%%{version}/%%{name}-%%{version}.tar.gz
Source0:        %{name}.tar.gz
BuildRoot:      %{_tmppath}/%{name}-%{version}-%{release}-root-%(%{__id_u} -n)

BuildRequires:  cmake clang bison flex python2 glibc-devel%{_isa}
BuildRequires:  fuse-devel systemd-devel
BuildRequires:  cairo-devel freetype-devel%{_isa} fontconfig-devel%{_isa} make
BuildRequires:  libjpeg-turbo-devel%{_isa} libtiff-devel%{_isa}
%ifarch x86_64
%if %{with macho}
# the i386 slices, which only an x86-64 host builds, link against the 32-bit host libraries
BuildRequires:  glibc-devel(x86-32) freetype-devel(x86-32) fontconfig-devel(x86-32)
BuildRequires:  libjpeg-turbo-devel(x86-32)
%endif
%endif
BuildRequires:  libglvnd-devel mesa-libGL-devel mesa-libEGL-devel
BuildRequires:  libxml2-devel elfutils-libelf-devel
BuildRequires:  libbsd-devel
BuildRequires:  ffmpeg-devel pulseaudio-libs-devel openssl-devel giflib-devel
BuildRequires:  libXrandr-devel libXi-devel libXcursor-devel libxkbfile-devel dbus-devel mesa-libGLU-devel
BuildRequires:  vulkan-headers llvm-devel libcap-devel bash vulkan-loader-devel
BuildRequires:  ruby perl gperf unifdef python3

Requires: darling-cli darling-python2 darling-ruby darling-perl darling-gui darling-gui-stubs darling-pyobjc

%description
Translation layer for running macOS software on Linux

%package extra
Summary:       Extra components for Darling
Group:         Utility
Requires:      darling darling-jsc darling-cli-extra

%description extra
Extra components for Darling that are not part of a typical ("stock") installation

%package core-native
Summary:    Darling components that run on Linux
Group:      Utility

%description core-native
The parts of Darling built for the host architecture: the darling launcher
(installed setuid root), darpath, darling-coredump, darlingserver, the Mach-O
loader (mldr) and the binfmt_misc configuration.

%if %{with macho}
%package core
Summary:    Darling core components
Group:      Utility
BuildArch:  noarch
Requires:   darling-core-native = %{version}-%{release} xdg-user-dirs

%description core
Core components of Darling

%package system
Summary:    Darling system components
Group:      Utility
BuildArch:  noarch
Requires:   darling-core

%description system
System components of Darling

%package cli
Summary:    Darling CLI components
Group:      Utility
BuildArch:  noarch
Requires:   darling-system darling-cli-gui-common darling-cli-python2-common

%description cli
CLI components of Darling

%package ffi
Summary:    libffi for Darling
Group:      Utility
BuildArch:  noarch
Requires:   darling-core

%description ffi
libffi built for use within Darling

%package cli-gui-common
Summary:    Darling CLI and GUI common components
Group:      Utility
BuildArch:  noarch
Requires:   darling-system

%description cli-gui-common
Components of Darling that are shared between the CLI and GUI components

%package iokitd
Summary:    Darling IOKit daemon
Group:      Utility
BuildArch:  noarch
Requires:   darling-system darling-iosurface

%description iokitd
IOKit daemon for Darling

%package cli-devenv-gui-common
Summary:    Darling developer environment and GUI common components
Group:      Utility
BuildArch:  noarch
Requires:   darling-system

%description cli-devenv-gui-common
Components of Darling that are shared between the developer environment and the GUI components

%package cli-extra
Summary:    Extra Darling CLI components
Group:      Utility
BuildArch:  noarch
Requires:   darling-cli

%description cli-extra
Non-standard CLI components of Darling

%package gui
Summary:    Darling GUI components
Group:      Utility
BuildArch:  noarch
Requires:   darling-system darling-cli-devenv-gui-common darling-iokitd darling-cli-gui-common darling-iosurface

%description gui
GUI components of Darling

%package python2
Summary:    Python 2 for Darling
Group:      Utility
BuildArch:  noarch
Requires:   darling-core darling-cli-python2-common darling-ffi

%description python2
Python 2 (and associated programs) built for use within Darling

%package cli-python2-common
Summary:    Darling CLI and Python 2 common components
Group:      Utility
BuildArch:  noarch
Requires:   darling-core

%description cli-python2-common
Components of Darling that are shared between the CLI and Python 2 components

%package pyobjc
Summary:    PyObjC for Darling
Group:      Utility
BuildArch:  noarch
Requires:   darling-gui-stubs darling-python2

%description pyobjc
PyObjC built for use within Darling

%package ruby
Summary:    Ruby for Darling
Group:      Utility
BuildArch:  noarch
Requires:   darling-core darling-ffi

%description ruby
Ruby built for use within Darling

%package perl
Summary:    Perl for Darling
Group:      Utility
BuildArch:  noarch
Requires:   darling-core

%description perl
Perl built for use within Darling

%package jsc
Summary:    JavaScriptCore for Darling
Group:      Utility
BuildArch:  noarch
Requires:   darling-system

%description jsc
JavaScriptCore built for use within Darling

%package iosurface
Summary:    IOSurface framework for Darling
Group:      Utility
BuildArch:  noarch
Requires:   darling-system

%description iosurface
IOSurface framework built for use within Darling

%package cli-devenv-gui-stubs-common
Summary:    Darling developer environment and GUI stubs common components
Group:      Utility
BuildArch:  noarch
Requires:   darling-cli-devenv-gui-common

%description cli-devenv-gui-stubs-common
Components of Darling that are shared between the developer environment and GUI stub components

%package gui-stubs
Summary:    Darling GUI stub components
Group:      Utility
BuildArch:  noarch
Requires:   darling-gui darling-cli-devenv-gui-stubs-common

%description gui-stubs
GUI stub components of Darling
%endif

%prep
%setup -q -n %{name}

%build
%{__mkdir_p} build
pushd build
	# Release is broken https://github.com/darlinghq/darling/issues/331
	#          -DCMAKE_BUILD_TYPE=Release \
	CFLAGS="" CXXFLAGS="" CPPFLAGS="" LDFLAGS="" \
	%{__cmake} -DCMAKE_INSTALL_PREFIX=%{_prefix} \
	           -DOpenGL_GL_PREFERENCE=GLVND \
	           -DDEBIAN_PACKAGING=ON \
%if %{with macho}
	           -DDARLING_SWIFT_WMO=ON \
	           "-DDARLING_ARCHS=%{darling_archs}" \
%else
	           -DDARLING_BUILD_MACHO=OFF \
%endif
	           ..
	%{make_build} -j `nproc`
popd

%install
[ "$RPM_BUILD_ROOT" != "/" ] && rm -rf $RPM_BUILD_ROOT
# core_native is what runs on Linux (launcher, mldr, darlingserver, binfmt.d); the rest is the Mach-O part
DARLING_COMPONENTS=(
	core_native
%if %{with macho}
	core
	system
	cli
	ffi
	cli_gui_common
	iokitd
	cli_dev_gui_common
	cli_extra
	gui
	python
	cli_python_common
	pyobjc
	ruby
	perl
	jsc
	iosurface
	cli_dev_gui_stubs_common
	gui_stubs
%endif
)
PACKAGE_SUFFIXES=(
	core-native
%if %{with macho}
	core
	system
	cli
	ffi
	cli-gui-common
	iokitd
	cli-devenv-gui-common
	cli-extra
	gui
	python2
	cli-python2-common
	pyobjc
	ruby
	perl
	jsc
	iosurface
	cli-devenv-gui-stubs-common
	gui-stubs
%endif
)
pushd build
	for index in "${!PACKAGE_SUFFIXES[@]}"; do
		rm -rf tmp/${PACKAGE_SUFFIXES[index]}
		DESTDIR=tmp/${PACKAGE_SUFFIXES[index]} %{__cmake} -DCOMPONENT=${DARLING_COMPONENTS[index]} -P cmake_install.cmake
		find tmp/${PACKAGE_SUFFIXES[index]} \( ! -type d -o -type d -empty \) -printf '"/%%P"\n' > files.${PACKAGE_SUFFIXES[index]}.txt
		cp -rla tmp/${PACKAGE_SUFFIXES[index]}/. %{?buildroot}/
	done

%if %{with macho}
	# pack up a "source" (actually binary) tarball for `darling-cli-devenv`
	# NOTE: this is probably not the best approach. fix this if something better comes up.
	rm -rf tmp/cli-devenv
	DESTDIR=tmp/cli-devenv %{__cmake} -DCOMPONENT=cli_dev -P cmake_install.cmake
	rm -f %{_sourcedir}/darling-cli-devenv.tar.gz
	tar --transform "s|^\./|darling-cli-devenv/|" -cf %{_sourcedir}/darling-cli-devenv.tar.gz -C tmp/cli-devenv .
%endif
popd

%files
%doc LICENSE

%files extra

%files core-native -f build/files.core-native.txt

%if %{with macho}
%files core -f build/files.core.txt
%files system -f build/files.system.txt
%files cli -f build/files.cli.txt
%files ffi -f build/files.ffi.txt
%files cli-gui-common -f build/files.cli-gui-common.txt
%files iokitd -f build/files.iokitd.txt
%files cli-devenv-gui-common -f build/files.cli-devenv-gui-common.txt
%files cli-extra -f build/files.cli-extra.txt
%files gui -f build/files.gui.txt
%files python2 -f build/files.python2.txt
%files cli-python2-common -f build/files.cli-python2-common.txt
%files pyobjc -f build/files.pyobjc.txt
%files ruby -f build/files.ruby.txt
%files perl -f build/files.perl.txt
%files jsc -f build/files.jsc.txt
%files iosurface -f build/files.iosurface.txt
%files cli-devenv-gui-stubs-common -f build/files.cli-devenv-gui-stubs-common.txt
%files gui-stubs -f build/files.gui-stubs.txt
%endif

%changelog
* Tue May 02 2023 Ariel Abreu <facekapow@outlook.com> - 0.1.20230502-1
- Update to latest version and Fedora 37
- Split package into multiple packages (one for each component of Darling)

* Tue Aug 18 2020 Andy Neff <andy@visionsystemsinc.com> - 0.1.20200331-1
- Update for latest version and Fedora 31

* Mon Aug 12 2019 Andy Neff <andy@visionsystemsinc.com> - 0.1-4
- Update for Fedora 30

* Tue Mar 12 2019 Andy Neff <andy@visionsystemsinc.com> - 0.1-3
- Remove bad commented macro, cleanup, and verify master works on Fedora 29

* Wed Jul 18 2018 Andy Neff <andy@visionsystemsinc.com> - 0.1-2
- Update for Fedora 28 and new master

* Tue Jan 23 2018 Andy Neff <andy@visionsystemsinc.com> - 0.1-1
- Initial version working for Fedora 27

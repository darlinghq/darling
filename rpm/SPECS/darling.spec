#disable binary striping, in case this is a problem
%global __os_install_post %{nil}
#Disable debug packages, since these are emulated files mostly
%define debug_package %{nil}

Name:           darling
Version:        0.1.20200331
Release:        1%{?dist}
Summary:        Darling

Group:          Utility
License:        GPLv3
URL:            https://www.darlinghq.org/
# Use this line for Source0 if there are ever official versions.
# Source0:        https://github.com/darlinghq/darling/archive/%%{version}/%%{name}-%%{version}.tar.gz
Source0:        %{name}.tar.gz
Source1:        dkms.conf
BuildRoot:      %{_tmppath}/%{name}-%{version}-%{release}-root-%(%{__id_u} -n)

BuildRequires:  cmake clang bison flex python2 glibc-devel(x86-64) glibc-devel(x86-32)
BuildRequires:  fuse-devel systemd-devel kernel-devel dkms
BuildRequires:  cairo-devel freetype-devel(x86-64) fontconfig-devel(x86-64)
BuildRequires:  freetype-devel(x86-32) fontconfig-devel(x86-32) make
BuildRequires:  libjpeg-turbo-devel(x86-64) libtiff-devel(x86-64)
BuildRequires:  libjpeg-turbo-devel(x86-32) libtiff-devel(x86-32)
BuildRequires:  libglvnd-devel mesa-libGL-devel mesa-libEGL-devel
BuildRequires:  libxml2-devel elfutils-libelf-devel
BuildRequires:  libbsd-devel
BuildRequires:  ffmpeg-devel pulseaudio-libs-devel openssl-devel giflib-devel
BuildRequires:  libXrandr-devel libXcursor-devel libxkbfile-devel dbus-devel mesa-libGLU-devel
# Normally rpm will pick up all the mac pieces as dependencies. Disable that.
AutoReqProv:    no

%description
Darling macOS emulator

%package mach
Summary:        Darling mach dkms module
Group:          Utility
Requires:       dkms make gcc kernel-devel
AutoReqProv:    no

%description mach
Linux kernel module for darling-mach

%prep
%setup -q -n %{name}

%build
%{__mkdir_p} build
pushd build
  # Release is broken https://github.com/darlinghq/darling/issues/331
  #          -DCMAKE_BUILD_TYPE=Release \
  %{__cmake} -DCMAKE_INSTALL_PREFIX=/usr \
             -DOpenGL_GL_PREFERENCE=GLVND \
             ..
  %{make_build} -j `nproc`
popd

%install
[ "$RPM_BUILD_ROOT" != "/" ] && rm -rf $RPM_BUILD_ROOT
pushd build
  %{make_install}
  %{make_build} lkm_generate
popd
%{__install} -d -m 755 %{?buildroot}/usr/src/%{name}-mach-%{version}/miggen
cp -dr --no-preserve=ownership src/external/lkm %{?buildroot}/usr/src/%{name}-mach-%{version}/lkm
cp -dr --no-preserve=ownership build/src/external/lkm/osfmk %{?buildroot}/usr/src/%{name}-mach-%{version}/miggen/osfmk
# Copy rtsig.h
cp -dr --no-preserve=ownership build/src/startup/rtsig.h %{?buildroot}/usr/src/%{name}-mach-%{version}/lkm/darling/

%{__install} -m 644 %{SOURCE1} %{?buildroot}/usr/src/%{name}-mach-%{version}

# https://github.com/dell/dkms/issues/25
%preun mach
/usr/sbin/dkms remove -m %{name}-mach -v %{version} --all --rpm_safe_upgrade || :

%pre mach
occurrences=$(/usr/sbin/dkms status "%{name}/%{version}" | wc -l)
if [ ! ${occurrences} -gt 0 ]; then
  /usr/sbin/dkms remove -m %{name}-mach -v %{version} --all --rpm_safe_upgrade || :
fi

%post mach
/usr/sbin/dkms add -m %{name}-mach -v %{version} --rpm_safe_upgrade || :
/usr/sbin/dkms build -m %{name}-mach -v %{version} || :
/usr/sbin/dkms install -m %{name}-mach -v %{version} || :

%files
%doc LICENSE
%{_bindir}/darling
%{_libexecdir}/darling

%files mach
%{_prefix}/src/%{name}-mach-%{version}

%changelog
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

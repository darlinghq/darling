#disable binary striping, in case this is a problem
%global __os_install_post %{nil}
#Disable debug packages, since these are emulated files mostly
%define debug_package %{nil}

Name:           darling
Version:        0.1
Release:        1%{?dist}
Summary:        Darling

Group:          Utility
License:        GPLv3
URL:            https://www.darlinghq.org/
# Source0:        https://github.com/darlinghq/darling/archive/%%{version}/%%{name}-%%{version}.tar.gz
Source0:        %{name}.tar.gz
Source1:        dkms.conf
BuildRoot:      %{_tmppath}/%{name}-%{version}-%{release}-root-%(%{__id_u} -n)

BuildRequires:  cmake clang bison flex python2 glibc-devel(x86-64) glibc-devel(x86-32)
BuildRequires:  fuse-devel systemd-devel kernel-devel dkms
BuildRequires:  cairo-devel freetype-devel(x86-64) fontconfig-devel(x86-64)
BuildRequires:  freetype-devel(x86-32) fontconfig-devel(x86-32)
BuildRequires:  libjpeg-turbo-devel(x86-64) libtiff-devel(x86-64)
BuildRequires:  libjpeg-turbo-devel(x86-32) libtiff-devel(x86-32)
BuildRequires:  libglvnd-devel mesa-libGL-devel

#It will pick up all the mac pieces as dependencies. No thank you!
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
             ..
  %{make_build}
popd

%install
[ "$RPM_BUILD_ROOT" != "/" ] && rm -rf $RPM_BUILD_ROOT
pushd build
  %{make_install}
  %{make_build} lkm_generate
popd
%{__install} -d -m 755 %{?buildroot}/usr/src/%{name}-mach-%{version}/miggen
cp -dr --no-preserve=ownership src/lkm %{?buildroot}/usr/src/%{name}-mach-%{version}/lkm
cp -dr --no-preserve=ownership build/src/lkm/osfmk %{?buildroot}/usr/src/%{name}-mach-%{version}/miggen/osfmk
%{__install} -m 644 %{SOURCE1} %{?buildroot}/usr/src/%{name}-mach-%{version}

%post
setcap cap_sys_rawio=ep %{_libexecdir}/darling/bin/mldr
setcap cap_sys_rawio=ep %{_libexecdir}/darling/bin/mldr32

#setsebool -P mmap_low_allowed 1

%preun mach
/usr/sbin/dkms remove -m %{name}-mach -v %{version} --all

%post mach
occurrences=$(/usr/sbin/dkms status | grep "%{name}" | grep "%{version}" | wc -l)
if [ ! ${occurrences} -gt 0 ];
then
  /usr/sbin/dkms add -m %{name}-mach -v %{version}
fi
/usr/sbin/dkms build -m %{name}-mach -v %{version}
/usr/sbin/dkms install -m %{name}-mach -v %{version}

%files
%doc LICENSE
%{_bindir}/darling
%{_libexecdir}/darling

%files mach
%{_sysconfdir}/udev/rules.d/00-darling-mach.rules
%{_prefix}/src/%{name}-mach-%{version}

%changelog
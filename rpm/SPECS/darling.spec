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

BuildRequires:  cmake clang bison flex python2 glibc-devel(x86-32)
BuildRequires:  fuse-devel systemd-devel kernel-devel dkms
BuildRequires:  cairo-devel freetype-devel(x86-64) fontconfig-devel(x86-64)
BuildRequires:  freetype-devel(x86-32) fontconfig-devel(x86-32)
BuildRequires:  libjpeg-turbo-devel(x86-64) libtiff-devel(x86-64)
BuildRequires:  libjpeg-turbo-devel(x86-32) libtiff-devel(x86-32)
BuildRequires:  libglvnd-devel mesa-libGL-devel

%description
Darling macOS emulator

%package dkms
Summary:        Darling mach dkms module
Group:          Utility
Requires:       dkms make gcc kernel-devel

%description dkms
Linux kernel module for darling

%prep
%setup -q -n %{name}

%build
mkdir -p build
pushd build
  # Release is broken! https://github.com/darlinghq/darling/issues/331
  %{__cmake} -DCMAKE_INSTALL_PREFIX=/usr \
             -DCMAKE_BUILD_TYPE=Debug \
             ..
  %{make_build}
popd

%install
[ "$RPM_BUILD_ROOT" != "/" ] && rm -rf $RPM_BUILD_ROOT
pushd build
  %{make_install}
popd
cp -dr --no-preserve=ownership src/lkm /usr/src/darling-mach-0.1
cp -dr --no-preserve=ownership src/lkm/osfmk /usr/src/darling-mach-0.1/miggen
%{__install} -m 644 %{SOURCE1} %{?buildroot}/usr/src/darling-mach-0.1

%preun dkms
/usr/sbin/dkms remove -m %{name} -v %{version} --all

%post dkms
occurrences=/usr/sbin/dkms status | grep "%{name}" | grep "%{version}" | wc -l
if [ ! occurrences > 0 ];
then
    /usr/sbin/dkms add -m %{name} -v %{version}
fi
/usr/sbin/dkms build -m %{name} -v %{version}
/usr/sbin/dkms install -m %{name} -v %{version}

%files
%doc LICENSE
%{_bindir}
%{_libdir}

%files dkms
/etc/udev/rules.d/00-darling-mach.rules
/usr/src/darling-mach-0.1

%changelog
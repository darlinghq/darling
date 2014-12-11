Name:       libkqueue
Summary:    Userspace implementation of the kqueue event notification mechanism
Version:    2.0.1
Release:    1
# The entire source code is MIT, event.h which is BSD (2-clause)
License:    MIT and BSD
Group:      System Environment/Libraries
URL:        http://sourceforge.net/p/libkqueue/wiki/Home/
Source0:    http://downloads.sourceforge.net/%{name}/%{name}-%{version}.tar.gz
Provides:   libkqueue.so.0

%description
A user space implementation of the kqueue(2) kernel event notification
mechanism. libkqueue acts as a translator between the kevent structure and the
native kernel facilities.

%package devel
Group: Development/Libraries
Summary: Development files for %{name}-%{version}
Requires: %{name}%{?_isa} = %{version}-%{release}

%description devel
%{summary}

%prep
%setup -q

# %%{configure} sets --target 'x86_64-redhat-linux-gnu' instead of 'linux'
%build
CFLAGS="%{optflags} -fpic" %{_configure} --prefix=%{_prefix} --libdir=%{_libdir}
make

%install
%{make_install}

%check
make check

%post -p /sbin/ldconfig

%postun -p /sbin/ldconfig

%files
%doc LICENSE
%{_libdir}/libkqueue.so.*

%files devel
%dir %{_includedir}/kqueue
%dir %{_includedir}/kqueue/sys
%{_libdir}/libkqueue.so
%{_includedir}/kqueue/sys/event.h
%{_libdir}/pkgconfig/libkqueue.pc
%{_mandir}/man2/kqueue.2.*
%{_mandir}/man2/kevent.2.*

%changelog
* Wed May 08 2013 Mark Heily <mark@heily.com> - 2.0.1-1
- New upstream release

* Wed May 01 2013 Eric Radman <ericshane@eradman.com> - 2.0-1
- Adapt to libkqueue 2.0

* Thu Apr 04 2013 Eric Radman <ericshane@eradman.com> - 1.0.1-6
- Set CFLAGS using optflags since we're using a custom configure
- Shorten summary

* Wed Apr 03 2013 Eric Radman <ericshane@eradman.com> - 1.0.1-5
- Use %% to avoid warning about special character in comment
- Remove defattr

* Fri Jan 18 2013 Eric Radman <ericshane@eradman.com> - 1.0.1-4
- Configure respects CFLAGS and LDFLAGS. Also pass --libdir to the configure script

* Sat Jan 12 2013 Eric Radman <ericshane@eradman.com> - 1.0.1-3
- Add license text for event.h
- Use the dir macro for the include directories
- Remove static package, exclude .a file, and make sure new directories are owned by the RPM

* Thu Jan 03 2013 Eric Radman <ericshane@eradman.com> - 1.0.1-3
- Configure respects CFLAGS and LDFLAGS. Also pass --libdir= to the configure script instead of setting LIB

* Mon Dec 31 2012 Eric Radman <ericshane@eradman.com> - 1.0.1-2
- Do not set the buildroot (F-10 does not require it)
- Do not manually clean (F-13)

* Thu Dec 27 2012 Eric Radman <ericshane@eradman.com> - 1.0.1-1
- Initial build with separate -devel package. Based on work done by Mark Heily,
  Aditya Patawari and John Haxby


%global __os_install_post %{nil}
%define debug_package %{nil}
%define commit_date %{getenv:DARLING_COMMIT_DATE}
%if "%{commit_date}" == ""
	%define commit_date 0
%endif
%global _buildshell /bin/bash

Name:           darling-macho-deps
Version:        0.1.%{commit_date}
Release:        1%{?dist}
Summary:        Dependency generator for Mach-O files (used by Darling)

Group:          Utility
License:        GPLv3
URL:            https://www.darlinghq.org/
# Use this line for Source0 if there are ever official versions.
# Source0:        https://github.com/darlinghq/darling/archive/%%{version}/%%{name}-%%{version}.tar.gz
Source0:        %{name}.tar.gz

BuildRequires:  bash

%description
An RPM dependency generator for Mach-O files, used by Darling to generate ELF dependencies for the library it builds

%prep
%setup -q -n %{name}

%build
true

%install
[ "$RPM_BUILD_ROOT" != "/" ] && rm -rf $RPM_BUILD_ROOT
%{__mkdir_p} %{?buildroot}/usr/lib/rpm/fileattrs
cp darling.attr %{?buildroot}/usr/lib/rpm/fileattrs/
cp darling-deps.req %{?buildroot}/usr/lib/rpm/

%files
/usr/lib/rpm/fileattrs/darling.attr
/usr/lib/rpm/darling-deps.req

%changelog
* Wed Oct 25 2023 Benjamin Gaillard <git@benjamin.gaillard.name> - 0.1.20231025-1
- Use default build root

* Tue May 02 2023 Ariel Abreu <facekapow@outlook.com> - 0.1.20230502-1
- Initial version working for Fedora 37

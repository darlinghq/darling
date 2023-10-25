# NOTE: this package MUST be built after building the main `darling` package

#disable binary striping, in case this is a problem
%global __os_install_post %{nil}
#Disable debug packages, since these are emulated files mostly
%define debug_package %{nil}
%define commit_date %{getenv:DARLING_COMMIT_DATE}
%if "%{commit_date}" == ""
	%define commit_date 0
%endif
%global _buildshell /bin/bash

Name:           darling-cli-devenv
Version:        0.1.%{commit_date}
Release:        1%{?dist}
Summary:        Darling developer environment

Group:          Utility
License:        GPLv3
URL:            https://www.darlinghq.org/
# Use this line for Source0 if there are ever official versions.
# Source0:        https://github.com/darlinghq/darling/archive/%%{version}/%%{name}-%%{version}.tar.gz
Source0:        %{name}.tar.gz

BuildRequires:  bash

Requires:       darling-cli darling-python2 darling-ruby darling-perl darling-cli-devenv-gui-common darling-cli-devenv-gui-stubs-common darling-iosurface
Conflicts:      darling-gui darling-gui-stubs

%description
A Darling environment for CLI-only programs for building and developing using the command line developer tools

%prep
%setup -q -n %{name}

%build
true

%install
[ "$RPM_BUILD_ROOT" != "/" ] && rm -rf $RPM_BUILD_ROOT
cp -rla . %{?buildroot}/

%files
%{_libexecdir}/darling

%changelog
* Wed Oct 25 2023 Benjamin Gaillard <git@benjamin.gaillard.name> - 0.1.20231025-1
- Use default build root

* Tue May 02 2023 Ariel Abreu <facekapow@outlook.com> - 0.1.20230502-1
- Initial version working for Fedora 37
- Split package off from `darling` package

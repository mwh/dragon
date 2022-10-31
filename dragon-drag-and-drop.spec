# File: dragon-drag-and-drop.spec
# Location: https://gitlab.com/bgstack15/dragon.git
# Author: bgstack15
# Startdate: 2021-01-20
# Title: Rpm spec for dragon-drag-and-drop package
# Purpose: Provide build instructions for rpm for package
# History:
# Usage:
# Reference:
#    bgstack15 package
#    https://gitlab.com/jeremieglbd/xdragon/-/blob/master/xdragon.spec
# Improve:
# Documentation:
# Dependencies:

%define sname dragon
%define branch packages
%define devtty "/dev/null"
%define debug_package %{nil}

Summary:	   simple drag-and-drop source/sink for X or Wayland
Name:		   dragon-drag-and-drop
Version:	   1.1.2
Release:	   1
License:	   GPL-3.0
Source0:    https://gitlab.com/bgstack15/%{sname}/-/archive/%{branch}/%{sname}-%{branch}.tar.gz
URL:        https://github.com/mwh/dragon
Packager:   B. Stack <bgstack15@gmail.com>
BuildRequires: gtk3-devel
BuildRequires: gcc

%description
Many programs, particularly web applications, expect files to be dragged
into them now. If you don't habitually use a file manager that is a
problem. dragon is a lightweight drag-and-drop source for X that solves
this problem.

%prep
%setup -q -c %{name}
ls1="$( ls -1 2>/dev/null )"
test -d "${ls1}" && { find "${ls1}" -mindepth 1 -maxdepth 1 -exec mv {} . \; ; }
rmdir "${ls1}"
exit 0

%build
%make_build 

%install
%{__install} -m0755 -d %{buildroot}%{_bindir} %{buildroot}%{_pkgdocdir} \
   %{buildroot}%{_mandir}/man1
%{__install} -m0755 dragon %{buildroot}%{_bindir}/dragon-drag-and-drop
%{__install} -m0644 example/download.sh %{buildroot}%{_pkgdocdir}
%{__install} -m0644 debian/dragon-drag-and-drop.1 %{buildroot}%{_mandir}/man1

%clean
rm -rf %{buildroot}

%files
%{_bindir}/*
%{_mandir}/man1/*
%doc %{_pkgdocdir}/download.sh

%changelog
* Wed Jan 20 2021 B. Stack <bgstack15@gmail.com> - 1.1.2-1
- Initial rpm spec

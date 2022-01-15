%define prefix	/usr
%define name	cssed-python-plugin
%define version	0.2.2
%define release 0%{?dist}

Summary:         Python bindings for cssed's plugable interface
Name:            %{name}
Version:         %{version}
Release:         %{release}
Group:           Development/Tools
License:         GPL
Source:          http://prdownloads.sourceforge.net/cssed/%{name}-%{version}.tar.gz
Url:             http://cssed.sourceforge.net
BuildRoot:       %{_tmppath}/cssed-%{version}-%{release}-root-%(%{__id_u} -n)
BuildArch:       i386
Requires:        gtk2 >= 2.0.6, glib2 >= 2.0, cssed >= 0.4.0, python >= 2.2
Buildrequires:   cssed-devel >= 0.4.0, gtk2-devel >= 2.0.6, glib2-devel >= 2.0, python-devel >= 2.2

%description 
cssed is a tiny GTK+ CSS editor and validator
for web developers. This plugin provides python
bindings for cssed's plugable interface.

%prep
%setup -q -n %{name}-%{version}

%build
[ ! -f Makefile ] || make distclean

%configure
make %{?_smp_mflags}

%install
rm -rf %{buildroot}
DESTDIR=%{buildroot} make install

%find_lang %{name}

%clean
rm -rf %{buildroot}

%files 
%defattr(-,root,root)
%{_libdir}/cssed/plugins/pythonscript.so

%doc AUTHORS COPYING ChangeLog README INSTALL NEWS

%changelog
* Tue Jul 04 2006 Iago Rubio <iago.rubio@hispalinux.es> 0.2.2-0
- Changes to use the find_lang macro. 
* Wed Nov 23 2005 Iago Iago Rubio <iago.rubio@hispalinux.es> 0.2-0
- Fixed bad auto-package not getting the last code. Version bump to get major in sync with other plugins.
* Sun Sep 18 2005 Iago Rubio <iago.rubio@hispalinux.es> 0.1-1
- Updated requires for cssed devel package. Fixed python requires.
* Sat Sep 17 2005 Iago Rubio <iago.rubio@hispalinux.es> 0.1-0
- Initiall RPM.



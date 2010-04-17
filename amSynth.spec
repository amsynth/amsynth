# needsrootforbuild

Name:		amSynth
Version:	svn
Source:		%{name}-%{version}.tar.gz
Summary:	A simple virtual analog synthesizer
Release:	1
Url:		http://amsynthe.sourceforge.net
License:	GPL
Group:		Sound

BuildRequires:	alsa-devel gcc-c++ gtkmm24-devel libsndfile-devel

%description
A virtual-analog polyphonic synthesizer.


Authors:
--------
    Nick Dowell <nixx2097@users.sourceforge.net>

%prep
%setup -q

%build
autoreconf --force --install
%configure
make

%install  
make DESTDIR="$RPM_BUILD_ROOT" PREFIX=%{_prefix} install

%clean  
[ "$RPM_BUILD_ROOT" != "/" -a -d "$RPM_BUILD_ROOT" ] && rm -rf $RPM_BUILD_ROOT

%files
%defattr(-,root,root)
%{_bindir}/*
%{_datadir}/amSynth

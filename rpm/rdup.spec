Summary: rdup - utility to create a file list suitable for making backups
Name: rdup
Version: 2.0.14
Release: 1

Group: Applications/Archiving
Vendor: Miek Gieben <hdup-user@miek.nl>
URL: http://miek.nl/projects/hdup2/hdup.html
License: GPL

Source: http://www.miek.nl/projects/hdup2/previous/%{name}-%{version}.tar.gz
BuildRoot: %{_tmppath}/%{name}-root
Requires: tar >= 1.14.00
# Recommends: gzip, bzip2, openssh, mcrypt  # not required

%description
hdup is used to back up a filesystem.
Features include encryption of the archive (with mcrypt),
compression of the archive (bzip/gzip/none), the ability to
transfer the archive to a remote host or restoring from a
remote host (with ssh), the ability to split up archives, and
no obscure archive format (it is a normal compressed tar file).

%prep
%setup -c
mv hdup2.0.14/* . && rmdir hdup2.0.14 # Move into standard directory

%build
%configure
make

%install
rm -rf $RPM_BUILD_ROOT
install -d $RPM_BUILD_ROOT/usr/sbin 
%makeinstall bindir=$RPM_BUILD_ROOT/usr/sbin

%clean
rm -rf $RPM_BUILD_ROOT

%files
%defattr(-,root,root)
/usr/sbin/hdup
%dir /etc/hdup
%config(noreplace) /etc/hdup/hdup.conf
%attr(0644,root,root) %{_mandir}/man1/hdup*
%attr(0644,root,root) %{_mandir}/man5/hdup.conf*
%doc doc/FAQ.html contrib/*.pl 
%doc Credits ChangeLog* README

%changelog
* Fri Sep 30 2005 Miek Gieben <miek@miek.nl>
- bump the version depency of tar to a more current one

* Thu Nov 11 2004 Miek Gieben <miekg@miek.nl>
- patch from Boris to update to hdup2.0.0

* Thu Jun 19 2003 Miek Gieben <miekg@miek.nl>
- spec fixes from Anders Bjorklund
- 1.6.17, release 2

* Wed Jun 18 2003 Miek Gieben <miekg@miek.nl>
- 1.6.17
- install in /usr/sbin

* Fri Jun 13 2003 Anders Bjorklund <andersb@blacksun.ca>
- 1.6.16
- inital RPM release

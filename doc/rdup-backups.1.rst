==============
 RDUP-BACKUPS
==============

----------------------------------------------------------
rdup-backups - introduction into making backups with rdup
----------------------------------------------------------

.. include:: header.rst

INTRODUCTION
============
rdup is a simple program that prints out a list of files and
directories that are changed on a filesystem. 

It is much more sophisticated than for instance *find*, 
because rdup will
find files that are removed and directories that are renamed.  By
glueing rdup together with a few very simple shell and perl
scripts you can create a simple to understand, but powerfull backup
solution. 

rdup includes a few scripts to make backups and restores easy to
perform. rdup supports remote, encrypted and compressed backups.

It is always best to backup to **another** medium, be it a different
local harddisk or a NFS mounted filesystem or use the remote backup
capabilities to securely copy the backup to another system all together.

There is one wrapper script for rdup to make backups. This is
*rdup-simple*.  It uses a hardlinked backup scheme where each day
has its own directory.  This wrapper calls; Brdup, rdup-snap
and rdup-snap-link. It further more supports encryption,
compressions and remote backups. This works by inserting the
apropiate rdup helper utility in the pipeline.

BACKING UP WITH RDUP-SIMPLE
===========================

When using rdup-simple the backup process consists out of two
phases. During phase one a copy is made of any previous backups backups.
This a hardlinked copy, meaning that it will take up very little space.
It uses GNU cp to make this copy. See the manual page of
rdup-snap-link for more information. rdup-simple uses
~/.rdup as the directory to write its internal filelist and timestamp
file.

In phase two, rdup-simple will only update the files that
are changed since the last backup. For these files the hardlink is
removed or overwritten with a new version of the file. The net result
is that each backup represents a complete view of your filesystem.

With rdup-simple you have a full view on what your filesystem
looked like at any specific date. I personaly keep about three months of
backups and I can go back to any specific date in that time frame.

EXAMPLES
---------
LOCAL BACKUPS
-------------
Backing up my homedir to the backup directory::

	rdup-simple ~ /vol/backup/$HOSTNAME


|

This will create a backup in /vol/backup/$HOSTNAME/200705/15. So
each day will have its own directory. Multiple sources are allowed, so::

	rdup-simple ~ /etc/ /var/lib /vol/backup/$HOSTNAME

|	..this is a hack to force a newline

Will backup your homedirectory, /etc and /var/lib to the backup
location. Also if you need to compress your backup, simple add
a '-z' switch::

	rdup-simple -z ~ /etc/ /var/lib /vol/backup/$HOSTNAME

|

REMOTE BACKUPS
--------------
For a remote backup to work, both the sending machine and the receiving
machine must have rdup installed. The currently implemented
protocol is *ssh*

Dumping my homedir to the remote server::

	rdup-simple ~ ssh://miekg@remote/vol/backup/$HOSTNAME

|

The syntax is almost identical only the destination starts with
the magic string 'ssh://'. Compression and encryption are just
as easily enabled as with a local backup, just add '-z' and/or
a '-k keyfile' argument::

	rdup-simple -z -k 'secret-file' ~ ssh://miekg@remote/vol/backup/$HOSTNAME

RESTORE
-------
In principle a restore is as easy as using the standard system tools to
copy a directory to another location. However when the
\-a flag is used extended attributes are set, these are normally not
read by the unix utilities. In this case you should restore by using
rdup-cp to copy the files to another location. But there is also
a script that can be used: rdup-restore.

LOCAL RESTORE
-------------
Restoring my homedir to a temporaty directory::

	rdup-restore /vol/backup/$HOSTNAME/200705/14/home/miekg /tmp/restore-miek

|

And ofcourse the compression and encryption also works here, so
to restore a compressed backup you need only to add the 'z' flag::

	rdup-restore -z /vol/backup/$HOSTNAME/200705/14/home/miekg /tmp/restore-miek

|

REMOTE RESTORE
--------------
When doing a remote restore the files are *pulled* from the
remote server and then copied to your local server.

::

 rdup-restore ssh://miek@remote/vol/backup/$HOSTNAME /tmp/restore

|

Compression and encryption will work as expected.

.SH SEE ALSO
rdup(1), rdup-snap(1), rdup-gzip(1), rdup-gpg(1) and rdup-crypt(1).

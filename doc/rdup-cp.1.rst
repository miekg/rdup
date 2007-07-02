=========
 RDUP-CP
=========

-------------------------------
rdup-cp - copy files from rdup
-------------------------------

.. include:: header.rst

SYNOPSIS
========
	rdup-cp [OPTIONS] DEST

DESCRIPTION
===========
rdup-cp copies the files it gets from standard input to the directory
DEST. This script can be used to restore files from a backup,
especially when the -a option of rdup is used.
.PP
The command line would look something like this:

.RS
rdup /dev/null FROM | rdup-cp DEST
.RE

Or when you need rdup to fill in the extended attributes:

.RS
rdup -a /dev/null FROM | rdup-cp DEST
.RE

.B DEST
can only be a local directory.

OPTIONS
=======

.TP
.B -a
Write the uid and gid information to the file's extended user attributes:
r_uid and r_gid. This option only works if 
.B attr
is available
on the system. This currently works for Linux and Solaris 10.

.TP
.B -p NUM	
Strip NUM slashes of the path names, thereby stripping a common prefix
of all names.

.TP
.B -c 
Used in conjunction with rdup -c. Process the files' contents also.

.TP
.B -v
Echo the files processed to standard error.

.TP
.B -h
Short help message.

.TP
.B -V
Show the version.

SEE ALSO
========
rdup(1) and cp(1).

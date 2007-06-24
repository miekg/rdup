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

\fIDEST\fR can only be a local directory.

OPTIONS
=======

-a

Write the uid and gid information to the file's extended user attributes:
r_uid and r_gid. This option only works if **attr** is available
on the system. This currently works for Linux and Solaris 10.

-p NUM	

Strip NUM slashes of the path names, thereby stripping a common prefix
of all names.

-c 

Used in conjunction with rdup -c. Process the files' contents also.

-v

Echo the files processed to standard error.

-h

Short help message.

-V

Show the version.

SEE ALSO
========
rdup(1) and cp(1).

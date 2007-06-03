================
 RDUP-SNAP-LINK
================

--------------------------------------------------------------
rdup-snap-link, create a hard linked copy of a previous backup
--------------------------------------------------------------

.. include:: header.rst

SYNOPSIS
========
	rdup-snap-link DAYS BACKUPDIR NOW

DESCRIPTION
===========
rdup-snap-link creates a hard linked copy of a previous
backup directory. If such a directory is created /rdup-snap-link/
exists with 0. If a full dump should be performed a 1 is returned.
If 2 is returned an error has occured. GNU cp is used for the actual 
creation of the copy. On non GNU operating systems you may need to
install GNU cp.

This utility is called from *rdup-simple*.

FILES
=====
@sysconfdir@/rdup/rdup-snapshot.gnu for the location of GNU cp and GNU
date. This is probably only needed on non GNU/Linux systems.
This file has the following format::

 GNU_DATE=/path/to/gnu_date
 GNU_CP=/path/to/gnu_cp

SEE ALSO
========
rdup-backups(1) and rdup-simple(1).

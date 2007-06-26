===========
 RDUP-PURGE
===========

-----------------------------------
rdup-purge - delete old directories 
-----------------------------------

.. include:: header.rst

SYNOPSIS
========
	rdup-purge [OPTIONS] DIRECTORY

DESCRIPTION
===========
rdup-purge deletes old backup directories. It does this in a logarithmic
way (idea taken from simple-backup). It will delete in the following way:

* the last 30 days - keep everything
* last 100 days, keep a backup every 7 days, delete the rest
* last 1000 days, keep a backup every 100 days, delete the rest
* more than a 1000 days - delete everything.


OPTIONS
=======

-n

Do a dry run, don't delete anything from the filesystem.

-h

Short help message.

-V

Show the version.

SEE ALSO
========
rdup(1) and rm(1).

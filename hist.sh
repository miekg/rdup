#!/bin/bash

# also copied form plan9
#Hist prints the names, dates, and sizes of all versions of the named files,
#looking backwards in time, stored in the dump file system. If the file exists
#in the main tree, the first line of output will be its current state. For
#example,
#
#        hist ~rsc/.bash_history
#produces
#
#May 19 16:11:37 EDT 2005 /home/am3/rsc/.bash_history 6175
#May 18 23:32:16 EDT 2005 /dump/am/2005/0519/home/am3/rsc/.bash_history 5156
#May 17 23:32:31 EDT 2005 /dump/am/2005/0518/home/am3/rsc/.bash_history 5075
#May 16 07:53:47 EDT 2005 /dump/am/2005/0517/home/am3/rsc/.bash_history 5065

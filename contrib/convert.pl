#!/usr/bin/perl -wn
# convert a pre 1.1.x internal rdup filelist to rdup 1.1.x format
# Author: Miek Gieben (miek@miek.nl)
# License: GPLv3
split;
if ($_[0] eq "41471") { $l[3] = "l" } # symlink
if ($_[3] eq "*")     { $l[3] = "-" }   
print "@_\n";

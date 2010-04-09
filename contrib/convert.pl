#!/usr/bin/perl -wn
split;
if ($_[0] eq "41471") { $l[3] = "l" } # symlink
if ($_[3] eq "*")     { $l[3] = "-" }   
print "@_\n";

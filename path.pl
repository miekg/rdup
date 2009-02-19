#!/usr/bin/perl 
# +- 0775 1000 1000 21 1432 /home/miekg/bin/gitvi
# extend this path by inserting a path prefix - say /vol/backup
# before it - make sure the pathlen is adjusted too

$prefix = "/vol/backup";
$prelen = length($prefix);

while(<>) {
    @p = split / /, $_, 7;
    $p[4] += $prelen;
    $p[6] = $prefix . $p[6];
    print "@p";
}

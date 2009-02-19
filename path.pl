#!/usr/bin/perl 
# +- 0775 1000 1000 21 1432 /home/miekg/bin/gitvi

$prefix = "/vol/backup";
$prelen = length($prefix);

while(<>) {
    @p = split / /, $_, 7;
    # add
    $p[4] += $prelen;
    $p[6] = $prefix . $p[6];
    print "@p";
}

#!/usr/bin/perl

use Fcntl ':mode';
use feature 'switch';

use warnings;
use strict;

# convert rdup's internal list to rdup output
# if you save this list on your backup. You
# can backup as a normal user and save the
# original user/group info.
#
# If you feed this to rdup-tr you get your
# backup back in the original state
# (otherwise all files would user/user, where
# user is the user running the ssh)

# Internal list looks like: (may change)
# # mode dev inode linktype pathlen filesize path
# 16877 2309 2 * 5 4096 /home
#
# rdup output looks like
# #type perm user/group pathlen filesize path
# +d 0755 0 0 5 0 /home

use constant { 
    MODE    => 0,
    DEV     => 1,
    INODE   => 2,
    LINK    => 3,
    PLEN    => 4,
    FSIZE   => 5,
    PATH    => 6
};

my $prefix = "/vol/backup";
my $prelen = length($prefix);

my @p;
<>;
while (<>) {
    chomp;
    @p = split / /, $_, 7;

    if (S_ISDIR($p[MODE])) { $p[FSIZE] = 0; }

    if (defined $prefix) {
	$p[PLEN] += $prelen;
	$p[PATH] =  $prefix . $p[PATH];
    }

    # plusmin
    print "+";

    # type, trying out the new Perl 5.10 stuff
    given ($p[MODE]) {
	when (S_ISDIR $_) {
	    print "d";
	}

	when (S_ISREG $_) {
	    if ($p[LINK] eq 'h') { print "h"; }
	    if ($p[LINK] eq '*') { print "-"; }
	}

	when (S_ISLNK $_) {
	    print "l";
	}

	# ...
    }

    # perms
    printf " %04o", $p[MODE] & 07777;

    # user/group (don't have that info (yet))
    print " 1000 1000";

    # pathlen
    printf " %s %s %s\n", $p[PLEN], $p[FSIZE], $p[PATH];

}

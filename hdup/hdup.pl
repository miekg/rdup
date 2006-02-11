#!/usr/bin/perl -w

# be a worthy replacement of the C version of hdup2
# read the config file and use rdup to create the
# tar archives.

use strict;
use Config::IniFiles;
use Unix::Syslog;

my $VERSION="rdup(hdup2) @RDUP_VERSION@";

my $hdupconf = "hdup.conf";
my $cfg;
my %rec;

$rec{global} = {
        ARCHIVEDIR => '',       # backup directory
        DATE => '',             # not impl. default to ISO
        DIR => [],              # list of directories to backup
        USER => '',             # owner of archive
        GROUP => '',            # group of archive
        PRERUN => '',           # pre run cmd
        POSTRUN => '',          # post run cmd
        FREE => '',             # how much free space 
        COMPRESSION => '',      # compression program to use
        ALGORITHM => '',        # encryption algorithm to use
        KEY => '',              # encryption key file to use
        ONEFILESYSTEM => '',    # yes/no one filesystem
        NOBACKUP => ''          # if there use .nobackup
        };

# parse options



$cfg = Config::IniFiles->new(-file => "$hdupconf");

if (! $cfg->SectionExists('global') ) {
        die "No [global] section in the configuration file: $hdupconf";
}

# read in the keywords
foreach my $sec ($cfg->Sections) {
        print "$sec\n";
        $rec{$sec}->{ARCHIVEDIR} = $cfg->val($sec, 'archive dir');
        $rec{$sec}->{COMPRESSION} = $cfg->val($sec, 'compression');
        # list structures
        @{$rec{$sec}->{DIR}} = split /, */, $cfg->val($sec, 'dir');
}
# inherit from [global] to children??

print $rec{global}->{ARCHIVEDIR},"\n";
print @{$rec{elektron}->{DIR}},"\n";


# run rdup, feed it's output to tar
# put the archive some where, chown/chgrp it
# encrypt it
# ready

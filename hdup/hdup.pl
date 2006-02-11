#!/usr/bin/perl -w

# be a worthy replacement of the C version of hdup2
# read the config file and use rdup to create the
# tar archives.

# we only allow
# bzip2/gzip/none
# date cannot be specified anymore
# compression level gone

use strict;
use Config::IniFiles;
use Unix::Syslog;
use Getopt::Std;

my $VERSION="\@PACKAGE_NAME\@(hdup2) \@PACKAGE_VERSION\@";

my $TRUE = 1;
my $FALSE = 0;
my $cfg;
my %rec;
my $SCHEME;
my $HOST;
my %opt;

# if yes/true/on -> TRUE
# else no/false/off -> FALSE
sub yesno {
        if (!defined($_)) {
                return $FALSE;
        }
        
        if (/yes/i || /true/i || /on/i) {
                return $TRUE;
        } 
        return $FALSE;
}


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

# parse options/arguments
getopts('c:', \%opt);

my $CONFIG=$opt{c} || "/etc/hdup/hdup.conf";

#if !defined then exit

$SCHEME=$ARGV[0];   # null/monthly daily/one, weekly->monthly
$HOST=$ARGV[1];     # the go'old host

# get the scheme
if ($SCHEME =~ /weekly/) {
        warn "Weekly is not implemented, doing a null dump\n";
        $SCHEME = "null";
}
if ($SCHEME =~ /monthly/) {
        $SCHEME = "null";
}
if ($SCHEME =~ /daily/) {
        $SCHEME = "one";
}
if ($SCHEME ne "null" && $SCHEME ne "one") {
        die "Scheme $SCHEME is not known\n";
}


my (undef, undef, undef, $mday, $mon, $year, undef, undef, undef) =
                localtime(time());
my $DATE = ($year + 1900) . "-" . 
                sprintf("%02d", ($mon + 1)). "-" . 
                sprintf("%02d", $mday);
my $BACKUPDIR = ($year + 1900) . sprintf("%02d", ($mon + 1));

$cfg = Config::IniFiles->new(-file => "$CONFIG") || die;
if (! $cfg->SectionExists($HOST) ) {
        die "No [$HOST] section in the configuration file: $CONFIG\n";
}
if (! $cfg->SectionExists('global') ) {
        die "No [global] section in the configuration file: $CONFIG\n";
}

# read in the keywords
foreach my $sec ($cfg->Sections) {
        $rec{$sec}->{ARCHIVEDIR} = $cfg->val($sec, 'archive dir');
        $rec{$sec}->{COMPRESSION} = $cfg->val($sec, 'compression');
        $rec{$sec}->{ONEFILESYSTEM} = yesno($cfg->val($sec, 'one filesystem'));
        $rec{$sec}->{TAROPTION} = $cfg->val($sec, 'tar option');
        $rec{$sec}->{TAR} = $cfg->val($sec, 'tar');

        # list structures
        @{$rec{$sec}->{DIR}} = split /, */, $cfg->val($sec, 'dir') if
                                (defined($cfg->val($sec, 'dir')));
}

# SCHEME is NULL
my $TARFILE = "$HOST-$DATE.tar.gz";
my $rdupcmd = "/home/miekg/svn/rdup/trunk/rdup -q $HOST-list " . join(' ', @{$rec{$HOST}->{DIR}});
my $tarcmd  = "tar --create --gzip --no-recursion --file $TARFILE --files-from -";

my $rdup = open(RDUP, "$rdupcmd |")  or die "Couldn't fork rdup: $!\n";
my $tar  = open(TAR, "| $tarcmd")  or die "Couldn't fork tar: $!\n";
while (<RDUP>) {
        my ($mode, $uid, $gid, $psize, $fsize, $path) = split / /, $_;

        if (/^\+/) {
                # only add '+' files
               print TAR $path;
        }
}
close(TAR)                          or die "Couldn't close tar: $!\n";
close(RDUP)                         or die "Couldn't close rdup: $!\n";

print $TARFILE,"\n";

# encrypt
# chown/chgrp
# split?

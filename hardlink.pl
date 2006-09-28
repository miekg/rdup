#!/usr/bin/perl -w
#
# Copyright (c) 2005, 2006 Miek Gieben; Mark J Hewitt
# See LICENSE for the license
# Hardlink script

use strict;

use Getopt::Long qw(:config no_ignore_case bundling);
use File::Basename;
use File::Path;
use File::Copy;
use File::Copy::Recursive qw(dircopy rcopy fcopy);
use Fcntl qw(:mode);

my $ts = time;
my ($sec, $min, $hour, $mday, $mon, $year, $wday, $yday, $isdst) = localtime($ts);
my $S_MMASK = 07777;      # mask to get permission
my $S_ISDIR = 040000;
my $S_ISLNK = 0120000;

my $progName = basename $0;
my ($help, $version, $verbose, $attr, $restore, $remote, @backupDir);

GetOptions("h" => \$help,
        "V" => \$version,
        "a" => \$attr, # extended attr support
        "c" => \$remote,
        "v" => \$verbose,
        "b=s" => \@backupDir);

usage() if $help;
version() if $version;
my $hostname = `hostname`;    # This hostname
chomp $hostname;
# Location of backup root in local filesystem
if ($#backupDir == -1) { die "** Need at least one -b option"; }
my $attr_there = check_attr();    # Can we set extended attributes?

# Statistics
my $ftsize = 0;        # Total file size
my $ireg = 0;        # Number of regular files
my $idir = 0;        # Number of directories
my $ilnk = 0;        # Number of symbolic links
my $irm  = 0;        # Number of files removed
my ($mode, $uid, $gid, $psize, $fsize, $path);

if ($remote) {
        while (($_ = <STDIN>)) {
                chomp;
                ($mode, $uid, $gid, $psize, $fsize) = split(" ", $_, 5);
                _mirror(@backupDir);
        }
} else {
        while (($_ = <STDIN>)) {
                chomp;
                ($mode, $uid, $gid, $psize, $fsize, $path) = split(" ", $_, 6);
                _mirror(@backupDir);
        }
}
my $te = time;

printf STDERR "** #REG FILES  : %d\n", $ireg;
printf STDERR "** #DIRECTORIES: %d\n", $idir;
printf STDERR "** #LINKS      : %d\n", $ilnk;
printf STDERR "** #(RE)MOVED  : %d\n", $irm;
printf STDERR "** SIZE        : %d MB\n", $ftsize/1024/1024;
printf STDERR "** STORED IN   : %s\n",  join(", ",@backupDir);
printf STDERR "** ELAPSED     : %d s\n", $te - $ts;

exit 0;

sub _mirror {
        my @backupDir = @_;
        my $dump = substr($mode, 0, 1);
        my $modebits = substr($mode, 1);

        sanity_check($dump, $modebits, $psize, $fsize, $uid, $gid);

        if ($remote) {
                $path = "";
                read STDIN, $path, $psize;
        }

        die "** Empty path"  if ($path eq "");

        my $bits = $modebits & $S_MMASK;
        my $typ = 0;
        $typ = 1 if ($modebits & $S_ISDIR) == $S_ISDIR;
        $typ = 2 if ($modebits & $S_ISLNK) == $S_ISLNK;
        print STDERR "$path\n" if $verbose;

        if ($dump eq '+') {        # add
                if ($typ == 0) {        # REG

                        foreach my $dir (@backupDir) {
                                my $target = "$dir/$path";

                                if (exist($target) == 1) {      
                                        unlink $target or warn "** Cannot unlink $target: $!";
                                }
                                if ($remote) {
                                        open FILE, ">$target" or warn "** Cannot create $target: $!";
                                        if ($fsize != 0) {
                                                copyout($fsize, *FILE);
                                        }
                                        chown $uid, $gid, *FILE;
                                        chown_attr($uid, $gid, $target);
                                        chmod $bits, *FILE;
                                        close FILE;
                                } else {
                                        copy($path, $target) or warn "** Copy failed: $! $path $target";
                                        chown $uid, $gid, $target;
                                        chown_attr($uid, $gid, $target);
                                        chmod $modebits, $target;
                                }
                        }
                        $ftsize += $fsize;
                        $ireg++;
                } elsif ($typ == 1) {        # DIR
                        foreach my $dir (@backupDir) {
                                my $target = "$dir/$path";
                                exist($target);  # side effects Go!
                                if (-f _ || -l _) {      # Type of entiry has changed
                                        unlink $target or warn "** Cannot unlink $target: $!";
                                }
                                mkpath($target) unless -d _;
                                print $target . "\n" unless -d _;
                                chown $uid, $gid, $target;
                                chown_attr($uid, $gid, $target);
                                if ($> != 0 && $bits == 0) {
                                        print STDERR "chmod $target to 700\n";
                                        $bits = 0700;
                                }
                                chmod $bits, $target;
                        }
                        $idir++;
                } elsif ($typ == 2) {        # LNK; target id the content
                        foreach my $dir (@backupDir) {
                                my $target = "$dir/$path";
                                if (exist($target) == 1) {      # We only have a suffix if the file exists
                                        unlink $target or warn "** Cannot unlink $target: $!";
                                }
                                if ($remote) {
                                        my $linkTarget = "";
                                        read STDIN, $linkTarget, $fsize;
                                        symlink $target, $linkTarget;
                                } else {
                                        symlink(readlink($path), $target) or warn "** Cannot create link: $target -> $path: $!";
                                }

                                chown $uid, $gid, $target;
                                chown_attr($uid, $gid, $target);
                        }
                        $ilnk++;
                }
        } else {        # Remove
                if (!$restore) {
                        foreach my $dir (@backupDir) {
                                my $target = "$dir/$path";
                                if (exist($target)) {      
                                        unlink $target or warn "** Cannot rename $target: $!";
                                }
                        }
                        $irm++;
                }
        }
}

sub chown_attr {
        return unless $attr_there;
        return unless $attr;

        my $xuid = $_[0];
        my $xgid = $_[1];
        my $file = $_[2];

        if ($^O eq "linux") {
                system("attr -q -s r_uid -V$xuid \"$file\"");
                system("attr -q -s r_gid -V$xgid \"$file\"");
        }
}

sub exist {
        my $filename = $_[0];

        lstat($filename);
        return 0 unless -e _;
        return 1;
}

sub sanity_check {
        my $dump = $_[0];
        my $mode = $_[1];
        my $psize = $_[2];
        my $fsize = $_[3];
        my $uid = $_[4];
        my $gid = $_[5];

        die "** $progName: dump ($dump) must be + or -"   if $dump ne "+" && $dump ne "-";
        die "** $progName: mode ($mode) must be numeric"  unless $mode =~ "[0-9]+";
        die "** $progName: psize ($psize) must be numeric" unless $psize =~ "[0-9]+";
        die "** $progName: fsize ($fsize) must be numeric" unless $fsize =~ "[0-9]+";
        die "** $progName: uid ($uid) must be numeric"   unless $uid =~ "[0-9]+";
        die "** $progName: gid ($gid) must be numeric"   unless $gid =~ "[0-9]+";
}

sub check_attr {
        if ($^O eq "linux") {
                map {return 1 if -x $_ . '/' . "attr"}  split(/:/, $ENV{'PATH'});
        } else {
                warn "Cannot set extended attributes";
        }
        return 0;
}

sub usage {
        print "$progName -b DIR [OPTIONS]\n\n";
        print "Update hardlinks according to the filelist of rdup\n\n";
        print "OPTIONS\n";
        print " -b DIR  use DIR as the backup directory\n";
        print " -a      write extended attributes r_uid/r_gid with uid/gid\n";
        print " -c      process the file content also (rdup -c), for remote backups\n";
        print " -v      print the files processed to standard error\n";
        print " -h      this help\n";
        print " -V      print version\n";
        exit 0;
}

sub version {
        print "** $progName: 0.2.18 (rdup-utils)\n";
        exit 0;
}

sub copyout {
        my $count = $_[0];
        my $pipe = $_[1];

        my $buf;
        my $n;

        while ($count > 4096) {
                $n = read STDIN, $buf, 4096;
                syswrite $pipe, $buf, $n;
                $count -= 4096;
        }
        if ($count > 0) {
                $n = read STDIN, $buf, $count;
                syswrite $pipe, $buf, $n;
        }
}

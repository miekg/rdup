#!/usr/bin/perl

# gets filelist and list of directories
# prefix directory with "D " 'D space'
# print to stdout: files to be backupped
# print to stderr: files to be deleted (gone on disk, but still in the
# list)
# if list is not there a full dump is done
# the incremental stuff is related to creation date of the list
# .nobackup files are honered (cmd line arg)

# walk dirs and get the filenames
# while in_list(file) and newer -> backup, else keep in list

use Fcntl ':mode'; # for IS_DIR

# incr -n .nobackup [-0] FILELIST DIR [DIR ...]
$FALSE= 0;
$TRUE = 1;

# -0 switch
if ($TRUE) {
        $delimeter = "\n";
} else {
        $delimeter = "\0";
}

# -n switch
# TODO
$nobackup = ".nobackup";

sub read_filelist {
        if (-e $ARGV[1]) {
                open FILELIST, "r", $ARGV[1] or die "Cannot open $ARGV[1]: $!";
                @filelist = <FILELIST>;
                close FILELIST;
        } else {
                @filelist = (1);
        }
}

sub dir_crawl {
}

# return all elements which are in @a and not in @b
sub substract {
        my ($a_ref, $b_ref) = @_;

        @a = @$a_ref; @b = @$b_ref;

        # Get the difference between them
        foreach $e (@a, @b) { $count{$e}++ }
        foreach $e (keys %count) {
                if ($count{$e} != 2) {
                        push @diff, $e;
                }
        }

        # Get the intersection of @a and @diff
        undef(%count);
        foreach $e (@a, @diff) { $count{$e}++ }
        # the intersection between them
        foreach $e (keys %count) {
                if ($count{$e} == 2) {
                        push @sub, $e;
                }
        }
        undef(@diff);
        return @sub;
}

read_filelist();
# list as on disk
#(undef, undef, undef, undef, undef, undef, undef,
#undef, $listatime, $listmtime, $listctime, undef, undef)
#        = stat($filename);
$listmtime = time() - (3600 * 24);  # yesterday

# filelist as found by the crawler
@currentlist = qw (a b c   e);
@backup      = qw (a b   d e);
@remove      = substract(\@currentlist, \@backup);

foreach $path (@backup) {
        (undef, undef, $mode, undef, $uid, $gid, undef, 
        $size, $atime, $mtime, $ctime, undef, undef) 
                = stat($path);
        
        # with directories don't look a mod. time 
        if (S_ISDIR($mode)) {
                print STDOUT "+d", $path, $delimeter;
        } else {
                if ($#filelist) {
                        # null dump
                        print STDOUT "+ ", $path, $delimeter;
                } else {
                        # compare mod. time
                        # kleiner dan van gemaakt voor testen
                        if ($mtime < $listmtime) {
                                print STDOUT "+ ", $path, $delimeter;
                        }
                }
        }
}

# to be removed
foreach $rm (@remove) {

        (undef, undef, $mode, undef, $uid, $gid, undef, 
        $size, $atime, $mtime, $ctime, undef, undef) 
                = stat($rm);

        if (S_ISDIR($mode)) {
                print STDOUT "-d", $rm, $delimeter;
        } else {
                print STDOUT "- ", $rm, $delimeter;
        }
}

# @backup is gonna be the new @currentlist

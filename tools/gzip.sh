#!/bin/bash
#
# Copyright (c) 2005, 2006 Miek Gieben
# See LICENSE for the license
#
# zip rdup -c's output

set -o nounset

S_ISDIR=16384   # octal: 040000 (This seems to be portable...)
S_ISLNK=40960   # octal: 0120000
S_MMASK=4095    # octal: 00007777, mask to get permission
newsize=0
OPT=""          # gzip or gunzip
PROGNAME=$0

_echo2() {
        echo "** $PROGNAME: $1" > /dev/fd/2
}

cleanup() {
        _echo2 "Signal received while processing \`$path', exiting"
        if [[ ! -z $TMPDIR ]]; then
                rm -rf $TMPDIR
        fi
        exit 1
}
# trap at least these
trap cleanup SIGINT SIGPIPE

usage() {
        echo "$PROGNAME [-d]"
        echo
        echo "compress or decompress the file's contents"
        echo
        echo OPTIONS:
        echo " -d        decompress the files"
        echo " -h        this help"
}

while getopts "dh" o; do
        case $o in
                d) OPT="-d";;
                h) usage && exit;;
                \?) usage && exit;;
        esac
done
shift $((OPTIND - 1))

TMPDIR=`mktemp -d "/tmp/rdup.backup.XXXXXX"`
if [[ $? -ne 0 ]]; then
        _echo2 "Mktemp failed" > /dev/fd/2
        exit 1
fi
chmod 700 $TMPDIR

while read mode uid gid psize fsize
do
        dump=${mode:0:1}        # to add or remove
        mode=${mode:1}          # st_mode bits
        typ=0
        path=`head -c $psize`   # gets the path
        if [[ $(($mode & $S_ISDIR)) == $S_ISDIR ]]; then
                typ=1;
        fi
        if [[ $(($mode & $S_ISLNK)) == $S_ISLNK ]]; then
                typ=2;
        fi
        if [[ $dump == "+" ]]; then
                # add
                case $typ in
                        0)      # REG
                        if [[ $fsize -ne 0 ]]; then
                                # catch
                                head -c $fsize | gzip $OPT -c > $TMPDIR/file.$$.gz
                                newsize=`stat --format "%s" $TMPDIR/file.$$.gz`
                                echo "$dump$mode $uid $gid $psize $newsize"
                                echo -n "$path"
                                cat $TMPDIR/file.$$.gz
                                rm -f $TMPDIR/file.$$.gz
                        else
                                # no content
                                echo "$dump$mode $uid $gid $psize $fsize"
                                echo -n "$path"
                        fi
                        ;;
                        1)      # DIR
                        echo "$dump$mode $uid $gid $psize $newsize"
                        echo -n "$path"
                        ;;
                        2)      # LNK, target is in the content!
                        target=`head -c $fsize`
                        echo "$dump$mode $uid $gid $psize $fsize"
                        echo -n "$path"
                        echo -n "$target"
                        ;;
                esac
        else
                # there is no content
                echo "$dump$mode $uid $gid $psize $fsize"
                echo -n "$path"
        fi
done

rm -rf $TMPDIR

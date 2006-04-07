#!/bin/bash
#
# Copyright (c) 2005, 2006 Miek Gieben
# See LICENSE for the license
#
# This script implement a mirroring backup scheme
# -c is used for remote mirroring

set -o nounset

S_ISDIR=16384   # octal: 040000 (This seems to be portable...)
S_ISLNK=40960   # octal: 0120000
S_MMASK=4095    # octal: 00007777, mask to get permission
remote=0
verbose=0
idir=0; ireg=0; ilnk=0
ftsize=0
ts=`date +%s` # gnuism
restoredir=""
PROGNAME=$0

cleanup() {
        # can also happen when running remotely (no /dev/fd/2)
        echo "** $PROGNAME: Signal received while processing \`$path', exiting" 
        exit 1
}
# trap at least these
trap cleanup SIGINT SIGPIPE

usage() {
        echo "$PROGNAME [OPTIONS]"
        echo
        echo Restore the files in the filelist from rdup
        echo
        echo OPTIONS
        echo " -c      process the file content also (rdup -c), for remote backups"
        echo " -v      echo the files processed to stderr"
        echo " -h      this help"
}

local_mirror() {
        declare -a path # catch spacing in the path
        while read mode uid gid psize fsize path
        do
                dump=${mode:0:1}                # to add or remove
                mode=${mode:1}                  # st_mode bits
                bits=$(($mode & $S_MMASK))      # permission bits
                bits=`printf "%o" $bits`        # and back to octal again
                typ=0
                if [[ $(($mode & $S_ISDIR)) == $S_ISDIR ]]; then
                        typ=1;
                fi
                if [[ $(($mode & $S_ISLNK)) == $S_ISLNK ]]; then
                        typ=2;
                fi
                
                [[ $verbose -eq 1 ]] && echo $path > /dev/fd/2

                if [[ $dump == "+" ]]; then
                        # add
                        case $typ in
                                0)      # REG
                                cat "$path" > "$backupdir/$path"
                                chown $uid:$gid "$backupdir/$path"
                                chmod $bits "$backupdir/$path"
                                ftsize=$(($ftsize + $fsize))
                                ireg=$(($ireg + 1))
                                ;;
                                1)      # DIR
                                # check for fileTYPE changes
                                [[ ! -d "$backupdir/$path" ]] && mkdir -p "$backupdir/$path" 
                                chown $uid:$gid "$backupdir/$path"
                                chmod $bits "$backupdir/$path"
                                idir=$(($idir + 1))
                                ;;
                                2)      # LNK
                                cp -RP "$path" "$backupdir/$path"
                                chown -h $uid:$gid "$backupdir/$path"
                                ilnk=$(($ilnk + 1))
                                ;;
                        esac
                else
                        echo "** $PROGNAME: ignoring remove of \`$path\'"
                fi
        done 
        te=`date +%s`
        echo "** #REG FILES  : $ireg" > /dev/fd/2
        echo "** #DIRECTORIES: $idir" > /dev/fd/2
        echo "** #LINKS      : $ilnk" > /dev/fd/2
        echo "** SIZE        : $(($ftsize / 1024 )) KB" > /dev/fd/2
        echo "** STORED IN   : $backupdir" > /dev/fd/2
        echo "** ELAPSED     : $(($te - $ts)) s" > /dev/fd/2
}

remote_mirror() {
        while read mode uid gid psize fsize
        do
                dump=${mode:0:1}        # to add or remove
                mode=${mode:1}          # st_mode bits
                bits=$(($mode & $S_MMASK)) # permission bits
                bits=`printf "%o" $bits` # and back to octal again
                typ=0
                path=`head -c $psize`   # gets the path
                if [[ $(($mode & $S_ISDIR)) == $S_ISDIR ]]; then
                        typ=1;
                fi
                if [[ $(($mode & $S_ISLNK)) == $S_ISLNK ]]; then
                        typ=2;
                fi

                # check sanity of data?

# debugging - the output of rdup should perfectly match our reads
#echo "$dump$mode $uid $gid $psize $fsize"
#                echo "m{"$mode"}"
#                echo "u{"$uid"}"
#                echo "g{"$gid"}"
#                echo "l{"$psize"}"
#                echo "s{"$fsize"}"
#                echo "p{"$path"}"

                if [[ -e "$backupdir/$path" ]]; then
                        suffix=`mirror_suffix "$backupdir/$path"`
                fi

                if [[ $dump == "+" ]]; then
                        # add
                        case $typ in
                                0)      # REG
                                if [[ $fsize -ne 0 ]]; then
                                        # catch
                                        head -c $fsize > "$backupdir/$path"
                                else 
                                        # empty
                                        touch "$backupdir/$path"
                                fi
                                chown $uid:$gid "$backupdir/$path" 2>/dev/null
                                chmod $bits "$backupdir/$path"
                                ftsize=$(($ftsize + $fsize))
                                ireg=$(( $ireg + 1))
                                ;;
                                1)      # DIR
                                [[ ! -d "$backupdir/$path" ]] && mkdir -p "$backupdir/$path"
                                chown $uid:$gid "$backupdir/$path" 2>/dev/null
                                chmod $bits "$backupdir/$path"
                                idir=$(( $idir + 1))
                                ;;
                                2)      # LNK, target is in the content! 
                                target=`head -c $fsize`
                                ln -sf "$target" "$backupdir/$path" 
                                chown -h $uid:$gid "$backupdir/$path" 2>/dev/null
                                ilnk=$(( $ilnk + 1))
                                ;;
                        esac
                else
                        echo "** $PROGNAME: ignoring remove of \`$path\'"
                fi
        done 
        te=`date +%s`
        # /dev/fd/2 is not available remotely
        echo "** #REG FILES  : $ireg"
        echo "** #DIRECTORIES: $idir"
        echo "** #LINKS      : $ilnk"
        echo "** SIZE        : $(($ftsize / 1024 )) KB"
        echo "** STORED IN   : $backupdir"
        echo "** ELAPSED     : $(($te - $ts)) s"
}

while getopts ":cvh" options; do
        case $options in
                c) remote=1;;
                v) verbose=1;;
                h) usage && exit;;
                \?) usage && exit;;
        esac
done
shift $((OPTIND - 1))

if [[ $remote -eq 0 ]]; then
        local_restore
else
        remote_restore
fi

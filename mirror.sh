#!/bin/bash
#
# Copyright (c) 2005, 2006 Miek Gieben
# See LICENSE for the license
#
# This script implement a mirroring backup scheme
# -c is used for remote mirroring

S_ISDIR=16384   # octal: 040000 (This seems to be portable...)
S_ISLNK=40960   # octal: 0120000
S_MMASK=4095    # octal: 00007777, mask to get permission
remote=0
verbose=0
idir=0; ireg=0; ilnk=0; irm=0
ftsize=0
ts=`date +%s` # gnuism

mirror_suffix() {
	s=$(stat -c '%y' "$@")  # gnuism
	echo "+${s:8:2}.${s:11:5}"
}

mirror_create_top() {
        # need to reverse the order
        dir=$1;
        while [[ $dir != "/" ]]
        do
                dirs="$dir $dirs"
                dir=`dirname $dir`
        done
        for d in $dirs; do
                if [[ ! -d $d ]]; then
                        mkdir -m 755 "$d"
                        chown root:backup "$d"
                fi
        done
}

usage() {
        echo "$0 [OPTIONS]"
        echo
        echo Mirror the filelist from rdup
        echo
        echo FILE - search for this file in the backup directory
        echo
        echo OPTIONS
        echo " -c      process the file content also (rdup -c). Remote backups"
        echo " -b DIR  use DIR as the backup directory, YYYYMM will be added"
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
                                if [[ -f "$backupdir/$path" ]]; then
                                        suffix=`mirror_suffix "$backupdir/$path"`
                                        mv "$backupdir/$path" "$backupdir/$path$suffix"
                                fi
                                cat "$path" > "$backupdir/$path"
                                chown $uid:$gid "$backupdir/$path"
                                chmod $bits "$backupdir/$path"
                                ftsize=$(($ftsize + $fsize))
                                ireg=$(($ireg + 1))
                                ;;
                                1)      # DIR
                                [[ ! -d "$backupdir/$path" ]] && mkdir -p "$backupdir/$path" 
                                chown $uid:$gid "$backupdir/$path"
                                chmod $bits "$backupdir/$path"
                                idir=$(($idir + 1))
                                ;;
                                2)      # LNK
                                if [[ -L "$backupdir/$path" ]]; then
                                        suffix=`mirror_suffix "$backupdir/$path"`
                                        mv "$backupdir/$path" "$backupdir/$path$suffix"
                                fi
                                cp -a "$path" "$backupdir/$path"
                                chown -h $uid:$gid "$backupdir/$path"
                                ilnk=$(($ilnk + 1))
                                ;;
                        esac
                else
                        # remove. It could be the stuff is not there, don't
                        # error on that.
                        if [[ -e "$backupdir/$path" ]]; then
                                suffix=`mirror_suffix "$backupdir/$path"`
                                mv "$backupdir/$path" "$backupdir/$path$suffix"
                        fi
                        irm=$(($irm + 1))
                fi
        done 
        te=`date +%s`
        echo "** #REG FILES  : $ireg" > /dev/fd/2
        echo "** #DIRECTORIES: $idir" > /dev/fd/2
        echo "** #LINKS      : $ilnk" > /dev/fd/2
        echo "** #(RE)MOVED  : $irm" > /dev/fd/2
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

# debugging - the output of rdup should perfectly match our reads
#echo "$dump$mode $uid $gid $psize $fsize"
#                echo "m{"$mode"}"
#                echo "u{"$uid"}"
#                echo "g{"$gid"}"
#                echo "l{"$psize"}"
#                echo "s{"$fsize"}"
#                echo "p{"$path"}"

                if [[ $dump == "+" ]]; then
                        # add
                        case $typ in
                                0)      # REG
                                if [[ -f "$backupdir/$path" ]]; then
                                        suffix=`mirror_suffix "$backupdir/$path"`
                                        mv "$backupdir/$path" "$backupdir/$path$suffix"
                                fi
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
                                # size should be 0
                                [[ ! -d "$backupdir/$path" ]] && mkdir -p "$backupdir/$path"
                                chown $uid:$gid "$backupdir/$path" 2>/dev/null
                                chmod $bits "$backupdir/$path"
                                idir=$(( $idir + 1))
                                ;;
                                2)      # LNK, target is in the content! 
                                if [[ -L "$backupdir/$path" ]]; then
                                        suffix=`mirror_suffix "$backupdir/$path"`
                                        mv "$backupdir/$path" "$backupdir/$path$suffix"
                                fi
                                target=`head -c $fsize`
                                ln -sf "$target" "$backupdir/$path" 
                                chown -h $uid:$gid "$backupdir/$path" 2>/dev/null
                                ilnk=$(( $ilnk + 1))
                                ;;
                        esac
                else
                        # remove
                        if [[ -e "$backupdir/$path" ]]; then
                                suffix=`mirror_suffix "$backupdir/$path"`
                                mv "$backupdir/$path" "$backupdir/$pathsuffix"
                        fi
                        irm=$(( $irm + 1))
                fi
        done 
        te=`date +%s`
        echo "** #REG FILES  : $ireg"
        echo "** #DIRECTORIES: $idir"
        echo "** #LINKS      : $ilnk"
        echo "** #(RE)MOVED  : $irm"
        echo "** SIZE        : $(($ftsize / 1024 )) KB"
        echo "** STORED IN   : $backupdir"
        echo "** ELAPSED     : $(($te - $ts)) s"
}

while getopts ":cNhb:" options; do
        case $options in
                c) remote=1;;
                b) backupdir=$OPTARG;;
                v) verbose=1;;
                h) usage && exit;;
                \?) usage && exit;;
        esac
done
shift $((OPTIND - 1))

if [ -z "$backupdir" ]; then
        backupdir="/vol/backup/`hostname`"
fi
backupdir=$backupdir/`date +%Y%m`

if [[ $remote -eq 0 ]]; then
        local_mirror
else
        remote_mirror
fi

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
newpath=""
PROGNAME=$0

_echo() {
        echo "** $1"
}

_echo2() {
        echo "** $1" > /dev/fd/2
}

cleanup() {
        # can also happen when running remotely (no /dev/fd/2)
        _echo2 "$PROGNAME: Signal received while processing \`$path', exiting"
        exit 1
}
# trap at least these
trap cleanup SIGINT SIGPIPE

usage() {
        echo "$PROGNAME [OPTIONS] DIRECTORY"
        echo
        echo Restore the files in the filelist from rdup to DIRECTORY
        echo DIRECTORY is created when it does not exist
        echo
        echo OPTIONS
        echo " -c      process the file content also (rdup -c), for remote backups"
        echo " -v      echo the files processed to stderr"
        echo " -h      this help"
}

# return a pathname without the suffix: +DD.MM:HH
sanitize() {
        name=""
        if [[ "$1" =~ "(.+)\\+(..)\\.(..):(..)$" ]]; then
                name=${BASH_REMATCH[1]}
        else
                name=$1
        fi
        echo $name
}

local_restore() {
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

                # create the new filename
                newpath=sanitize $path

                # it can be that the file without extension does not 
                # exist in the backup directory because it is removed
                # from disk. If this is the case, skip it
                #
                if [[ ! -e $path ]]; then
                        continue
                fi

                if [[ $dump == "+" ]]; then
                        # add
                        case $typ in
                                0)      # REG
                                cat "$path" > "$restoredir/$newpath"
                                chown $uid:$gid "$restoredir/$newpath"
                                chmod $bits "$restoredir/$newpath"
                                ftsize=$(($ftsize + $fsize))
                                ireg=$(($ireg + 1))
                                ;;
                                1)      # DIR
                                # check for fileTYPE changes
                                [[ ! -d "$restoredir/$newpath" ]] && mkdir -p "$restoredir/$newpath" 
                                chown $uid:$gid "$restoredir/$newpath"
                                chmod $bits "$restoredir/$newpath"
                                idir=$(($idir + 1))
                                ;;
                                2)      # LNK
                                cp -RP "$path" "$restoredir/$newpath"
                                chown -h $uid:$gid "$restoredir/$newpath"
                                ilnk=$(($ilnk + 1))
                                ;;
                        esac
                else
                        _echo2 "$PROGNAME: Ignoring removal of \`$path\'"
                fi
        done 
        te=`date +%s`
        _echo2 "#REG FILES  : $ireg" 
        _echo2 "#DIRECTORIES: $idir" 
        _echo2 "#LINKS      : $ilnk" 
        _echo2 "SIZE        : $(($ftsize / 1024 )) KB" 
        _echo2 "STORED IN   : $restoredir" 
        _echo2 "ELAPSED     : $(($te - $ts)) s" 
}

remote_restore() {
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
#               echo "$dump$mode $uid $gid $psize $fsize"
#               echo "m{"$mode"}"
#               echo "u{"$uid"}"
#               echo "g{"$gid"}"
#               echo "l{"$psize"}"
#               echo "s{"$fsize"}"
#               echo "p{"$path"}"

                # create the new filename
                newpath=sanitize $path

                # it can be that the file without extension does not 
                # exist in the backup directory because it is removed
                # from disk. If this is the case, skip it
                #
                if [[ ! -e $path ]]; then
                        continue
                fi

                if [[ $dump == "+" ]]; then
                        # add
                        case $typ in
                                0)      # REG
                                if [[ $fsize -ne 0 ]]; then
                                        # catch
                                        head -c $fsize > "$restoredir/$newpath"
                                else 
                                        # empty
                                        touch "$restoredir/$newpath"
                                fi
                                chown $uid:$gid "$restoredir/$newpath" 2>/dev/null
                                chmod $bits "$restoredir/$newpath"
                                ftsize=$(($ftsize + $fsize))
                                ireg=$(( $ireg + 1))
                                ;;
                                1)      # DIR
                                [[ ! -d "$restoredir/$newpath" ]] && mkdir -p "$restoredir/$newpath"
                                chown $uid:$gid "$restoredir/$newpath" 2>/dev/null
                                chmod $bits "$restoredir/$newpath"
                                idir=$(( $idir + 1))
                                ;;
                                2)      # LNK, target is in the content! 
                                target=`head -c $fsize`
                                ln -sf "$target" "$restoredir/$newpath" 
                                chown -h $uid:$gid "$restoredir/$newpath" 2>/dev/null
                                ilnk=$(( $ilnk + 1))
                                ;;
                        esac
                else
                        _echo "$PROGNAME: Ignoring removal of \`$path\'"
                fi
        done 
        te=`date +%s`
        # /dev/fd/2 is not available remotely
        _echo "#REG FILES  : $ireg"
        _echo "#DIRECTORIES: $idir"
        _echo "#LINKS      : $ilnk"
        _echo "SIZE        : $(($ftsize / 1024 )) KB"
        _echo "STORED IN   : $restoredir"
        _echo "ELAPSED     : $(($te - $ts)) s"
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

# 1 argument keyfile used for encryption
if [[ $# -eq 0 ]]; then
        _echo "$PROGNAME: Need a directory as argument"
        exit 1
fi
if [[ -f $1 ]]; then
        _echo "$PROGNAME: Cannot restore to \`$'" 
        exit 1
fi

if [[ ! -e $1 ]]; then
        mkdir $1
fi
restoredir=$1

if [[ $remote -eq 0 ]]; then
        local_restore
else
        remote_restore
fi

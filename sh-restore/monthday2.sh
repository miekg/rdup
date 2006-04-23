#!/bin/bash

# Copyright (c) 2005, 2006 Miek Gieben
# See LICENSE for the license
#
# Print a filelist suitable for Restore a entire backed up directory
# +MONTHDAY (restore to MONTHDAY's state)

monthday=0
max=0
min=99
prevfile=""
name=""
PROGNAME=$0
# previous vars
pmode=""
puid=""
pgid=""
ppsize=""
pfsize=""

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
        echo "$PROGNAME [OPTIONS] [+DAY]"
        echo
        echo Print a list suitable for restoring
        echo
        echo +DAY - restore up to this month day
        echo
        echo OPTIONS:
        echo " -c        work with the files' contents (rdup -c)"
        echo " -h        this help"
        echo " -V        print version"
}

version() {
        echo "$PROGNAME: 0.2.8 (rdup-utils)"
}

# echo the file's status line to stdout
print_file_status() {
        # echo status line
        echo -n "${_mode[$1]} ${_uid[$1]} ${_gid[$1]} ${_psize[$1]} ${_fsize[$1]} "
        # echo pathname, if month == 0, there is no suffix
        if [[ ${_month[$1]} -eq 0  ]]; then
                echo "${_path[$1]}"
        else
                printf "%s+%02d.%02d:%02d\n" "${_path[$1]}" ${_month[$1]} ${_hour[$1]} ${_min[$1]}
        fi
}

# cat the file's content to stdout
cat_file() {
        cat ${_tmp_path[$1]}
}

# remove the files stored sofar
rm_tmp_files() {
        for j in ${_seq $1}; do
                rm -f ${_tmp_path[$j]}
        done
}

store() {
        _mode[$i]=$mode
        _uid[$i]=$uid
        _gid[$i]=$gid
        _psize[$i]=$psize
        _fsize[$i]=$fsize
        _path[$i]="$path"
        _tmp_path[$i]="$TMPDIR/$basename.$i"
        _month[$i]=$mo
        _hour[$i]=$ho
        _min[$i]=$mi
        # catch the current file's contents
        head -c $fsize > "${_tmp_path[$i]}"
}

_seq() {
        j=$1
        while [[ $j -ge 0 ]] ; do
                echo $j
                j=$(($j - 1))
        done
}

remote=0
while getopts ":Vh" o; do
        case $o in
                c) remote=1;;
                h) usage && exit;;
                V) version && exit;;
                \?) _echo2 "Invalid option"; usage && exit;;
        esac
done
shift $((OPTIND - 1))

if [[ $# -eq 1 ]]; then
        # we have a + argument
        if [[ $1 =~ "\\+(.+)" ]]; then
                monthday=${BASH_REMATCH[1]}
                if [[ ! $monthday =~ "[0-9]+" ]]; then
                        _echo2 "+DAY must be numerical"
                        exit 1
                fi
        else
                _echo2 "Need a +DAY argument"
                exit 1
        fi
fi

if [[ $monthday -lt 0 || $monthday -gt 31 ]]; then
        _echo2 "+DAY out of bounds [0..31]"
        exit 1
fi

i=0
declare -a _mode
declare -a _uid
declare -a _gid
declare -a _psize
declare -a _fsize
declare -a _path
declare -a _tmp_path
declare -a _month
declare -a _hour
declare -a _min
# work dir
TMPDIR=`mktemp -d "/tmp/rdup.backup.XXXXXX"`
if [[ $? -ne 0 ]]; then
        _echo2 "Mktemp failed" > /dev/fd/2
        exit 1
fi
chmod 700 $TMPDIR

# store everything you get, until we hit
# a new name. Then process what we've got
# and get on with the newer one
#
# all the filenames are received in ascending
# order

while read -r mode uid gid psize fsize
do
        path=$(head -c $psize)
        
        if [[ "$path" =~ "(.+)\\+(..)\\.(..):(..)$" ]]; then
                basename=${BASH_REMATCH[1]}
                mo=${BASH_REMATCH[2]}
                ho=${BASH_REMATCH[3]}
                mi=${BASH_REMATCH[4]}
        else 
                basename="$path"
                mo=0
                ho=0
                mi=0
        fi

        if [[ ! -z "$prevfile" && "$prevfile" != "$basename" ]]; then
                # a new file has been seen. Figure out what to
                # do with the ones we have

                # the first one is the one without suffix
                if [[ $monthday -eq 0 ]]; then
                        print_file_status 0
                        cat_file 0
                        rm_tmp_files $i
                        i=0 
                        store $i
                        continue;
                fi

                if [[ $monthday -gt ${_month[$i]} ]]; then
                        # after any of the versions, use the
                        # plain one
                        print_file_status 0
                        cat_file 0
                        rm_tmp_files $i
                        i=0 
                        store $i
                        continue;
                fi
                
                for j in $(_seq $i); do
                        if [[ $monthday -eq ${_month[$j]} ]]; then
                                # exact match
                                print_file_status $j       
                                cat_file $j
                                rm_tmp_files $i
                                i=0 
                                store $i
                                continue;
                        fi
                        if [[ $monthday -gt ${_month[$j]} ]]; then
                                x=$(($j - 1))
                                print_file_status $x
                                cat_file $x
                                rm_tmp_files $i
                                i=0 
                                store $i
                                continue;
                        fi
                done
                
                # nothing matched, because $monthday is smaller
                # than any of the months we have, use the first
                # stored version.

                if [[ ${_month[1]} -eq 0 ]]; then
                        _echo2 "Huh this have a value!"
                        print_file_status 0
                        cat_file 0
                        rm_tmp_files $i
                        i=0 
                        store $i
                        continue;
                        clear and continue with the rest
                else 
                        print_file_status 1
                        cat_file 1
                        rm_tmp_files $i
                        i=0 
                        store $i
                        continue;
                fi
        else 
                store $i
                i=$(($i + 1))
                prevfile=$basename
        fi

done

#!/bin/bash

# Copyright (c) 2005, 2006 Miek Gieben
# See LICENSE for the license
#
# Print a filelist suitable for Restore a entire backed up directory
# +MONTHDAY (restore to MONTHDAY's state)

set -o nounset

monthday=0
max=0
prevfile=""
name=""
PROGNAME=$0
# previous vars
pmode=""
puid=""
pgid=""
ppsize=""
pfsize=""

usage() {
        echo "$PROGNAME [OPTIONS] [+DAY]"
        echo
        echo print a list suitable for restoring
        echo
        echo +DAY - restore up to this month day
        echo
        echo OPTIONS:
        echo "-h        this help"
}

while getopts "h" o; do
        case $o in
                h) usage && exit;;
                \?) usage && exit;;
        esac
done
shift $((OPTIND - 1))

if [[ $# -eq 1 ]]; then
        # we have a + argument
        if [[ $1 =~ "\\+(.+)" ]]; then
                monthday=${BASH_REMATCH[1]}
                if [[ ! $monthday =~ "[0-9]+" ]]; then
                        echo "** $PROGNAME: +DAY must be numerical" > /dev/fd/2
                        exit 1
                fi
        else
                echo "** $PROGNAME: Need a +DAY argument" > /dev/fd/2
                exit 1
        fi
fi

if [[ $monthday -lt 0 || $monthday -gt 31 ]]; then
        echo "** $PROGNAME: +DAY out of bounds" > /dev/fd/2
        exit 1
fi

declare -a path
while read mode uid gid psize fsize path
do
        if [[ "$path" =~ "(.+)\\+(.+)\\.(.+):(.+)$" ]]; then
                name=${BASH_REMATCH[1]}
                days=$((10#${BASH_REMATCH[2]})) # force base 10
        else 
                name="$path"
                days=99
        fi

        if [[ "$prevfile" != "$name" ]]; then
                # new name
                echo "$pmode $puid $pgid $ppsize $pnewsize"
                echo -n "$prevfile"
                max=0
        else 
                if [[ ($days -le $monthday) && $days -gt $max ]]; then
                        max=$days
                fi
        fi
        
        pmode=$mode
        puid=$uid
        pgid=$gid 
        ppsize=$psize
        pfsize=$fsize
        prevfile="$name"
done

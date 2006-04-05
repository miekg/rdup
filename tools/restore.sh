#!/bin/bash

# Restore a entire backed up directory
# when no options are given restore to yesterday's
# state (i.e. copy all files without the +extenstion)
# -n DAYSAGO (restore to DAYSAGO's state)

set -o nounset

S_ISDIR=16384   # octal: 040000 (This seems to be portable...)
S_ISLNK=40960   # octal: 0120000
S_MMASK=4095    # octal: 00007777, mask to get permission

#monthday=0
monthday=11
max=0
prevfile=""
name=""
declare -a path

# one argument +n where n the monthday of the restore

while read mode uid gid psize fsize path
do
        if [[ "$path" =~ "(.+)\\+(.+)\\.(.+):(.+)$" ]]; then
                name=${BASH_REMATCH[1]}
                days=$((10#${BASH_REMATCH[2]})) # force base 10
                #hour=$((10#${BASH_REMATCH[3]}))
                #min=$((10#${BASH_REMATCH[4]}))
        else 
                name="$path"
                days=99
                #hour=0
                #min=0
        fi

        if [[ "$prevfile" != "$name" ]]; then
                # new name
                echo $max "--" $prevfile
                max=0
        else 
                if [[ ($days -le $monthday) && $days -gt $max ]]; then
                        max=$days
                fi
        fi
        
        

        prevfile="$name"
done

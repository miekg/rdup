#!/bin/bash
#
# Copyright (c) 2005, 2006 Miek Gieben
# See LICENSE for the license
#
# Exclude certain FILES from getting back upped 
# Proof of concept as forking a grep on EACH entry
# is way too slow
S_ISDIR=16384   # octal: 040000 (This seems to be portable...)
S_ISLNK=40960   # octal: 0120000
S_MMASK=4095    # octal: 00007777, mask to get permissions

declare -a path # catch spacing in the path
while read mode uid gid path
do
        dump=${mode:0:1}        # to add or remove
        mode=${mode:1}          # st_mode bits
        typ=0
        if [[ $(($mode & $S_ISDIR)) == $S_ISDIR ]]; then
                typ=1;
        fi
        if [[ $(($mode & $S_ISLNK)) == $S_ISLNK ]]; then
                typ=2;
        fi
        
        case $typ in
                0|2)      # reg file or link
                echo "$path" | egrep '\.swp$' > /dev/null
                if [[ $? != 0 ]]; then
                        echo "$dump$mode $uid $gid $path"
                fi
                ;;
                1)      # directory
                ;;
        esac
done 

#!/bin/bash
#
# Copyright (c) 2005, 2006 Miek Gieben
# See LICENSE for the license
#
# grep on rdup's output

declare -a fileexcludelist
declare -a direxcludelist
fileexcludelist=".slide_img.* .thumb_img.*"
direxcludelist="^/lost+found/ ^/proc/ ^/dev/ ^/sys/ .Trash/ .Cache/"

S_ISDIR=16384   # octal: 040000 (This seems to be portable...)
S_ISLNK=40960   # octal: 0120000
S_MMASK=4095    # octal: 00007777, mask to get permission

declare -a path # catch spacing in the path
while read mode uid gid psize fsize path
do
        dump=${mode:0:1}        
        mode=${mode:1}         
        typ=0
        if [[ $(($mode & $S_ISDIR)) == $S_ISDIR ]]; then
                typ=1;
        fi
        if [[ $(($mode & $S_ISLNK)) == $S_ISLNK ]]; then
                typ=2;
        fi
        
        case $typ in
                0|2)      # reg file or link
                for file in $fileexcludelist; do 
                        if [[ "$path" =~ $i ]]; then
                                continue 2 # jump to next while
                        fi
                done
                # don't exclude, print it
                echo "$dump$mode $uid $gid $psize $fsize $path"
                ;;
                1)      # directory
                for i in $direxcludelist; do 
                        if [[ "$path" =~ $i ]]; then
                                continue 2
                        fi
                done
                # don't exclude, print it
                echo "$dump$mode $uid $gid $psize $fsize $path"
                ;;
        esac
done 

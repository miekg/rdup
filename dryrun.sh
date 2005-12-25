#!/bin/bash
#
# Copyright (c) 2005, 2006 Miek Gieben
# See LICENSE for the license
#
# echo what you want to do to stdout
# backup.sh /target
# in /target/YYYYMM/ a mirror is created
S_ISDIR=16384   # octal: 040000 (This seems to be portable...)
S_ISLNK=40960   # octal: 0120000
S_MMASK=4095    # octal: 00007777, mask to get permissions
backupdir=$1
suffix=`date +%Y%m%d.%H:%M`  # YYYYMMDD.HH:MM
bsuffix=`date +%Y%m` # YYYYMM
if [ -z $backupdir ]; then 
        echo "** Need archive directory"
        exit 1
fi
#mkdir -p $backupdir; mkdir -p $backupdir/$bsuffix
#chown root:backup $backupdir; chown root:backup $backupdir/$bsuffix
#chmod 755 $backupdir; chmod 755 $backupdir/$bsuffix
#backupdir=$backupdir/$bsuffix

while read mode uid gid path
do
        dump=${mode:0:1}        # to add or remove
        mode=${mode:1}          # st_mode bits
        bits=$(($mode & $S_MMASK)) # permission bits
        bits=`printf "%o" $bits` # and back to octal again
        typ=0
        if [[ $(($mode & $S_ISDIR)) == $S_ISDIR ]]; then
                typ=1;
        fi
        if [[ $(($mode & $S_ISLNK)) == $S_ISLNK ]]; then
                typ=2;
        fi
        
        if [[ $dump == "+" ]]; then
                # add
                case $typ in
                        0)      # reg file
                        echo "cat $path > $backupdir/$path"
                        ;;
                        1)      # directory
                        echo mkdir -p $backupdir/$path
                        ;;
                        2)      # link
                        echo cp -a $path $backupdir/$path
                        ;;
                esac
                echo chown $uid:$gid $backupdir/$path
                echo chmod $bits $backupdir/$path
        else
                # remove
                echo mv $backupdir/$path $backupdir/$path.$suffix
        fi
done 

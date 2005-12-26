#!/bin/bash
#
# Copyright (c) 2005, 2006 Miek Gieben
# See LICENSE for the license
#
# This actually creates the backup
# bu /target
# in /target/YYYYMM/ a mirror is created
S_ISDIR=16384   # octal: 040000 (This seems to be portable...)
S_ISLNK=40960   # octal: 0120000
S_MMASK=4095    # octal: 00007777, mask to get permissions
backupdir=$1
suffix=`date +%Y%m%d.%H:%M`  # YYYYMMDD.HH:MM
bsuffix=`date +%Y%m` # YYYYMM
while getopts ":n:b:cCd" options; do
        case $options in
                b) backupdir=$OPTARG; shift;;
        esac
done
if [ -z $backupdir ]; then 
        echo "** Setting archive directory to /vol/backup/`hostname`"
        backupdir="/vol/backup/`hostname`"
fi
mkdir -p $backupdir; mkdir -p $backupdir/$bsuffix
chown root:backup $backupdir; chown root:backup $backupdir/$bsuffix
chmod 755 $backupdir; chmod 755 $backupdir/$bsuffix
backupdir=$backupdir/$bsuffix

declare -a path # catch spacing in the path
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
                        [ -f "$backupdir/$path" ] && mv "$backupdir/$path" "$backupdir/$path.$suffix"
                        cat "$path" | gzip -c > "$backupdir/$path"
                        ;;
                        1)      # directory
                        [ ! -d "$backupdir/$path" ] && mkdir -p "$backupdir/$path"
                        ;;
                        2)      # link
                        [ -L "$backupdir/$path" ] && mv "$backupdir/$path" "$backupdir/$path.$suffix"
                        cp -a "$path" "$backupdir/$path"
                        ;;
                esac
                chown $uid:$gid "$backupdir/$path"
                chmod $bits "$backupdir/$path"
        else
                # remove
                mv "$backupdir/$path" "$backupdir/$path.$suffix"
        fi
done 

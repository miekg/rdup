#!/bin/bash
#
# Copyright (c) 2005, 2006 Miek Gieben
# See LICENSE for the license
#
# This actually creates the backup
# bu /target
# in /target a mirror is created
# the file contents is gzipped
S_ISDIR=16384   # octal: 040000 (This seems to be portable...)
S_ISLNK=40960   # octal: 0120000
S_MMASK=4095    # octal: 00007777, mask to get permissions
backupdir=$1
suffix=`date +%Y%m%d.%H:%M`  # YYYYMMDD.HH:MM
if [ -z $backupdir ]; then 
        echo "** Need archive directory"
        exit 1
fi
mkdir -p $backupdir
chown root:backup $backupdir
chmod 755 $backupdir

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
                        cat $path | gzip -c > $backupdir/$path
                        ;;
                        1)      # directory
                        [ ! -d $backupdir/$path ] && mkdir -p $backupdir/$path
                        ;;
                        2)      # link
                        cp -a $path $backupdir/$path
                        ;;
                esac
                chown $uid:$gid $backupdir/$path
                chmod $bits $backupdir/$path
        else
                # remove
                mv $backupdir/$path $backupdir/$path.$suffix
        fi
done 

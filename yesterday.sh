#!/bin/bash
#
# Copyright (c) 2005, 2006 Miek Gieben
# See LICENSE for the license
#
# inspired by yesterday of plan9
# -n days ago
# -c copy
# -C copy carefull
# -d diff -u

what=0
while getopts ":n:cCd" options; do 
        case $options in
                n) ;;
                c) what=1; shift;;
                C) what=2; shift;;
                d) what=3; shift;;
        esac
done
backupdir=$1
bsuffix=`date +%Y%m`
if [ -z $backupdir ]; then
        echo "** Need archive directory"
        exit 1
fi
backupdir=$backupdir/$bsuffix

while shift
do
        file=$1
        if [[ -z $file ]]; then
                continue
        fi
        if [[ ${file:0:1} != "/" ]]; then
                file=`pwd`/$file
        fi
        [ ! -e $backupdir$file ] && \
                echo "** Not found in archive: $file" && continue

        # print
        case $what in
                0)
                        $backupdir$file
                ;;
                1)
                        cp -a $backupdir$file $file
                ;;
                2)
                        cmp $backupdir$file $file > /dev/null
                        if [[ $? ]]; then
                                cp -a $backupdir$file $file
                        fi
                ;;
                3)
                        echo diff -u `basename $backupdir$file` `basename $file`
                        [ -f $backupdir$file ] && diff -u $backupdir$file $file
                        [ -h $backupdir$file ] && diff -u $backupdir$file $file
                ;;
        esac
done

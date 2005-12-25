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
bsuffix=`date +%Y%m`
what=0
while getopts ":n:b:cCd" options; do 
        case $options in
                n) ;;
                b) backupdir=$OPTARG; shift;;
                c) what=1; shift;;
                C) what=2; shift;;
                d) what=3; shift;;
        esac
done
if [ -z $backupdir ]; then
        echo "** Setting archive directory to /vol/backup"
        backupdir="/vol/backup"
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
                        echo $backupdir$file
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

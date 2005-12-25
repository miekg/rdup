#!/bin/bash

# inspired by yesterday of plan9
# -n days ago
# -c copy
# -C copy carefull
# -d diff -u
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
        echo $backupdir$file
done

#        # copy
#        echo cp -a $backupdir$file .
#        # copy careful
#        cmp $backupdir$file $file > /dev/null
#        if [[ $? ]]; then
#                # copy only when the differ
#                echo cp -a $backupdir$file .
#        fi
#
#        # diff, only for files and links
#        echo diff -u `basename $backupdir$file` `basename $file`
#        [ -f $backupdir$file ] && diff -u $backupdir$file $file
#        [ -h $backupdir$file ] && diff -u $backupdir$file $file
#        exit 0

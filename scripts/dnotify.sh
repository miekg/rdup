#!/bin/bash

# helper script to run rdup on a mounted usb backup disk
# called by dnotify as
# dnotify -e dnotify.sh {} dir [dir | ... ]
# each dir is backed up

# usb backup disklayout
# /
# /etc
#     /elektron-timestamp
#     /host2-timestamp
#
# /elektron
#          /200601
#          /200602
#          ...
# /host2
#       /200601
#       /200602
#       ....
# /host3
#       /...

# what directory has changed? And is our disk
h=`hostname`
files=`ls -t $1`
mountpoint=`echo $files | awk '{ print $1 } '`
hostname=`ls $1/$mountpoint | sort -nr | head -1`

# what is the latest backup of this host
# $changed/host -> backupdir
# $changed/etc/host -> admin dir

etc="$1/$mountpoint/$hostname/etc"
backuphost="$1/$mountpoint/$hostname/$h"

echo $mountpoint
echo $hostname
echo $etc
echo $backuphost

# now if we are in the same month we do an incremental
# otherwise we do a full dump
d=`date +%Y%m`

if [[ $d != $latest ]]; then
        DUMP=null
        echo rm -f "$etc/$h-timestamp"
else
        DUMP=incr
fi

echo $d
echo $DUMP
backupdir=$backuphost/$d

if [[ ! -e "$backupdir" ]]; then
        echo    mkdir -p "$backupdir"
fi
if [[ ! -e "$etc" ]]; then
        echo    mkdir -p "$etc"
fi

# dirs in $@
shift

echo /usr/sbin/rdup -N "$etc/$h-timestamp" "$etc/$h-list" $@ 
echo /usr/sbin/mirror.sh -b $backupdir

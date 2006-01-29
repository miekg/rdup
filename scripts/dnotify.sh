#!/bin/bash

# helper script to run rdup on a mounted usb backup disk
# called by dnotify as
# dnotify -e dnotify.sh {}

# usb backup disklayout
# /
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
files=`ls -t $1`
changed=`echo $files | awk '{ print $1 } '`

# what is the latest backup of this host
latest=`ls $1/$changed | sort -nr | head -1`

# now if we are in the same month we do an incremental
# otherwise we do a full dump
d=`date +%Y%m`

if [[ $d != $latest ]]; then
        DUMP=full
else
        DUMP=incr
fi

echo $DUMP
echo $changed
echo $latest

# rdup away



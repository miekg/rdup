#!/bin/bash

# start rdup and perform some locking

. /home/miekg/miek.nl/php/rdup.rc
#mount $SMBSHARE
echo $LOCKFILE

# some defaults
FREE=${FREE:-5}

blocksize=`stat -tf /tmp | awk '{ print $5 }'`
free=`stat -tf /tmp | awk '{ print $8 }'`
((bytes=blocksize*free))
((bytes=bytes/1024/1024/1024))
if [ $bytes -lt $FREE ]; then
        echo "Not enough free space"
fi

if ! (umask 222; echo $$ >$LOCKFILE) 2>/dev/null; then
        echo "No lock"
        exit 1
fi

if [ -z $DIRECTORY ]; then
        echo "No dirs"
        exit 2
fi

if [ -z $BACKUP ]; then
        echo "Nothing to backup"
        exit 3
fi

if [ $COMPRESSION = "1" ]; then
        OPT="-z";
fi
echo /usr/sbin/rdup $OPT -b $BACKUP $DIRECTORIES
rm -f $LOCKFILE

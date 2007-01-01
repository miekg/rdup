#!/bin/sh

# start rdup and perform some locking

. /home/miekg/miek.nl/php/rdup.rc
echo $LOCKFILE

if ! (umask 222; echo $$ >$LOCKFILE) 2>/dev/null; then
        echo "No lock"
        exit 1
fi

if [ -z $DIRECTORIES ];then
        echo "No dirs"
        exit 2
fi

if [ -z $BACKUP ];then
        echo "Nothing to backup"
        exit 3
fi

if [ $COMPRESSION = "1" ]; then
        OPT="-z";
fi

/usr/sbin/rdup $OPT -b $BACKUP $DIRECTORIES
rm -f $LOCKFILE

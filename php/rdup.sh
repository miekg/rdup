#!/bin/bash

. /home/miekg/miek.nl/php/rdup.rc

function T {
        STRING=`echo -n $1 | sha1sum | awk ' { print $1 } '`
        TRANS=`grep "$LANG $STRING" lang.txt`

        if [ -z "$TRANS" ]; then
                echo $1
        fi
        echo -n ${TRANS:43}
}

# start rdup and perform some locking

#mount $SMBSHARE

echo `T configuration`

exit

ERR=""
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
        ERR="Lockfile could not be acquired"
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

case $LANG in
        NL)
        HEADER_OK=<<EOF
De backup is successvol verlopen,zie onderstaand rapport.

EOF
        HEADER_NOK=<<EOF
Er is een probleem opgetreden tijdens het maken van de backup.

EOF
        ;;
        EN)
        *)
        HEADER_OK=<<EOF
The backup was successfull, see the report below.

EOF
        HEADER_NOK=<<EOF
There was a problem when performing the backup.

EOF
        ;;
esac

REPORT=`tempfile`
echo $HEADER > $REPORT
echo /usr/sbin/rdup $OPT -b $BACKUP $DIRECTORIES  # >> $REPORT
exit_code=$?
rm -f $LOCKFILE

#!/bin/sh

# all relative to /etc/rdup
# $1 = timestamp file
# $2 = file list
# $3 ... = dirs

# so we need:
# dump null timestamp.home filelist.home /home

type=$1  # null/incr
name=$2
# dirs in $@
shift
shift

if [[ $type == "null" ]]; then
        # kill the timestamp and inc list
        rm -f /etc/rdup/ts-$name /etc/rdup/list-$name
fi

/usr/sbin/rdup -N /etc/rdup/ts-$name /etc/rdup/list-$name $dir | \
/usr/sbin/excl.sh | /usr/sbin/mirror.sh

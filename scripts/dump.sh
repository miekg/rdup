#!/bin/sh

# small script to tie a backup together

# all relative to /etc/rdup
# $1 = timestamp file
# $2 = file list
# $3 = dir

# so we need:
# dump null timestamp.home filelist.home /home

type=$1  # null/incr
ts=$2
list=$3
dir=$4

if [[ $type == "null" ]]; then
        # kill the timestamp
        rm -f /etc/rdup/$ts
        # kill the inc list
        rm -f /etc/rdup/$list
fi

/usr/sbin/rdup -N /etc/rdup/$ts /etc/rdup/$list $dir | /usr/sbin/excl | /usr/sbin/backup

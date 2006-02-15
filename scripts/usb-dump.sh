#!/bin/sh

# get the path were we live
if [[ $0 =~ ^/ ]]; then
        p=$0
 else
        p=`pwd`/$0
fi

d=`date +%Y%m`
mountpath=`dirname $p`

# only to get root 
gksudo -m "Perform backup of $HOSTNAME to $mountpath as root?" -t "rdup @ $HOSTNAME" "cat /dev/null"
if [[ $? -ne 0 ]]; then
        exit
fi

## Set the directories ##
STAMP="$mountpath/$HOSTNAME/$HOSTNAME.timestamp"
LIST="$mountpath/$HOSTNAME/$HOSTNAME.list"
BACKUPDIR="$mountpath/$HOSTNAME/$d"

# create top-level backup dir
sudo mkdir -p "$mountpath/$HOSTNAME"

if [[ ! -d "$BACKUPDIR" ]]; then
        # kill the timestamp and inc list
        sudo      mkdir -p "$BACKUPDIR"
        sudo      rm -f "$LIST"
        sudo      rm -f "$STAMP"
fi


case $HOSTNAME in
        elektron*)
        DIRS="   "
        sudo /usr/sbin/rdup -N "$STAMP" "$LIST" $DIRS |\
        /usr/sbin/mirror.sh -b "$BACKUPDIR"
        ;;

        floep*)
        DIRS="   "
        sudo /usr/sbin/rdup -N "$STAMP" "$LIST" $DIRS |\
        /usr/sbin/mirror.sh -b "$BACKUPDIR"
        ;;
esac

#!/bin/sh

usage() {
        echo "$0 [OPTIONS] NAME DIR [DIR ...]"
        echo 
        echo NAME - suffix for filelist and timestamp files
        echo DIR  - directories to back up
        echo
        echo OPTIONS
        echo "-b DIR  backup directory. Default: /vol/backup/HOSTNAME"
        echo "-e      filelist and timestamp are put in backup directory"
        echo "-h      this help"
}

d=`date +%Y%m`
etc=0

while getopts ":b:eh" o; do
        case $o in
                b) BACKUPDIR=$OPTARG;;
                e) etc=1;;
                h) usage && exit;;
                \?) usage && exit;;
        esac
done
shift $((OPTIND - 1))
if [[ -z $1 ]]; then
        echo "NAME is mandatory"
        exit 1
fi
if [[ -z $BACKUPDIR ]]; then
        BACKUPDIR="/vol/backup/$HOSTNAME"
fi

if [[ $etc -eq 0 ]]; then
        ETC="/etc/rdup"
else
        ETC="$BACKUPDIR"
fi

NAME=$1  
BACKUPDIR_DATE="$BACKUPDIR/$d"
STAMP="$ETC/$HOSTNAME.$NAME.timestamp"
LIST="$ETC/$HOSTNAME.$NAME.list"
# DIRS in $@
shift

mkdir -p "$BACKUPDIR"
if [[ ! -d "$BACKUPDIR_DATE" ]]; then
        # kill the timestamp and inc list
        mkdir -p "$BACKUPDIR_DATE"
        rm -f "$LIST"
        rm -f "$STAMP"
fi

if [[ -f /usr/bin/excl.sh ]]; then
        /usr/sbin/rdup -N "$STAMP" "$LIST" $@ |\
        /usr/sbin/excl.sh  /usr/sbin/mirror.sh -b $BACKUPDIR
else
        /usr/sbin/rdup -N "$STAMP" "$LIST" $@ |\
        /usr/sbin/mirror.sh -b $BACKUPDIR
fi

#!/bin/sh

# create a (mirror) backup in /vol/backup/`hostname`
# figure out of the dump should be a full one or incremental


usage() {
        echo "$0 [OPTIONS] NAME DIR [DIR ...]"
        echo 
        echo NAME - suffix for filelist and timestamp files
        echo DIR \ - directories to back up
        echo
        echo OPTIONS
        echo "-b DIR     backup directory. Default: /vol/backup/HOSTNAME"
        echo "-e         filelist and timestamp are put in backup directory"
        echo "-x SCRIPT  use SCRIPT as exclude script"
        echo "-h         this help"
}

d=`date +%Y%m`
etc=0
exclude=""

while getopts ":b:eh" o; do
        case $o in
                b) BACKUPDIR=$OPTARG;;
                e) etc=1;;
                x) exclude=$$OPTARG;;
                h) usage && exit;;
                \?) usage && exit;;
        esac
done
shift $((OPTIND - 1))
if [[ -z $1 ]]; then
        echo "** NAME is mandatory" > /dev/fd/2
        exit 1
fi
if [[ -z $BACKUPDIR ]]; then
        BACKUPDIR="/vol/backup/$HOSTNAME" 
fi

# where to put the admin files.
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
if [[ -z $@ ]]; then
        echo "** No directories to backup" > /dev/fd/2
        exit 1
fi

# check the first directory in the list
mkdir -p "$BACKUPDIR"
if [[ ! -d "$BACKUPDIR_DATE/$1" ]]; then
        # kill the timestamp and inc list
        mkdir -p "$BACKUPDIR_DATE"
        rm -f "$LIST"
        rm -f "$STAMP"
        echo "** Full dump" > /dev/fd/2
else
        echo "** Incremental dump" > /dev/fd/2
fi

if [[ ! -z $exclude ]]; then
        /usr/sbin/rdup -N $STAMP $LIST $@ | $exclude | /usr/sbin/mirror.sh -b $BACKUPDIR
else
        /usr/sbin/rdup -N $STAMP $LIST $@ | /usr/sbin/mirror.sh -b $BACKUPDIR
fi

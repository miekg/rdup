#!/bin/bash

# create a (mirror) backup in /vol/backup/`hostname`
# figure out of the dump should be a full one or incremental

set -o nounset

usage() {
        echo "$PROGNAME [OPTIONS] NAME DIR [DIR ...]"
        echo 
        echo NAME - suffix for filelist and timestamp files
        echo DIR \ - directories to back up
        echo
        echo OPTIONS:
        echo " -b DIR     backup directory. Default: /vol/backup/HOSTNAME"
        echo " -e         filelist and timestamp are put in backup directory"
        echo " -x SCRIPT  use SCRIPT as exclude script"
        echo " -h         this help"
        echo " -V         print version"
}

version() {
        echo "$PROGNAME: @PACKAGE_VERSION@ (rdup-utils)"
}

d=`date +%Y%m`
etc=0
exclude=""
PROGNAME=$0
BACKUPDIR=""

_echo2() {
        echo "** $PROGNAME: $1" > /dev/fd/2
}

while getopts ":b:x:ehV" o; do
        case $o in
                b) BACKUPDIR=$OPTARG;;
                e) etc=1;;
                x) exclude=$OPTARG;;
                V) version && exit;;
                h) usage && exit;;
                \?) usage && exit;;
        esac
done
shift $((OPTIND - 1))
if [[ $# -eq 0 ]]; then
        echo "NAME is mandatory"
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
        echo "No directories to backup" 
        exit 1
fi

# check the first directory in the list
mkdir -m 755 -p "$BACKUPDIR"            # gnuism?
if [[ ! -d "$BACKUPDIR_DATE/$1" ]]; then
        # kill the timestamp and inc list
        mkdir -m 755 -p "$BACKUPDIR_DATE"
        rm -f "$LIST"
        rm -f "$STAMP"
        _echo2 "Full dump"
else
        _echo2 "Incremental dump"
fi

if [[ ! -z $exclude ]]; then
        /usr/sbin/rdup -N $STAMP $LIST $@ | $exclude | /usr/sbin/mirror.sh -b $BACKUPDIR
else
        /usr/sbin/rdup -N $STAMP $LIST $@ | /usr/sbin/mirror.sh -b $BACKUPDIR
fi

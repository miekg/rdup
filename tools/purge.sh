#!/bin/bash

# Purge older backup directories
# -b directory -x X : X months ago

set -o nounset
PROGNAME=$0

usage() {
        echo "$PROGNAME [OPTIONS] HOST" 
        echo
        echo HOST - delete backups from this host
        echo
        echo OPTIONS:
        echo " -b DIR     backup directory. Default: /vol/backup/HOSTNAME"
        echo " -x NUM     delete backups from X months ago, defaults to 2"
        echo " -y         yes, remove my backups and don't ask questionse"
        echo " -h         this help"
}

monthsago() {
        echo `date --date "$1 months ago" +%Y%m` # YYYYMM
}

question () {
    printf "%s (y/n) " "$*"
    read answer
    case "$answer" in
        [Yy]|[Yy][Ee][Ss])
            return 0
            ;;
        *)
            return 1
            ;;
    esac
}

BACKUPDIR=""
MONTHS=2
ASK=1
while getopts ":b:x:hy" o; do
        case $o in
                b) BACKUPDIR=$OPTARG;;
                x) MONTHS=$OPTARG;;
                y) ASK=0;;
                h) usage && exit;;
                \?) usage && exit;;
        esac
done
shift $((OPTIND - 1))

if [[ $# -eq 0 ]]; then
        echo "** $PROGNAME: HOST is mandatory" > /dev/fd/2
        exit 1
fi
if [[ -z $BACKUPDIR ]]; then
        BACKUPDIR="/vol/backup/"
fi
if [[ $MONTHS -lt 2 ]]; then
        echo "** $PROGNAME: Will not delete backups less than 2 months old" > /dev/fd/2
fi

# setup the backup directory
BACKUPDIR=$BACKUPDIR/$1
D=`monthsago $MONTHS`
DEL=$BACKUPDIR/$D

if [[ $ASK -eq 1 ]]; then
        question "** $PROGNAME: Continue and remove $DEL?" || exit 1
fi

if [[ -z $DEL ]]; then
        echo "** $PROGNAME: No directory?" > /dev/fd/2
        exit 1
fi

rm -rf $DEL

#!/bin/sh

# Purge older backup directories
# -b directory -x X : X months ago

set -o nounset

usage() {
        echo "$0 [OPTIONS] HOST" 
        echo
        echo HOST - delete backups from this host
        echo
        echo OPTIONS:
        echo "-b DIR     backup directory. Default: /vol/backup/HOSTNAME"
        echo "-x NUM     delete backups from X months ago, defaults to 2"
        echo "-h         this help"
}

monthsago() {
        echo `date --date "$1 months ago" +%Y%m` # YYYYMM
}

BACKUPDIR=""
MONTHS=2
while getopts ":b:x:h" o; do
        case $o in
                b) BACKUPDIR=$OPTARG;;
                x) MONTHS=$OPTARG;;
                h) usage && exit;;
                \?) usage && exit;;
        esac
done
shift $((OPTIND - 1))

if [[ $# -eq 0 ]]; then
        echo "** $0: HOST is mandatory" > /dev/fd/2
        exit 1
fi
if [[ -z $BACKUPDIR ]]; then
        BACKUPDIR="/vol/backup/$HOSTNAME"
fi
if [[ $MONTHS -lt 2 ]]; then
        echo "** $0: Will not delete backups less than 2 months old" > /dev/fd/2
fi

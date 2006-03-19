#!/bin/bash

# create a dump on a external harddisk

set -o nounset

##
# For each HOST you should define the directories to backup 
##
case $HOSTNAME in
        elektron*)
        DIRS="/home/miekg/"
        ;;

        floep*)
        DIRS=""
        ;;
        *)
        # no such host
        zenity --error --title "rdup @ $HOSTNAME" --text "No backups defined for <b>$HOSTNAME</b>"
        exit 1
esac
##
# Don't need to edit anything below this line
##

if [ ! -x /usr/sbin/rdup ]; then 
        zenity --error --title "rdup @ $HOSTNAME" --text "rdup can not be found"
        exit 1
fi

# get the path were we live
if [[ $0 =~ ^/ ]]; then
        p=$0
 else
        p=`pwd`/$0
fi

d=`date +%Y%m`
mountpath=`dirname $p`

if [[ -z $DIRS ]]; then
         zenity --error --title "rdup @ $HOSTNAME" --text "No backup directories defined"
         exit 1
fi

# only to get root 
gksudo -m "Perform backup of <b>$HOSTNAME</b> to $mountpath as root?" -t "rdup @ $HOSTNAME" "cat /dev/null"
if [[ $? -ne 0 ]]; then
        exit
fi

## Set the directories ##
STAMP="$mountpath/$HOSTNAME/$HOSTNAME.timestamp"
LIST="$mountpath/$HOSTNAME/$HOSTNAME.list"
BACKUPDIR="$mountpath/$HOSTNAME"
BACKUPDIR_DATE="$mountpath/$HOSTNAME/$d"

# create top-level backup dir
sudo mkdir -m 755 -p $BACKUPDIR
if [[ ! -d "$BACKUPDIR_DATE" ]]; then
        # kill the timestamp and inc list
        sudo mkdir -m755 -p "$BACKUPDIR_DATE"
        sudo rm -f "$LIST"
        sudo rm -f "$STAMP"
        TEXT="<i>Full dump</i> of <b>HOSTNAME</b> completed"
else
        TEXT="<i>Incremental dump</i> of <b>$HOSTNAME</b> completed"
fi

sudo /usr/sbin/rdup -N $STAMP $LIST $DIRS |\
sudo /usr/sbin/mirror.sh -b $BACKUPDIR 2>&1 |\
mail -s "$TEXT" root@localhost
# backup completed
zenity --info --title "rdup @ $HOSTNAME" --text "$TEXT"

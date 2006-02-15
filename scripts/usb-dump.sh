#!/bin/sh

##
# For each HOST you should define the directories to backup 
##
case $HOSTNAME in
        elektron*)
        DIRS=
        ;;

        floep*)
        DIRS=
        ;;
esac

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
        TIMESTAMP=
        TEXT="Full dump in progress..."
else
        TIMESTAMP="-N $STAMP"
        TEXT="Incremental dump in progress..."
fi

sudo /usr/sbin/rdup $TIMESTAMP $LIST $DIRS |\
/usr/sbin/mirror.sh -b $BACKUPDIR |\
zenity --progress --pulsate -t "rdup @ $HOSTNAME" --text $TEXT

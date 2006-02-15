#!/bin/sh

##
# For each HOST you should define the directories to backup 
##
case $HOSTNAME in
        elektron*)
        DIRS="/home/miekg/bin"
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
BACKUPDIR="$mountpath/$HOSTNAME"
BACKUPDIR_DATE="$mountpath/$HOSTNAME/$d"

# create top-level backup dir
sudo mkdir -p $BACKUPDIR
if [[ ! -d "$BACKUPDIR_DATE" ]]; then
        # kill the timestamp and inc list
        sudo mkdir -p "$BACKUPDIR_DATE"
        sudo rm -f "$LIST"
        sudo rm -f "$STAMP"
        TIMESTAMP=
        TEXT="Full dump of $HOSTNAME completed"
else
        TIMESTAMP="-N $STAMP"
        TEXT="Incremental dump of $HOSTNAME completed"
fi

echo $TEXT
echo  /usr/sbin/rdup $TIMESTAMP $LIST $DIRS 
echo  /usr/sbin/mirror.sh -b $BACKUPDIR
sudo /usr/sbin/rdup $TIMESTAMP $LIST $DIRS |\
sudo /usr/sbin/mirror.sh -b $BACKUPDIR

zenity --info --title "rdup @ $HOSTNAME" --text "$TEXT"

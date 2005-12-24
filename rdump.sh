#!/bin/bash

# 1. create a dir for the backup
# 2. link null to dir: null -> dir
# 3. incremental dumps, dump to null
# 4. a new null dump removes the link and
#    goes to step 1

case $1 in
        elektron-etc)
                DIR="/etc"
                HOST=$1
        ;;
        elektron-homes)
                DIR="/home"
                HOST=$1
        ;;
        elektron-var-mail)
                DIR="/var/mail"
                HOST=$1
        ;;
        elektron-var-svn)
                DIR="/var/svn"
                HOST=$1
        ;;
        elektron-var-lib)
                DIR="/var/lib"
                HOST=$1
        ;;
        *)
                echo "Need a host-def as argument"
                exit 1
        ;;
esac

ARCHIVEDIR=/vol/backup
EXCLUDE="--exclude .svn/ --exclude tmp/ --exclude .slide_img --exclude
.thumb_img --exclude lost+found --exclude .Cache/"
SUFFIX=`date +%Y%m%d%H%M`
NULLDIR=`date +%Y%m`
OPT="-a --stats --compress --delete -b"
# .nobackup files are interpreted by rsync
FILTER=.nobackup

case $2 in
        null)
                mkdir -p $ARCHIVEDIR/$HOST/$NULLDIR
                chmod 755 $ARCHIVEDIR/$HOST # set liberal permissions
                chmod 755 $ARCHIVEDIR/$HOST/$NULLDIR # set liberal permissions
                # remove prev. null link, and add the new one
                ( cd $ARCHIVEDIR/$HOST; rm -f null )
                ( cd $ARCHIVEDIR/$HOST; ln -sf $NULLDIR null )
                for i in $DIR ; do 
                    rsync $EXCLUDE --filter ": /$FILTER" $OPT --suffix $SUFFIX $i $ARCHIVEDIR/$HOST/null
                done
        ;;
        inc|incremental)
                # this should also work for remote
                for i in $DIR ; do 
                        rsync --filter ": /$FILTER" $OPT --suffix $SUFFIX $i $ARCHIVEDIR/$HOST/null
                done
        ;;
        *)
                echo "Need: inc or null as argument"
                exit 1
       ;;
esac

echo "Succesfully backed up: $HOST ($2)"
        
exit 0

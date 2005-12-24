#!/bin/bash


# 1. create a dir for the backup
# 2. link null to dir: null -> dir
# 3. incremental dumps, dump to null
# 4. a new null dump removes the link and
#    goes to step 1

# a bash prototype of my new rdup utility
# TODO
# remote stuf
# encryption - get that in rsync

ARCHIVEDIR=/vol/backup
HOST=elektron-etc
DIR="/etc"

<<<<<<< .mine
SUFFIX=.`date +%Y%m%d%H%M`
=======
EXCLUDE="--exclude .svn/ --exclude tmp/ --exclude .slide_img --exclude
.thumb_img --exclude lost+found --exclude .Cache/"


SUFFIX=`date +%Y%m%d%H%M`
>>>>>>> .r19
NULLDIR=`date +%Y%m`
#OPT="-av --stats --compress --delete -b"
<<<<<<< .mine
OPT="-av --compress -C --delete -b -S"
=======
OPT="-av -W --compress --delete -b -S"
>>>>>>> .r19
# .nobackup files are interpreted by rsync
FILTER=.nobackup

case $1 in
        null)
                mkdir -p $ARCHIVEDIR/$HOST/$NULLDIR
                # remove prev. null link, and add the new one
                ( cd $ARCHIVEDIR/$HOST; rm -f null )
                ( cd $ARCHIVEDIR/$HOST; ln -sf $NULLDIR null )
                for i in $DIR ; do 
<<<<<<< .mine
                    rsync --filter ": /$FILTER" $OPT --suffix $SUFFIX $i $ARCHIVEDIR/$HOST/null
=======
                    rsync $EXCLUDE --filter ": /$FILTER" $OPT --suffix $SUFFIX $i $ARCHIVEDIR/$HOST/null
>>>>>>> .r19
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
       ;;
esac
        


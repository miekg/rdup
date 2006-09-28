#!/bin/sh

#/tmp/backup/YYYYMM/DD

# check if there is something in /tmp/backup
# no -> do a full dump *** 
# yes
# check if there is a previous dump
# go back a number of days...like 7 (option) -7
# copy the the old dir to the new dir
# if found 
# yes -> cp -rpl old new
#        do an incremental dump
# no -> do a full dump

# exit code: 1 full dump, 0 incremental dump


function what2do {
        dir=`date +%Y%m/%d --date "$i days ago"`
        backupdir=/tmp/backup
        if [[ ! -e $backupdir ]]; then
                mkdir -p $backupdir

                echo FULL DUMP
                return 1;
        fi

# backup dir is there
# find last backup
        for i in `seq 1 7`; do
                dir=`date +%Y%m/%d --date "$i days ago"`

                echo $dir
                if [[ -d $backupdir/$dir ]]; then
                        # copy 'em over
                        cp -plr $backupdir/$dir $backupdir/`date +%Y%m/%d`
                        echo INC DUMP
                        return 0
                fi
        done
        echo FULL
        return 1
}

what2do
purpose=$? # save return code from what2do

to="$backupdir/`date +%Y%m/%d`"

case $purpose in
        0)
        ./rdup -N STAMP LIST ~/Documents | ./hardlink.pl -b $to
        ;;
        1)
        rm LIST
        rm STAMP
        ./rdup -N STAMP LIST ~/Documents | ./hardlink.pl -b $to
        ;;
esac
exit 0

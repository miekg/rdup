#!/bin/sh

# create a hardlinked backup
# this scripts figures out if the
# dump is incremental or full

prefix=@prefix@
exec_prefix=@exec_prefix@



NOW=`date +%Y%m/%d`
BACKUPDIR=/tmp/backup

mkdir -p $BACKUPDIR
if [[ ! $? ]]; then
        echo "Can not create backup directory"
        exit 1
fi

what() {
        for i in `seq 1 7`; do
                dir=`date +%Y%m/%d --date "$i days ago"`

                echo $dir
                if [[ -d $BACKUPDIR/$dir ]]; then
                        # copy 'em over
                        echo "Linking old dir: \`$dir'"
                        cp -plr $BACKUPDIR/$dir $BACKUPDIR/$NOW
                        return 0
                fi
        done
        return 1
}

what; purpose=$? # save return code from what2do

case $purpose in
        0)
        echo "INCREMENTAL DUMP"
        ./rdup -N STAMP LIST ~/Documents | ./pl-tools/hardlink.pl -b $BACKUPDIR/$NOW
        ;;
        1)
        echo "FULL DUMP"
        rm LIST
        rm STAMP
        ./rdup -N STAMP LIST ~/Documents | ./pl-tools/hardlink.pl -b $BACKUPDIR/$NOW
        ;;
esac
exit 0

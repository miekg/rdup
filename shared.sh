#!/bin/bash
#
# Copyright (c) 2005, 2006 Miek Gieben
# See LICENSE for the license
# default user: root:backup (0:34 on my system)

backup_defines() {
        S_ISDIR=16384   # octal: 040000 (This seems to be portable...)
        S_ISLNK=40960   # octal: 0120000
        S_MMASK=4095    # octal: 00007777, mask to get permis
        suffix=`date +%Y%m%d.%H:%M`  # YYYYMMDD.HH:MM
}

backup_cmd_options() {
        while getopts ":zNhb:k:" options; do
                case $options in
                        b) backupdir=$OPTARG;;
                        z) gzip=1;;
                        k) keyfile=$OPTARG;;
                        N) dry=1;;
                        v) verbose=1;;
                        h) backup_cmd_usage && exit
                esac
        done
        if [ -z $backupdir ]; then 
                backupdir="/vol/backup/`hostname`"
        fi
        backupdir=$backupdir/`date +%Y%m`
}

backup_cmd_usage() {
        echo $0 "-bzkhNv"
        echo " -b DIR  use DIR as the backup directory, YYYYMM will be added"
        echo " -z      gzip regular files before backing up"
        echo " -k KEY  use the file KEY as encryption key"
        echo " -N      dry-run, show what would have been executed"
        echo " -v      echo the files processed to stderr"
        echo " -h      this help"
}

backup_create_top() {
        # need to reverse the order
        dir=$1;
        while [[ $dir != "/" ]]
        do
                dirs="$dir $dirs"
                dir=`dirname $dir`
        done
        for d in $dirs; do
                [[ $dry -eq 0 ]] && mkdir -m 755 "$dir"
                [[ $dry -eq 1 ]] && echo "mkdir -m 755 $dir"
                [[ $dry -eq 0 ]] && chown root:backup "$dir"
                [[ $dry -eq 1 ]] && echo "chown root:backup $dir"
        done
}

sftpbackup_create_top() {
        dir=$1
        while [[ $dir != "/" ]]
        do
                dirs="$dir $dirs"
                dir=`dirname $dir`
        done
        for d in $dirs; do
                # - : don't make sftp fail
                echo -mkdir $d
                echo -chown 0 $d
                echo -chgrp 34 $d
                echo -chmod 775 $d
        done
}

backup_successfull() {
        echo "Succesfully performed backup of `hostname`"
        echo "Backup stored in $1"
}

backup_failed() {
        echo "** Failed the backup of `hostname`"
}

list_cmd_options() {
        while getopts ":n:b:k:cCNdhz" options; do
                case $options in
                        b) backupdir=$OPTARG;;
                        n) daysago=$OPTARG;;
                        d) diff=1;;
                        N) dry=1;;
                        K) keyfile=$OPTARG;;
                        z) gzip=1;;
                        c) copy=1;;
                        C) Ccopy=1;;
                        h) list_cmd_usage && exit
                esac
        done
        if [ -z $backupdir ]; then
                backupdir="/vol/backup/`hostname`"
        fi
        backupdir=$backupdir/`date +%Y%m`
}

list_cmd_usage() {
        echo $0 "-nbkCcdhz"
        echo " -n DAYSAGO  go back DAYSAGO days"
        echo " -b DIR      use DIR as the backup directory, YYYYMM will be added"
        echo " -k KEY      use the file KEY as decryption key"
        echo " -c          copy the backed up file over the current file"
        echo " -C          copy the backed up file over the current file, if they differ"
        echo " -d          show a diff with the backed up file "
        echo " -N          dry-run, show what would have been executed"
        echo " -z          backup file is gzipped"
        echo " -h          this help"
}

## calculate the date back in time $1 days ago
datesago() {
        daysago=$(($1 * 86400))
        epochago=$((`date +%s` - $daysago))

        echo `date --date "Jan 1, 1970 00:00:00 CET + $epochago seconds" +%Y%m%d`
}

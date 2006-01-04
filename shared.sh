#!/bin/bash
#
# Copyright (c) 2005, 2006 Miek Gieben
# See LICENSE for the license
# default user: root:backup (0:34 on my system)

# some general defines needed to read rdup's output
backup_defines() {
        S_ISDIR=16384   # octal: 040000 (This seems to be portable...)
        S_ISLNK=40960   # octal: 0120000
        S_MMASK=4095    # octal: 00007777, mask to get permis
        suffix=`date %M%d.%H:%M`  # MMDD.HH:MM
        gzip=0
        keyfile=""
        verbose=0
}

# add the current date suffix to $backupdir
backup_dirdate() {
        backupdir=$backupdir/`date +%Y%m`
}

backup_cmd_options() {
        while getopts ":zNhb:k:" options; do
                case $options in
                        b) backupdir=$OPTARG;;
                        z) gzip=1;;
                        k) keyfile=$OPTARG;;
                        v) verbose=1;;
                        h) backup_cmd_usage && exit
                esac
        done
        if [ -z $backupdir ]; then 
                backupdir="/vol/backup/`hostname`"
        fi
}

backup_cmd_usage() {
        echo $0 "-bzkhNv"
        echo " -b DIR  use DIR as the backup directory, YYYYMM will be added"
        echo " -z      gzip regular files before backing up"
        echo " -k KEY  use the file KEY as encryption key"
        echo " -v      echo the files processed to stderr"
        echo " -h      this help"
}

# create the top level backup directory
backup_create_top() {
        # need to reverse the order
        dir=$1;
        while [[ $dir != "/" ]]
        do
                dirs="$dir $dirs"
                dir=`dirname $dir`
        done
        for d in $dirs; do
                if [[ ! -d $d ]]; then
                        mkdir -m 755 "$d"
                        chown root:backup "$d"
                fi
        done
}

# create the command for sftp to make
# the top level backup directory
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
}

list_defines() {
        daysago=0
        diff=0
        keyfile=""
        gzip=0
        copy=0
        Ccopy=0
}

list_cmd_usage() {
        echo $0 "-nbkCcdhz"
        echo " -n DAYSAGO  go back DAYSAGO days"
        echo " -b DIR      use DIR as the backup directory, YYYYMM will be added"
        echo " -k KEY      use the file KEY as decryption key"
        echo " -c          copy the backed up file over the current file"
        echo " -C          copy the backed up file over the current file, if they differ"
        echo " -d          show a diff with the backed up file "
        echo " -z          backup file is gzipped"
        echo " -h          this help"
}

## find the closest file to the one N days ago
## if there are multiple return the latest
recent() {
        if [[ $1 -ge 32 ]]; then return; fi 

        for i in `seq $1 -1 0`; do 
                suffix=`datesago $i`
                yyyymm=${suffix:0:6}
                # first check without suffix
                files=`ls "$backupdir"/$yyyymm/"$2" 2>/dev/null`
                if [[ -z $files ]]; then
                        files=`ls "$backupdir"/$yyyymm/"$2.$suffix".* 2>/dev/null`
                        if [[ ! -z $files ]]; then
                                break
                        fi
                else
                        break
                fi
        done
        if [[ -z $files ]]; then 
                return
        fi
        echo $files | sort -n -r | head -1
}

## calculate the date back in time $1 days ago
datesago() {
        daysago=$(($1 * 86400))
        epochago=$((`date +%s` - $daysago))

        echo `date --date "Jan 1, 1970 00:00:00 CET + $epochago seconds" +%Y%m%d`
}

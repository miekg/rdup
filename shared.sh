#!/bin/bash
#
# Copyright (c) 2005, 2006 Miek Gieben
# See LICENSE for the license

backup_defines() {
        S_ISDIR=16384   # octal: 040000 (This seems to be portable...)
        S_ISLNK=40960   # octal: 0120000
        S_MMASK=4095    # octal: 00007777, mask to get permis
        suffix=`date +%Y%m%d.%H:%M`  # YYYYMMDD.HH:MM
}

backup_cmd_options() {
        while getopts ":zhb:" options; do
                case $options in
                        b) backupdir=$OPTARG;;
                        z) gzip=1;;
                        h) backup_cmd_usage && exit
                esac
        done
        if [ -z $backupdir ]; then 
                #echo "** Setting archive directory to /vol/backup/`hostname`"
                backupdir="/vol/backup/`hostname`"
        fi
        backupdir=$backupdir/`date +%Y%m`
}

backup_cmd_usage() {
        echo $0 "-bzh"
        echo " -b dir  use dir as the backup directory, YYYYMM will be added"
        echo " -z      gzip regular files before backing up"
        echo " -h      this help"
}

backup_create_top() {
        mkdir -m 755 -p $1
        # why don't we have a -p for chown :-(
        # well at least the permission are so the anybody can enter
        chown root:backup $1
}

backup_succesfull() {
        echo "Succesfully performed backup of `hostname`"
        echo "Backup stored in $1"
}

backup_failed() {
        echo "** Failed the backup of `hostname`"
}

list_cmd_options() {
        while getopts ":n:b:cCdhz" options; do
                case $options in
                        b) backupdir=$OPTARG;;
                        n) daysago=$OPTARG;;
                        d) diff=1;;
                        z) gzip=1;;
                        h) list_cmd_usage && exit
                esac
        done
        if [ -z $backupdir ]; then
                #echo "** Setting archive directory to /vol/backup/`hostname`"
                backupdir="/vol/backup/`hostname`"
        fi
        backupdir=$backupdir/`date +%Y%m`
}

list_cmd_usage() {
        echo $0 "-n:b:Ccdhz"
        echo " -n daysago  go back daysago days"
        echo " -b dir      use dir as the backup directory, YYYYMM will be added"
        echo " -c          copy the backed up file over the current file"
        echo " -C          copy the backed up file over the current file, if they differ"
        echo " -d          show a diff with the backed up file "
        echo " -z          backup file is gzipped"
        echo " -h          this help"
}

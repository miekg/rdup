#!/bin/bash
#
# Copyright (c) 2005, 2006 Miek Gieben
# See LICENSE for the license
tool_cmd_options() {
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

# add the current date suffix to $backupdir
# sadly this is also defined in mshared.sh
tool_dirdate() {
                backupdir=$backupdir/`date +%Y%m`
}

tool_defines() {
        S_ISDIR=16384   # octal: 040000 (This seems to be portable...)
        S_ISLNK=40960   # octal: 0120000
        S_MMASK=4095    # octal: 00007777, mask to get permission
        daysago=0
        diff=0
        keyfile=""
        gzip=0
        copy=0
        Ccopy=0
}

tool_cmd_usage() {
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
## blaat           -n0 -> this one
## blaat+05:6:10   -n5 -> this one
## blaat+07:6:10   -n7 -> this one
recent() {
        if [[ $1 -ge 32 ]]; then return; fi 

        for i in `seq $1 -1 0`; do 
                suffix=`datesago $i`
                dayfix=${suffix:6:8}  # mshared.sh has the def.
                yyyymm=${suffix:0:6}
                # first check without suffix
                files=`ls "$backupdir"/$yyyymm/"$2" 2>/dev/null`
                if [[ -z $files ]]; then
                        files=`ls "$backupdir"/$yyyymm/"$2+$daysuffix".* 2>/dev/null`
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
        echo `date --date "$1 days ago" +%Y%m%d` # YYYYMMDD
}

monthsago() {
        echo `date --date "$1 months ago" +%Y%m` # YYYYMM
}

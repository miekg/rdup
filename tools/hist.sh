#!/bin/bash
#
# Copyright (c) 2005, 2006 Miek Gieben
# See LICENSE for the license
#
# inspired by hist of plan9 

usage() {
        echo "$0 [OPTIONS] FILE [FILE ...]"
        echo
        echo Print file history from the dump
        echo 
        echo FILE - search for this file in the backup directory
        echo
        echo OPTIONS
        echo "-b DIR  backup directory. Default: /vol/backup/HOSTNAME"
        echo "-h      this help"
}

monthsago() {
       echo `date --date "$1 months ago" +%Y%m` # YYYYMM
}


while getopts ":b:h" o; do
        case $o in
                b) backupdir=$OPTARG;;
                h) usage && exit;;
                \?) usage && exit;;
        esac
done
if [ -z $backupdir ]; then
        backupdir="/vol/backup/`hostname`"
fi
shift $((OPTIND - 1))

for file in $@
do
        if [[ -z $file ]]; then
                continue;
        fi
        if [[ ${file:0:1} != "/" ]]; then
                file=`pwd`/$file
        fi

        # go back 3 months
        for i in 0 1 2 3; do
                yyyymm=`monthsago $i`
                b=$backupdir/$yyyymm
        
                [[ ! -e $b$file ]] && continue

                if [[ -d $b$file ]]; then  
                        ls -odh $b$file 2>/dev/null \
| awk ' { print $5" "$6" "$7" "$8" "$9$4 }'
                        ls -odh $b$file+??.??:?? 2>/dev/null \
| awk ' { print $5" "$6" "$7" "$8" "$9$4 }'
                        continue
                fi

                # print +DD.HH:MM
                ls -o $b$file 2>/dev/null \
| awk ' { print $5" "$6" "$7" "$8" "$9$4 }'
                ls -o $b$file+??.??:?? 2>/dev/null \
| awk ' { print $5" "$6" "$7" "$8" "$9$4 }'
        done
done

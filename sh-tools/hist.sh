#!/bin/bash
#
# Copyright (c) 2005, 2006 Miek Gieben
# See LICENSE for the license
#
# inspired by hist of plan9 

set -o nounset

usage() {
        echo "$0 [OPTIONS] FILE [FILE ...]"
        echo
        echo Print file history from the dump
        echo 
        echo FILE - search for this file in the backup directory
        echo
        echo OPTIONS:
        echo " -b DIR  backup directory. Default: /vol/backup/HOSTNAME"
        echo " -d      causes diff(1) to be run for each adjacent pair of backupped files "
        echo " -h      this help"
}

monthsago() {
        # in YYYYMM
        case $OSTYPE in
                linux*)
                # from data's info page
                echo $(date --date "$(date +%Y-%m-15) $1 months ago" +%Y%m)
                ;;
                freebsd*)
                echo $(date -v -$1m "+%Y%m")
                ;;
        esac
}

diff=0
backupdir=""
while getopts ":b:hd" o; do
        case $o in
                b) backupdir=$OPTARG;;
                d) diff=1;;
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

        prev=""
        # go back 3 months
        for i in 0 1 2 3; do
                yyyymm=`monthsago $i`
                b=$backupdir/$yyyymm
                # print them
                for f in $b$file $b$file+??.??:?? ; do
                        [[ ! -e $f ]] && continue

                        if [[ -d $f ]]; then
                                ls -ldh $f | awk ' { print "d "$6" "$7" "$8" "$9" "$10" "$5 }' |\
                                sed "s|$b|\.\.\.|g"
                                continue
                        fi
                        ls -lh $f | awk ' { print $6" "$7" "$8" "$9" "$10" "$5 }' |\
                        sed "s|$b|\.\.\.|g"

                        [[ ! -z $prev ]] && [[ $diff -eq 1 ]] && \
                                diff -u $prev $f

                        prev=$f
                done
        done
done

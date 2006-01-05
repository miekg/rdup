#!/bin/bash
#
# Copyright (c) 2005, 2006 Miek Gieben
# See LICENSE for the license

# some general defines needed to read rdup's output
mirror_defines() {
        S_ISDIR=16384   # octal: 040000 (This seems to be portable...)
        S_ISLNK=40960   # octal: 0120000
        S_MMASK=4095    # octal: 00007777, mask to get permission
        suffix=`date +\+%d.%H:%M`  # +DD.HH:MM
        keyfile=""
        gzip=0
        verbose=0
        idir=0
        ireg=0
        ilnk=0
        irm=0
        ftsize=0
        ts=`date +%s` # gnuism
}

# add the current date suffix to $backupdir
mirror_dirdate() {
        backupdir=$backupdir/`date +%Y%m`
}

mirror_cmd_options() {
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

mirror_cmd_usage() {
        echo $0 "-bzkhNv"
        echo " -b DIR  use DIR as the backup directory, YYYYMM will be added"
        echo " -z      gzip regular files before backing up"
        echo " -k KEY  use the file KEY as encryption key"
        echo " -v      echo the files processed to stderr"
        echo " -h      this help"
}

# create the top level backup directory
mirror_create_top() {
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

mirror_ok() {
        te=`date +%s`
        echo "** #REG FILES  : $ireg" > /dev/fd/2
        echo "** #DIRECTORIES: $idir" > /dev/fd/2
        echo "** #LINKS      : $ilnk" > /dev/fd/2
        echo "** #(RE)MOVED  : $irm" > /dev/fd/2
        echo "** SIZE        : $(($ftsize / 1024 )) KB" > /dev/fd/2
        echo "** STORED IN   : $backupdir" > /dev/fd/2
        echo "** ELAPSED     : $(($te - $ts)) s" > /dev/fd/2
}

rmirror_ok() {
        te=`date +%s`
        echo "** #REG FILES  : $ireg"
        echo "** #DIRECTORIES: $idir"
        echo "** #LINKS      : $ilnk"
        echo "** #(RE)MOVED  : $irm" 
        echo "** SIZE        : $(($ftsize / 1024 )) KB" 
        echo "** STORED IN   : $backupdir" 
        echo "** ELAPSED     : $(($te - $ts)) s" 
}

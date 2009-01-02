#!/bin/bash

# hardlink a previous backup directory
# to a new directory

# SYNOPSIS: rdup-ln.sh [-l N] BACKUPDIR
# -l N: look back N days (default 8)
# -n  : dry run: don't touch the filesystem
# BACKUPDIR: top directory of the backups
#
# By default a new directory
# BACKUPDIR/YYYYMM/DD wil be created

# Three returns code
# 0: BACKUPDIR/YYYYMM/DD is created (now make a inc dump)
# 1: BACKUPDIR/YYYYMM/DD is created (now make a full dump)
# 2: an error occured

if [[ "$1" == "-l" ]]; then
    LOOKBACK=$2
    shift; shift;
else
    LOOKBACK=8
fi

DATESTR='+%Y%m/%d'
TODAY=$(date $DATESTR)
TOPDIR=$1

if [[ -z $TOPDIR ]]; then
    exit 2
fi

if [[ -d $TOPDIR/$TODAY ]]; then
    exit 0
else
    if ! mkdir -p $TOPDIR/$TODAY; then
    exit 2
    fi
fi

let i=1
while [[ $i -le $LOOKBACK ]]; do
	D=$(date $DATESTR --date "$i days ago")
	#echo $D >&2
	if [[ -d $TOPDIR/$D ]]; then
	    echo "Hardlinking: \`$TOPDIR/$D'" >&2
	    if ! cp -plr $TOPDIR/$D/* $TOPDIR/$TODAY; then
		exit 2
	    fi
	    exit 0
	fi
        let i=i+1
done
exit 1

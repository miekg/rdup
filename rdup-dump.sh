#!/bin/bash

# a bach prototype of my new rdup utility

ARCHIVEDIR=/tmp/storage
HOST=elektron
DIR=/home/miekg/bin
DIR1=/home/miekg/adm

SUFFIX=`date +%Y%m%d%H%M`
OPT="-av --stats --compress --delete -b"

mkdir -p $ARCHIVEDIR/$HOST
rsync $OPT --suffix $SUFFIX $DIR $ARCHIVEDIR/$HOST
rsync $OPT --suffix $SUFFIX $DIR1 $ARCHIVEDIR/$HOST

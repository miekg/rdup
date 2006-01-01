#!/bin/bash

# Copyright (c) 2005, 2006 Miek Gieben
# See LICENSE for the license
#
# Mimic hdup2
# Note: hdup2 can do more than this, but at its core: "this is it"
# (I can know, I've written it)

h=`hostname`
d=`date +%Y-%m-%d`

while read line
do
        # 5 6 7 is to catch files with spaces - not foolproof
        awk '{ print $5$6$7 '} | tar --create \
--no-recursion --gzip --file $h.$d.tar.gz \
--files-from -
done

echo $h.$d.tar.gz 

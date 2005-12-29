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
        # 4 5 6 is to catch files with spaces
        awk '{ print $4$5$6 '} | tar --create \
--no-recursion --gzip --file $h.$d.tar.gz \
--files-from -
done

echo $h.$d.tar.gz 

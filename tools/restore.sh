#!/bin/bash

# Restore a entire backed up directory
# when no options are given restore to yesterday's
# state (i.e. copy all files without the +extenstion)
# -n DAYSAGO (restore to DAYSAGO's state)


# go to the backup directory
cd $1


for i in *; do 
        echo "["$i"]"
        if [[ $i =~ "\\+.+\\..+:.+$" ]]; then
                echo "extension"
        fi
#re-create the local directory sctructure and populate it with files


#        link/dir/file
done

#!/bin/bash

# Restore a entire backed up directory
# when no options are given restore to yesterday's
# state (i.e. copy all files without the +extenstion)
# -n DAYSAGO (restore to DAYSAGO's state)


# go to the backup directory
cd $1


for i in *; do 
        if [[ $i =~ "\\+(.+)\\.(.+):(.+)$" ]]; then
                days=${BASH_REMATCH[1]}
                hour=${BASH_REMATCH[2]}
                min=${BASH_REMATCH[3]}
                continue;
        fi

        echo "["$i"]"
done

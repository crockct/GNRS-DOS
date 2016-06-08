#!/bin/bash

# Creates slivers, one for each rspec file passed. 
#
# Uses 'omni.py' to contact the AM identified in the rspec file to reserve
# resources in the sliver.

USAGE="Usage: `basename $0` <slicename> <rspec-file1> [rspec-file2] ...."

if [ $# -lt 2 ]; then
    echo "$USAGE" && exit 1
fi

slicename=$1;
files="${@:2}"

#echo $PYTHONPATH

for f in $files;
do
    #cmd to extract AM nickname from the rspec file
    am_nickname=$(perl -n -e 'if(/AM nickname: (.*)/){print $1}' $f)
    #echo "DEBUG: $f $am_nickname"
    if [ -z $am_nickname ]; then
        echo "ERROR: AM nickname not found in '$f'. Skipping"
        continue
    fi
    #omni.py -a <AM url/nickname> createsliver <slicename> <rspec-file>
    opfile=".createsliver-%a.result"
    result=$(omni.py -a $am_nickname --outputfile=$opfile createsliver $slicename $f)
    echo $result
done

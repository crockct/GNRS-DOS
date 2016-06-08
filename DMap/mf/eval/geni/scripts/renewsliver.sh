#!/bin/bash
#Renews slivers created previously using the same passed rspec file(s)

USAGE="Usage: `basename $0` <slicename> <date=YYYYMMDD> <rspec-file1> [<rspec-file2>] ..."

if [ $# -lt 3 ]; then
    echo "$USAGE" && exit 1
fi

slicename=$1
date=$2;
files=${@:3}

for f in $files;
do
    #cmd to extract AM nickname from the rspec file
    am_nickname=$(perl -n -e 'if(/AM nickname: (.*)/){print $1}' $f);
    #echo "DEBUG: $f $am_nickname"
    #omni.py -a <AM url/nickname> renewsliver <slicename> <date>
    opfile=".renewsliver-%a.result"
    result=$(omni.py -a $am_nickname --outputfile=$opfile renewsliver $slicename $date)
    echo $result
    
done

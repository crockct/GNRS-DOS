#!/bin/bash
# 
# A simple script to generate a sequential trace of inserts followed by
# queries for a GNRS client.
# 
# Author: Robert Moore
# Last Modified: Jan 14, 2013

FROM=1
TO=100

if [ $# -ge 1 ]; then
	FROM=$1
	if [ $# -ge 2 ]; then
		TO=$2
	fi
fi

SEQ=0
for (( I=$FROM; I <= $TO; I++ )); do
	echo "$SEQ I $I $I,999,1";
	(( SEQ++ )) ;
done

for (( I=$FROM; I <= $TO; I++ )); do
	echo "$SEQ Q $I"
	(( SEQ++ )) ;
done

#!/bin/bash
FILE_PATH=$"/root/GNRS-DOS/tcpdump/"

DATE=`date +%Y-%m-%d`
NODE=`hostname`
FILE_EXT=$".pcap"

tcpdump -w $FILE_PATH$DATE$NODE$FILE_EXT


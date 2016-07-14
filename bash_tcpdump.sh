#!/bin/bash
FILE_PATH=$"/root/GNRS-DOS/tcpdump/"

DATE=`date +%Y-%m-%d`
NODE=`hostname`
FILE_EXT=$".pcap"

tcpdump -ttttXvvSw $FILE_PATH$DATE$NODE$FILE_EXT
# tttt proper timetable X = show payload in ascii and hex vv = verbosity level, S = absolute seq w = write to file


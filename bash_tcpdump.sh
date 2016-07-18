#!/bin/bash
FILE_PATH=$"/root/GNRS-DOS/tcpdump/"

DATE=`date +%Y-%m-%d_time_%H-%M_`

SB_SUFFIX=$".sb4.orbit-lab.org"
NODE=`hostname`
NODE=${NODE%$SB_SUFFIX}
FILE_EXT=$".pcap"

tcpdump -ttttXvvSw $FILE_PATH$DATE$NODE$FILE_EXT
# tttt proper timetable X = show payload in ascii and hex vv = verbosity level, S = absolute seq w = write to file


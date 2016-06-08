#!/bin/bash

USAGE="Usage: `basename $0` <nodes-file> <cmd=list|start|stop|clean> [custom-args]

        custom-args: 
            if cmd='start': <settings_file> <oml_cfg_file> 
        "

if [ $# -lt 2 ]; then
    echo "$USAGE" && exit 1
elif [ "$2" == "start" ]; then
    if [ $# -lt 4 ]; then
        echo "Missing settings and/or OML cfg file"
        echo "$USAGE" && exit 1
    elif [ ! -e "$3" -o ! -e "$4" ]; then
        echo "Cannot find settings and/or OML cfg file"
        echo "$USAGE" && exit 1
    fi
fi


nodesfile=$1
cmd=$2
cargs=( ${@: 3} )


scriptdir=$(dirname $0)
[[ ! -e $scriptdir/config ]] && echo "Can't find 'config'!" && exit 1
source $scriptdir/config
[[ ! -e $scriptdir/util.sh ]] && echo "Can't find 'util.sh'!" && exit 1
source $scriptdir/util.sh

readarray nodesarr < $nodesfile
hostnames=( $(perl -n -a -F, -e 'next if /^\s*#/;print $F[0], " "' $nodesfile) )
interfaces=( $(perl -n -a -F, -e 'next if /^\s*#/;print $F[1], " "' $nodesfile) )
ipv4addrs=( $(perl -n -a -F, -e 'next if /^\s*#/;print $F[3], " "' $nodesfile) )
guids=( $(perl -n -a -F, -e 'next if /^#/;print $F[4], " "' $nodesfile) )

r_sttgsfile="$r_cfgdir/mfhoststack.conf"
r_stackcommdir="/data/mfdemo"

r_omlfile="$r_cfgdir/stack-oml-config.xml"

loglevel="e"

#Usage:
#mfstack [-h] [-d|i|t|w|e|c|f] [-O] settings_file
#
#  -h    print this help and exit
#  -v    print version file and exit
#  -d|i|w|e|c|f  set log level(DEBUG|INFO|WARN|ERROR|CRITICAL|FATAL), default:QUIET
#  -O    report metrics with OML
#  settings_file required

r_stackcmd="/usr/local/bin/mfstack -$loglevel -O --oml-config $r_omlfile $r_sttgsfile" 
r_stacklog="/var/log/mfstack.log"

tmpnodesfile=".nodes_hosts"
truncate -s 0 $tmpnodesfile

lpcnt=0
nodecnt=0

declare -A c_guid
declare -A c_iface

for h in "${hostnames[@]}"; do

    #filter interfaces based on edge network 
    ipv4=${ipv4addrs[$lpcnt]}
    enetbin=$(echo "$ipv4.$Edgenetmask" | perl -n -a -F\\. -e 'print ((((($F[0] << 8) + $F[1] << 8) + $F[2] << 8) + $F[3]) >> $F[4])')

    lastoctet=${ipv4##*.}
    if [ $Edgenetbin -eq $enetbin -a $lastoctet -ge 128 ]; then
        c_host[$nodecnt]=$h
        c_guid[$h]=${guids[$lpcnt]}
        c_iface[$h]=${interfaces[$lpcnt]}
        echo -n "${nodesarr[$lpcnt]}" >> $tmpnodesfile
        ((nodecnt++))
    fi
    ((lpcnt++))
done

dolist() {

    banner "Host stack (or client) nodes:"
    for h in "${c_host[@]}"
    do
        subbanner "$h"
    done
}

dostart () {

    settingsfile=$1
    omlfile=$2

    banner "Bringing up Host Stacks"

    #set up dir for creating userapp-stack comm. sockets and stack config 
    $scriptdir/gssh $tmpnodesfile "sudo sh -c 'mkdir -p $r_stackcommdir;chmod 777 $r_stackcommdir';mkdir -p $r_cfgdir"

    #customize and copy stack config file
    for h in "${c_host[@]}"
    do
        subbanner "node: $h" "Setting up dirs and configuration files"

        #customize settings 
        tmpsttgsfile="$scriptdir/.mfhoststack.conf"
        cp $settingsfile $tmpsttgsfile
        eval "perl -pi -e 's/EDGE_IFACE/${c_iface[$h]}/' $tmpsttgsfile"
        eval "perl -pi -e 's/HOST_GUID/${c_guid[$h]}/' $tmpsttgsfile"
        scp -i $geni_key $tmpsttgsfile $geni_username@$h:$r_sttgsfile
        rm -f $tmpsttgsfile

        scp -i $geni_key $omlfile $geni_username@$h:$r_omlfile
    done


    subbanner "Starting stack processes"
    $scriptdir/gssh $tmpnodesfile "sudo sh -c '$r_stackcmd > $r_stacklog 2>&1 &'"
}

dostop () {

    $scriptdir/gssh $tmpnodesfile "sudo killall -9 mfstack"
}

doclean(){
    dostop
    $scriptdir/gssh $tmpnodesfile "sudo rm -rf /data/mfdemo"
    $scriptdir/gssh $tmpnodesfile "sudo rmdir /data" #remove if empty
}

case $cmd in

    list)
        dolist
        ;;
    start) 
        dostart "${cargs[0]}" "${cargs[1]}"
        ;;
    stop)
        dostop 
        ;;
    clean)
        doclean
        ;;
    *)
        echo "unknown command: $cmd"
        echo "$USAGE" && exit 1
esac

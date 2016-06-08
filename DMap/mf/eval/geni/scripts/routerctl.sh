#!/bin/bash

USAGE="Usage: $0 <nodes-file> <cmd=list|start|stop> <topologyfile>"

[[ $# -lt 3 ]] && echo "$USAGE" && exit

nodesfile=$1
cmd=$2
topologyfile=$3

[[ ! -e $topologyfile ]] && echo "ERROR: Can't find '$topologyfile'" && exit 1

scriptdir=$(dirname $0)
[[ ! -e $scriptdir/config ]] && echo "ERROR: Can't find 'config'" && exit 1
[[ ! -e $scriptdir/util.sh ]] && echo "ERROR: Can't find 'util.sh'" && exit 1
source $scriptdir/config
source $scriptdir/util.sh

readarray nodesarr < $nodesfile
hostnames=( $(perl -na -F, -e 'next if /^\s*#/;print $F[0], " "' $nodesfile) )
interfaces=( $(perl -na -F, -e 'next if /^\s*#/;print $F[1], " "' $nodesfile) )
hwaddrs=( $(perl -na -F, -e 'next if /^\s*#/;print $F[2], " "' $nodesfile) )
ipv4addrs=( $(perl -na -F, -e 'next if /^\s*#/;print $F[3], " "' $nodesfile) )
guids=( $(perl -na -F, -e 'next if /^#/;print $F[4], " "' $nodesfile) )

MF_CLICK_OPTIONS="-j 4 -p $r_clickcontrolport"
r_topologyfile="$r_scriptdir/topology.tp"

loglevel=1

#access router
accrtrcmd="/usr/local/bin/click $MF_CLICK_OPTIONS $r_mfsrcdir/router/click/conf/MF_IPAccessMultiRouter.click \
my_GUID=__GUID \
topo_file=$r_topologyfile \
core_dev=CORE_IFACE \
GNRS_server_ip=CORE_IP GNRS_server_port=$r_gnrsport \
GNRS_listen_ip=CORE_IP GNRS_listen_port=$r_gnrsclientport \
edge_dev=EDGE_IFACE \
edge_dev_ip=EDGE_IP"

#core router
rtrcmd="/usr/local/bin/click $MF_CLICK_OPTIONS $r_mfsrcdir/router/click/conf/MF_MultiRouter.click \
my_GUID=__GUID \
topo_file=$r_topologyfile \
core_dev=CORE_IFACE \
GNRS_server_ip=CORE_IP GNRS_server_port=$r_gnrsport \
GNRS_listen_ip=CORE_IP GNRS_listen_port=$r_gnrsclientport"

lpcnt=0
nodecnt=0
c_host=()

declare -A c_coreip
declare -A c_coreif
declare -A c_guid
declare -A c_edgeif
declare -A c_edgeip

tmpnodesfile=".nodes_rtr"
truncate -s 0 $tmpnodesfile

for h in "${hostnames[@]}"; do

    #filter interfaces based on desired network 
    ipv4=${ipv4addrs[$lpcnt]}
    cnetbin=$(echo "${ipv4}.$Corenetmask" | perl -n -a -F\\. -e \
        'print ((((($F[0] << 8) + $F[1] << 8) + $F[2] << 8) + $F[3]) >> $F[4])')
    enetbin=$(echo "${ipv4}.$Edgenetmask" | perl -n -a -F\\. -e \
        'print ((((($F[0] << 8) + $F[1] << 8) + $F[2] << 8) + $F[3]) >> $F[4])')
    #echo "DEBUG: ipv4 $ipv4 cnetbin $cnetbin enetbin $enetbin"

    #skip client nodes - last octet >= 128
    lastoctet=${ipv4##*.}
    if [ $lastoctet -ge 128 ]; then
        ((lpcnt++)) 
        continue
    fi

    if [ $Corenetbin -eq $cnetbin ]; then
        c_host[$nodecnt]=$h
        c_coreip[$h]=$ipv4
        c_coreif[$h]=${interfaces[$lpcnt]}
        c_guid[$h]=${guids[$lpcnt]}
        #create tmp file with only the selected router nodes 
        echo "${nodesarr[$lpcnt]}" >> $tmpnodesfile
        ((nodecnt++))
    elif [ $Edgenetbin -eq $enetbin ]; then
        c_edgeif[$h]=${interfaces[$lpcnt]}
        c_edgeip[$h]=$ipv4
    fi
    ((lpcnt++))
done

# Extracts set of guids in specified topology. 
#
# Used to identify nodes that need to be active to turn on the topo
# arg1: topologyfile

declare -a activeguid

function fillactiveguid() {

    topofile=$1
    while read line
    do
        [[ "$line" =~ ^[[:space:]]?# || "$line" =~ ^[[:space:]]?$ ]] && continue
        #format: hostguid nbrcnt nbr1guid nrb2guid ...
        arr=( $line )
        activeguid[${arr[0]}]=1
        for ((i=2; i <= ${arr[1]}; i++))
        do
            activeguid[${arr[$i]}]=1
            #echo "guid: ${arr[$i]} is active"
        done
    done < $topofile
}


function dolist() {

core=();c_cnt=0
edge=();e_cnt=0

    for h in "${c_host[@]}";
    do
        coreip=${c_coreip[$h]}
        coreif=${c_coreif[$h]}
        guid=${c_guid[$h]}
        active=true
        if [ -z ${activeguid[$guid]} ]; then
            active=false
        fi
        if [ -z ${c_edgeip[$h]} ]; then
            core[$c_cnt]="$h\n\tcore: $coreip/$coreif guid: $guid active: $active"
            ((c_cnt++))
        else
            edgeip=${c_edgeip[$h]}
            edgeif=${c_edgeif[$h]}
            edge[$e_cnt]="$h\n\tcore: $coreip/$coreif edge: $edgeip/$edgeif guid: $guid active: $active"
            ((e_cnt++))
        fi
    done

    subbanner "Core routers"
    for s in "${core[@]}";
    do
        echo -e "$s"
    done

    subbanner "Edge routers"
    for s in "${edge[@]}";
    do
        echo -e "$s"
    done
}

r_clicklogfile="/var/log/mfclick.log"

dostart () {

    banner "Starting MF Routers"
    for h in "${c_host[@]}"
    do
        #skip inactive routers
        [[ -z ${activeguid[${c_guid[$h]}]} ]] && continue

        #push topo file
        src="$topologyfile"; dst="$r_topologyfile"
        subbanner "node: $h" "cmd: scp $src -> $dst"
        scp -i $geni_key $src $geni_username@$h:"$dst"

        if [ -z ${c_edgeip[$h]} ]; then # core router
            cmd=$rtrcmd 
            msg2="Starting CORE MF router"
        else #access router
            msg2="Starting EDGE MF router"
            cmd=$accrtrcmd 
            cmd=${cmd//"EDGE_IFACE"/${c_edgeif[$h]}}
            cmd=${cmd//"EDGE_IP"/${c_edgeip[$h]}}
        fi
        cmd=${cmd//"__GUID"/${c_guid[$h]}}
        cmd=${cmd//"CORE_IFACE"/${c_coreif[$h]}}
        cmd=${cmd//"CORE_IP"/${c_coreip[$h]}}

        cmd="sudo sh -c 'export MF_CLICK_LOG_LEVEL=$loglevel; $cmd > $r_clicklogfile 2>&1 &'"
        subbanner "node: $h" "$msg2" "cmd: $cmd"
        ssh -i $geni_key $geni_username@$h "$cmd"
    done
}

dostop () {

    banner "Stopping MF routers"
    for h in "${c_host[@]}"
    do
        #skip inactive routers
        [[ -z ${activeguid[${c_guid[$h]}]} ]] && continue

        cmd="sudo killall -9 click"
        subbanner "node: $h" "cmd: $cmd"
        ssh -i $geni_key $geni_username@$h "$cmd"
    done
}

doclean() {

    dostop
    banner "Cleaning MF Routers"
    for h in "${c_host[@]}"
    do
        #skip inactive routers
        [[ -z ${activeguid[${c_guid[$h]}]} ]] && continue

        cmd="sudo sh -c 'killall -9 click; rm -f $r_clicklogfile'"
        subbanner "node: $h" "cmd: $cmd"
        ssh -i $geni_key $geni_username@$h "$cmd"
    done
}

fillactiveguid $topologyfile

case $cmd in

    list)
        dolist
        ;;

    start) 
        dostart
        ;;
    stop)
        dostop 
        ;;
    clean)
        doclean
        ;;
    *)
        echo "ERROR: unknown command: $cmd"
        printusage
        exit
esac

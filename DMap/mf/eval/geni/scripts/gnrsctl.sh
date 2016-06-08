#!/bin/bash

# Controls running of GNRS service across a node set.
# 
# Includes start/stop, status check, cleanup (process, persistent data & logs),

USAGE="Usage: $0 <nodes-file> {config|stat|start|stop|clean} [<custom-args>]

        custom-args:
            if 'config': <template-config-dir> <prefixes-file>
    "

[[ $# -lt 2 ]] && echo -e "$USAGE" && exit 1

nodesfile=$1
cmd=$2
cargs=( ${@: 3} )

scriptdir=$(dirname $0)
[[ ! -e $scriptdir/config ]] && echo "ERROR: Can't find 'config'!" && exit 1
[[ ! -e $scriptdir/util.sh ]] && echo "ERROR: Can't find 'util.sh'!" && exit 1
source $scriptdir/config
source $scriptdir/util.sh

export JAVA_HOME="$r_java_home"

JAVA_BIN="$JAVA_HOME/bin/java"

Reqinterval=1000 #usec
Tracefile="test.trace"

nodesarr=( $(perl -n -e 'next if /^\s*#/;print "$_ "' $nodesfile) )
hostnames=( $(perl -n -a -F, -e 'next if /^\s*#/;print $F[0], " "' $nodesfile) )
ipv4addrs=( $(perl -n -a -F, -e 'next if /^\s*#/;print $F[3], " "' $nodesfile) )

tmpsrvrnodesfile=".nodes_gnrsctl_srvr"
tmpcltnodesfile=".nodes_gnrsctl_clnt"
tmpallnodesfile=".nodes_gnrsctl_all"
truncate -s 0 $tmpsrvrnodesfile $tmpcltnodesfile $tmpallnodesfile

lpcnt=0

declare -A is_server
declare -A c_ipv4

for h in "${hostnames[@]}"; do

    # filter interfaces based on desired network 
    ipv4=${ipv4addrs[$lpcnt]}
    cnetbin=$(echo "$ipv4.$Corenetmask" | perl -n -a -F\\. -e 'print ((((($F[0] << 8) + $F[1] << 8) + $F[2] << 8) + $F[3]) >> $F[4])')
    if [ $Corenetbin -eq $cnetbin ]; then
        lastoctet=${ipv4##*.}

        c_host[$nodecnt]=$h
        c_ipv4[$h]=$ipv4
        if [ $lastoctet -lt 128 ]; then # server
            echo "${nodesarr[$lpcnt]}" | tee -a  $tmpsrvrnodesfile >> $tmpallnodesfile
            is_server[$h]=true
        else # client
            echo "${nodesarr[$lpcnt]}" | tee -a $tmpcltnodesfile >> $tmpallnodesfile
            is_server[$h]=false
        fi
        ((nodecnt++))
    fi
    ((lpcnt++))
done


function doconfig() {

    tmplcfgdir=$1
    prefixfile=$2
    tmpcfgdir="./.gnrsconfdir"
    mkdir -p $tmpcfgdir

    cp ${tmplcfgdir}/* $tmpcfgdir/
    cp ${prefixfile} $tmpcfgdir/prefixes.ipv4

    #write out AS to server instance binding file
    #create AS# to server instance binding
    astoipv4file="$tmpcfgdir/as-binding.ipv4"
    truncate -s 0 $astoipv4file
    echo -e "# AS to IP-port mappings for GNRS server instances" > $astoipv4file
    echo -e "# Syntax: ASnumber IPv4address  port\n" >> $astoipv4file

    #assign AS ids from 1..numAS
    asid=1;
    declare -A c_asid

    for h in ${c_host[@]}; do
        if [ "${is_server[$h]}" == true ]; 
        then
                echo "$asid ${c_ipv4[$h]} $r_gnrsport" >> $astoipv4file
                c_asid[$h]=$asid;
                ((asid++))
        fi
    done

    #do escape for slashes in pathnames
    gnrscfgdir=$(echo $r_gnrscfgdir | perl -pi -e 's/\//\\\//g')
    gnrsstatdir=$(echo $r_gnrsstatdir | perl -pi -e 's/\//\\\//g')
    gnrslogdir=$(echo $r_gnrslogdir | perl -pi -e 's/\//\\\//g')
    gnrsdbdir=$(echo $r_gnrsdbdir | perl -pi -e 's/\//\\\//g')

    #set placeholders in config files to reflect testbed settings
    eval "perl -pi -e 's/CONFIG_DIR/$gnrscfgdir/g' $tmpcfgdir/*.xml"
    eval "perl -pi -e 's/STAT_DIR/$gnrsstatdir/g' $tmpcfgdir/server.xml"
    eval "perl -pi -e 's/LOG_DIR/$gnrslogdir/g' $tmpcfgdir/log4j.xml"
    eval "perl -pi -e 's/DB_DIR/$gnrsdbdir/g' $tmpcfgdir/berkeleydb.xml"

    #do any per host customization and copy over to testbed nodes 
    lpcnt=0
    srvrcnt=0

    # keep copies of original for customization for each server
    cp $tmpcfgdir/net-ipv4.xml $tmpcfgdir/net-ipv4.xml.tmpl

    for h in ${c_host[@]}; do

        [[ "${is_server[$h]}" == false ]] && continue

        #Prior to copy, set local server IP address in net_ipv4.xml
        cp $tmpcfgdir/net-ipv4.xml.tmpl $tmpcfgdir/net-ipv4.xml
        eval "perl -pi -e 's/(<bindAddress)>.*(<\/bindAddress>)/\$1>${c_ipv4[$h]}\$2/' $tmpcfgdir/net-ipv4.xml"
        #clean prior config
        rm -f $tmpcfgdir/*.bak
        echo "INFO: Server: $h"
        echo -e -n "INFO:\tCopying configs ....."
        ssh -i $geni_key $geni_username@$h "rm -rf $r_gnrscfgdir; mkdir -p $r_gnrscfgdir"
        scp -q -i $geni_key -r $tmpcfgdir/* $geni_username@$h:$r_gnrscfgdir/
        echo " Done."
    done
}

function dostat() {

    r_gnrsstat="ps -ef | grep java | grep gnrs" 

    banner "Checking status of GNRS service" 
    $scriptdir/gssh $tmpsrvrnodesfile "$r_gnrsstat"
}

# Starts server processes on picked nodes
function dostart() {

    #Note: this is for csh 
    #r_gnrsstart="$JAVA_BIN -Dlog4j.configuration=file:$r_gnrscfgdir/log4j.xml -jar $r_jardir/gnrs-server-1.0.0-SNAPSHOT-jar-with-dependencies.jar $r_gnrscfgdir/server.xml > & $r_logdir/gnrs.out &"

    #Note: this is for bash
    r_gnrsstart="$JAVA_BIN -Dlog4j.configuration=file:$r_gnrscfgdir/log4j.xml -jar $r_jardir/gnrs-server-1.0.0-SNAPSHOT-jar-with-dependencies.jar $r_gnrscfgdir/server.xml > $r_logdir/gnrs.out 2>&1 &"

    banner "Starting GNRS service" 
    $scriptdir/gssh $tmpsrvrnodesfile "$r_gnrsstart"
}


# Stops server instances by sending a TERM signal
#
# TERM signal triggers a stats dump by the server processes before 
# exiting.

function dostop() {

    r_gnrsstop="killall -TERM java"

    banner "Stopping GNRS service" 
    $scriptdir/gssh $tmpsrvrnodesfile "$r_gnrsstop"
}

# Stops processes and cleans logs

function doclean() {

    r_gnrsclean="killall -9 java;rm -f $r_gnrslogdir/gnrsd.log*;rm -rf $r_gnrsdbdir $r_gnrsstatdir"

    banner "Cleaning GNRS service" 
    $scriptdir/gssh $tmpallnodesfile "$r_gnrsclean"
}

case $cmd in

    config)
        [[ ${#cargs[@]} -ne 2 || ! -d ${cargs[0]} || ! -f ${cargs[1]} ]] && echo "$USAGE" && exit 1
        doconfig ${cargs[0]} ${cargs[1]}
        ;;
    stat)
        dostat
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
        echo "$USAGE" && exit 1
        ;;
esac

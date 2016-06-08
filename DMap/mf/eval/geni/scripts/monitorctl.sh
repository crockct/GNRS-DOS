#!/bin/bash

USAGE="Usage: $0 <nodes-file> <cmd=start|stop|clean>" 

[[ $# -lt 2 ]] && echo "$USAGE" && exit 

nodesfile=$1
cmd=$2

scriptdir=$(dirname $0)
[[ ! -e $scriptdir/config ]] && echo "Can't find 'config'!" && exit 1
[[ ! -e $scriptdir/util.sh ]] && echo "Can't find 'util.sh'!" && exit 1
source $scriptdir/config
source $scriptdir/util.sh

r_nodemonbin="$r_mfbindir/mf_node_mon"
r_clickmonbin="$r_mfbindir/mf_click_mon"
r_nodemonlog="$r_logdir/nodemon.out"
r_clickmonlog="$r_logdir/clickmon.out"

function dostart() {

    #copy configs
    $scriptdir/gscp $nodesfile "$l_confdir/node-oml-config.xml" "$l_confdir/click-oml-config.xml"  "$r_scriptdir/"

    #gssh can substitute 'GUID' in cmd with guid of host
#    $scriptdir/gssh $nodesfile "$r_nodemonbin __GUID --oml-config $r_scriptdir/node-oml-config.xml > & $r_nodemonlog &"
#    $scriptdir/gssh $nodesfile "$r_clickmonbin $r_clickcontrolport __GUID --oml-config $r_scriptdir/click-oml-config.xml > & $r_clickmonlog &"
    $scriptdir/gssh $nodesfile "$r_nodemonbin __GUID --oml-config $r_scriptdir/node-oml-config.xml > $r_nodemonlog 2>&1 &"
    $scriptdir/gssh $nodesfile "$r_clickmonbin $r_clickcontrolport __GUID --oml-config $r_scriptdir/click-oml-config.xml > $r_clickmonlog 2>&1 &"

}

function dostop() {

    #TODO: should probably do something more graceful
    $scriptdir/gssh $nodesfile "killall -9 mf_node_mon mf_click_mon"

}

function doclean() {

    $scriptdir/gssh $nodesfile "killall -9 mf_node_mon mf_click_mon; rm -rf $r_nodemonlog $r_clickmonlog"

}

case $cmd in

    start) dostart
        ;;

    stop) dostop
        ;;

    clean) doclean
        ;;

    *) echo -e "Unknown cmd '$cmd' \n\n$USAGE" && exit

esac

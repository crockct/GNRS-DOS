#!/bin/bash

# Controls running of the collection of components that constitute a MobilityFirst experiment.
# 
# Includes start/stop, cleanup (processes, persistent data & logs),

cmd=$1

scriptdir=$(dirname $0)
[[ ! -e $scriptdir/config ]] && echo "Can't find 'config'!" && exit 1
source $scriptdir/config
[[ ! -e $scriptdir/util.sh ]] && echo "Can't find 'util.sh'!" && exit 1
source $scriptdir/util.sh


case $cmd in

    start)
        $scriptdir/gnrsctl.sh $scriptdir/nodes.gnrs start
        $scriptdir/routerctl.sh $scriptdir/nodes.rtr start
        $scriptdir/hostctl.sh $scriptdir/nodes.hosts start
        $scriptdir/setupmonitors.sh nodes.rtr 
        ;;
    stop)
        $scriptdir/gnrsctl.sh $scriptdir/nodes.gnrs stop
        $scriptdir/routerctl.sh $scriptdir/nodes.rtr stop
        $scriptdir/hostctl.sh $scriptdir/nodes.hosts stop
        $scriptdir/gssh nodes.rtr "killall -9 mf_node_mon mf_click_mon"
        ;;
    clean)
        $scriptdir/gnrsctl.sh $scriptdir/nodes.gnrs clean
        $scriptdir/routerctl.sh $scriptdir/nodes.rtr stop
        $scriptdir/hostctl.sh $scriptdir/nodes.hosts stop
        $scriptdir/gssh nodes.rtr "killall -9 mf_node_mon mf_click_mon"

        ;;
    *)
        echo "unknown command try {start|stop|clean}"
        ;;
esac

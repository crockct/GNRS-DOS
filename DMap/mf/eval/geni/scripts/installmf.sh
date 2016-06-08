#!/bin/bash

USAGE="Usage: `basename $0` <nodes-file> [<build_opts>={clean}]"

if [ $# -lt 1 ]; then
    echo "$USAGE" && exit 1
fi

nodesfile=$1
buildopt=""

if [ $# -gt 1 ]; then
    buildopt=$2
fi

scriptdir=$(dirname $0)
[[ ! -e $scriptdir/config ]] && echo "ERROR: Can't find 'config'!" && exit 1
[[ ! -e $scriptdir/util.sh ]] && echo "ERROR: Can't find 'util.sh'!" && exit 1
source $scriptdir/config
source $scriptdir/util.sh

# concurrent install across all nodes
banner "Setting up dirs and scripts"
$scriptdir/gscp $nodesfile "$scriptdir/.netrc" "$r_basedir/"
$scriptdir/gssh $nodesfile "mkdir -p $r_scriptdir $r_logdir $r_tracedir $r_tooldir $r_jardir $r_confdir"
$scriptdir/gscp $nodesfile  "$scriptdir/localinstallmf.sh" "$scriptdir/config" "$scriptdir/util.sh" "$r_scriptdir/"
# Note: the following won't work if a gui tool is used to enter passwd for git
# repo access as it doesn't pass -X to the ssh 
banner "Installing MF on target nodes..." "(this may take several minutes, logs follow)"
$scriptdir/gssh $nodesfile "$r_scriptdir/localinstallmf.sh $buildopt"

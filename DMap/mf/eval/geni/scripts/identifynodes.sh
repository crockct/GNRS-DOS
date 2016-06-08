#!/bin/bash

# Identifies the set of hosts (and their attributes) in a sliver. 
#
# Slivers are specified by rspec files used to create them, each 
# having the info of the corresponding AM. Host attributes incl. hostname,
# set of active interfaces, and the host guid (from the host ssh key).
# This uses the 'readyToLogin.py' tool contained in the 'gcf' distribution to
# determine hostnames, and then queries the hosts directly.

USAGE="Usage: `basename $0` <slicename> <rspec-file1> [<rspec-file2>] ..."

if [ $# -lt 2 ]; then
    echo "$USAGE"
    exit
fi

slicename=$1
files=${@:2}

scriptdir=$(dirname $0)
[[ ! -e $scriptdir/config ]] && echo "ERROR: Can't find 'config'!" && exit 1
source $scriptdir/config #get username,key for ssh

#retrieve the hostname and network interface details for each AM extracted from the individual rspec files. Output one line per host and per interface:
echo "#hostname,interface,hwaddr,ipv4addr,guid"

for f in $files;
do
    am_nickname=$(perl -n -e 'if(/AM nickname: (.*)/){print $1}' $f)

    #use GCF/omni tool 'readyToLogin' to retrieve hostname(s) of reserved instances
    hostnames=$(readyToLogin.py -a $am_nickname $slicename 2> /dev/null | perl -n -e 'if(/HostName (.*)/){print $1." "}')

    #echo "DEBUG: At AM $am_nickname got host(s): '$hostnames'"
    [[ -z $hostnames ]] && continue

    #retreive set of interfaces with addresses for this host
    #result: 
    #iface,iftype,hwaddr,ipv4addr,ipv4netmask
    #eth0,Ethernet,02:b9:5e:f7:d6:c4,192.1.242.158,192.1.242.255,255.255.255.128
    #eth1,Ethernet,02:03:60:3d:f8:28,10.44.2.1,10.44.255.255,255.255.0.0

    ifconfig_cmd="/sbin/ifconfig"

    for h in $hostnames; do
        [[ -z $h || $h == [[:space:]] ]] && continue
        #echo "DEBUG: Getting interfaces for host '$h'"
        ifaces=$(ssh -i $geni_key $geni_username@$h $ifconfig_cmd | perl -n -e 'if(/^(\S*)\s*Link encap:\s*(\S*)\s*HWaddr\s*(\S*)/){$if=$1;$iftype=$2;$ifhwaddr=$3;$newif=1;}if($newif==1 && /inet addr:\s*(\S*)\s*Bcast:\s*(\S*)\s*Mask:\s*(\S*)/){print $if.",".$iftype.",".$ifhwaddr.",".$1.",".$2.",".$3." ";$newif=0}')

        #We use the host RSA key file to derive a 32bit integer guid for host
        guid=$(echo 'ibase=16;obase=A;'`ssh -i $geni_key $geni_username@$h md5sum /etc/ssh/ssh_host_rsa_key.pub | perl -n -a -e 'print $F[0]' | tr '[:lower:]' '[:upper:]'` | bc)
        guid=$(echo "$guid%(2^31)" | bc)
        for i in $ifaces;
        do
            [[ -z $i || $i == [[:space:]] ]] && continue
            iface=$(echo $i | perl -n -a -F, -e 'print $F[0]')
            hwaddr=$(echo $i | perl -n -a -F, -e 'print $F[2]')
            ipv4addr=$(echo $i | perl -n -a -F, -e 'print $F[3]')
            echo "$h,$iface,$hwaddr,$ipv4addr,$guid" 
        done
    done
done
